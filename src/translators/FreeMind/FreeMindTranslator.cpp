#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <ByteOrder.h>
#include <Message.h>
#include <Screen.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Alert.h>
#include <File.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Bitmap.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "FreeMindTranslator.h"
#include "ConfigView.h"

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	status_t err	= B_OK;
	char		xmlString[11];
	if (outType == 0)
		outType = P_C_FREEMIND_TYPE;
	if (outType != P_C_FREEMIND_TYPE && outType != P_C_DOCUMENT_RAW_TYPE) {
		return B_NO_TRANSLATOR;
	}
	// Identify() runs against every file any app on the system scans via
	// BTranslatorRoster, not just ProjectConceptor/FreeMind documents - a
	// cheap, bounded read is all format detection should ever cost here.
	// This used to fall back to a full Unflatten() of the whole stream
	// (checking for a "PDocument::allNodes" field) whenever the 10-byte
	// read didn't come back with exactly 10 bytes - i.e. for any file
	// smaller than 10 bytes, which is a narrow case but still exactly the
	// kind of unbounded full-stream parse the fix for #73 removes from
	// StandardTranslator's Identify() too. A file that's too short to
	// contain "<map" isn't a FreeMind document either way.
	ssize_t	bytesRead	= inSource->Read(xmlString, 10);
	if (bytesRead < 4)
		return B_NO_TRANSLATOR;
	xmlString[bytesRead]	= '\0';
	if (strstr(xmlString,"<map")!=NULL)
		outType = P_C_DOCUMENT_RAW_TYPE;
	else
		return B_NO_TRANSLATOR;


	if (outType == P_C_FREEMIND_TYPE) {
		// outtype is text becaus xml is text
		outInfo->group = B_TRANSLATOR_TEXT;
		outInfo->type = P_C_FREEMIND_TYPE;
		outInfo->quality = 0.3;
		outInfo->capability = 0.7;
		strcpy(outInfo->name, "Freemind Mindmap format");
		strcpy(outInfo->MIME, "application/x-freemind");
	}
	else {
	// outtype group ist always B_TRANSLATOR_NONE because we dont support a standart type
		outInfo->group = B_TRANSLATOR_NONE;
		outInfo->type = P_C_DOCUMENT_RAW_TYPE;
		outInfo->quality = 0.4;
		outInfo->capability = 0.7;
		strcpy(outInfo->name, "ProjectConceptor format converted from Freemind");
		strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
	}
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	if ( (outType != P_C_DOCUMENT_RAW_TYPE) &&  (outType != P_C_FREEMIND_TYPE))
		return B_NO_TRANSLATOR;
	Converter	converter(inSource,ioExtension,outDestination);
	if (outType == P_C_DOCUMENT_RAW_TYPE)
		err = converter.ConvertFreeMind2PDoc();
	else
		err = converter.ConvertPDoc2FreeMind();
	inSource->Seek(0, SEEK_SET);
	outDestination->Seek(0, SEEK_SET);
	return err;
}

status_t MakeConfig(BMessage * ioExtension,	BView * * outView, BRect * outExtent)
{
	status_t err	= B_OK;
	*outView = new ConfigView(translatorName);
	return err;
}

status_t GetConfigMessage(BMessage * ioExtension)
{
	status_t err = B_OK;

	return err;
}

Converter::Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination)
{
	in		= inSource;
	config	= ioExtension;
	out		= outDestination;
	//necessary to avoid problems
	out->Seek(0, SEEK_SET);
	in->Seek(0, SEEK_SET);
	allConnections		= NULL;
	allNodes			= NULL;
	selected			= NULL;
}

status_t Converter::ConvertPDoc2FreeMind()
{
	status_t		err					= B_OK;
	BMessage		*inMessage			= new BMessage();
	BMessage		*tmpMessage			= new BMessage();
	int32			id					= 0;

 	allConnections	= new BMessage();
	selected		= new BMessage();
	allNodes		= new BMessage();


	err = inMessage->Unflatten(in);
	if (err == B_OK)
	{
		inMessage->FindMessage("PDocument::allConnections",allConnections);
		inMessage->FindMessage("PDocument::selected",selected);
		inMessage->FindMessage("PDocument::allNodes",allNodes);
		int32 i = 0;
		while(allNodes->FindMessage("node",i,tmpMessage)==B_OK)
		{
			tmpMessage->FindInt32("this",&id);
			nodes[id]=tmpMessage;
			tmpMessage = new BMessage();
			i++;
		}
		i = 0;
		while(allConnections->FindMessage("node",i,tmpMessage)==B_OK)
		{
			tmpMessage->FindInt32("this",&id);
			connections[id]=tmpMessage;
			tmpMessage = new BMessage();
			i++;
		}

		TiXmlDocument	doc;
		TiXmlElement	freeMap("map");
		freeMap.SetAttribute("version","0.9.0");
		freeMap.SetAttribute("background_color","#ffffff");
		TiXmlComment	comment("this File was gernerated by ProjectConceptor! - To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net");
		freeMap.InsertEndChild(comment);

		tmpMessage=GuessStartNode();
	//	tmpMessage = nodes.begin()->second;
		if (tmpMessage != NULL) {
			freeMap.InsertEndChild(ProcessNode(tmpMessage));
			doc.InsertEndChild(freeMap);
		}
		TiXmlPrinter	printer;
//		printer.SetStreamPrinting();
//		printer.SetLineBreak("\n");
//		printer.SetIndent("\t");
		doc.Accept( &printer );
		out->Write(printer.CStr(),strlen(printer.CStr()));
	}
	return err;
}

status_t Converter::ConvertFreeMind2PDoc()
{
	BMessage	document;
	BMessage	allNodes;
	BMessage	allConnections;
	middel.Set(400,400,600,550);
	off_t		size	= in->Seek(0,SEEK_END);
	in->Seek(0,SEEK_SET);
	if (size <= 0)
		return B_NO_TRANSLATOR;
	char		*xmlString	= new char[size+1];
	ssize_t		bytesRead	= in->Read(xmlString, size);
	xmlString[bytesRead > 0 ? bytesRead : 0]	= '\0';
	TiXmlDocument	doc;
	doc.Parse(xmlString);
	delete[] xmlString;
	if (doc.Error())
		return B_ERROR;
	TiXmlNode	*map	= doc.FirstChild("map");
	TiXmlNode	*root	= (map != NULL) ? map->FirstChild("node") : NULL;
	if ((root == NULL) || (root->ToElement() == NULL))
		return B_NO_TRANSLATOR;
	status_t	err		= CreateNode(&allNodes, &allConnections, root->ToElement(), 0, 0);
	if (err != B_OK)
		return err;
	// PDocument::Load() refuses documents without a format version
	document.AddInt32(P_C_DOC_FORMAT_VERSION_FIELD,P_C_DOC_FORMAT_VERSION);
	document.AddMessage("PDocument::allConnections",&allConnections);
	document.AddMessage("PDocument::allNodes",&allNodes);
	return document.Flatten(out);
}


TiXmlElement Converter::ProcessNode(BMessage *node)
{
	int32			i = 0;
	int32			tmpNode			= 0;
	int32			fromNode		= 0;
	int32			toNode			= 0;
	BMessage		*connection		= NULL;
	BMessage		*data			= new BMessage();
	BMessage		*attrib			= new BMessage();

	bool			found			= false;
	char			*name			= NULL;

	TiXmlElement	xmlNode("node");
	node->FindInt32("this", &tmpNode);
	//add this node to the processed List
	processedIDs.insert(tmpNode);
	//find the data field where name and attributes are stored
	node->FindMessage(P_C_NODE_DATA,data);
	data->FindString(P_C_NODE_NAME,(const char **)&name);
	char *str = new char[64];
	sprintf(str, "%ld", (long)tmpNode);
	xmlNode.SetAttribute("ID",str);
	xmlNode.SetAttribute("TEXT",(const char *)name);
	//add all Attributes
	type_code	type	= 0;
	int32		count	= 0;
	#ifdef B_ZETA_VERSION_1_0_0
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	#else
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK)
	#endif
	{
		if ( (data->FindMessage(name,count-1,attrib) == B_OK) && (attrib) )
		{
			char *attribName	= NULL;
			char *value			= NULL;
			attrib->FindString("Name",(const char **)&attribName);
			attrib->FindString("Value",(const char **)&value);
			//**need to hanlde bool
			TiXmlElement	xmlAttrib("attribute");
			xmlAttrib.SetAttribute("NAME",attribName);
			if(value)
				xmlAttrib.SetAttribute("VALUE",value);
			xmlNode.InsertEndChild(xmlAttrib);
		}
		i++;
	}
	//find all outgoing connections
	std::map<int32,BMessage*>::iterator iter;
	iter = connections.begin();
	while (iter!=connections.end())
	{
		connection=(*iter).second;
		connection->FindInt32(P_C_NODE_CONNECTION_FROM,&fromNode);
		connection->FindInt32(P_C_NODE_CONNECTION_TO, &toNode);
		if ((fromNode == tmpNode) && (processedIDs.find((*iter).first) == processedIDs.end()))
		{
			//check if the node was already insert if so we "connect via a arrowlink
			if (processedIDs.find(toNode) != processedIDs.end())
			{
				TiXmlElement	xmlLink("arrowlink");
				char *str = new char[64];
				sprintf(str, "%ld", (long)(*iter).first);
				xmlLink.SetAttribute("ID",str);
				str = new char[64];
				sprintf(str, "%ld", (long)toNode);
				xmlLink.SetAttribute("DESTINATION",str);
				xmlNode.InsertEndChild(xmlLink);
				processedIDs.insert((*iter).first);
			}
			else
			{
				std::map<int32,BMessage*>::iterator	found;
				found = nodes.find(toNode);
				if (found!=nodes.end())
				{
					processedIDs.insert((*iter).first);
					xmlNode.InsertEndChild(ProcessNode((*found).second));
				}
			}
		}
		else if ((toNode == tmpNode) && (processedIDs.find((*iter).first)==processedIDs.end()))
		{
			//check if the node was already insert if so we "connect via a arrowlink
			if (processedIDs.find(fromNode)!=processedIDs.end())
			{
				TiXmlElement	xmlLink("arrowlink");
				char *str = new char[64];
				sprintf(str, "%ld", (long)(*iter).first);
				xmlLink.SetAttribute("ID",str);
				str = new char[64];
				sprintf(str, "%ld", (long)fromNode);
				xmlLink.SetAttribute("DESTINATION",str);
				xmlNode.InsertEndChild(xmlLink);
			}
		}
		iter++;
	}
	return xmlNode;
}

BMessage* Converter::GuessStartNode(void)
{
	BMessage	*connection	= NULL;
	int32		fromNode	= 0;
	int32		toNode		= 0;
	int32		nodeID		= 0;
	bool		found		= false;
	std::set<int32>	visited;
	std::map<int32,BMessage*>::iterator iter;
	if (connections.empty() || nodes.empty())
		return nodes.empty() ? NULL : nodes.begin()->second;
	iter = connections.begin();
	//if there is a node given... we search for a Connection wich points to this node
	connection = (*iter).second;
	if (connection->FindInt32(P_C_NODE_CONNECTION_FROM, &fromNode) == B_OK)
	{
		nodeID = fromNode;
		visited.insert(fromNode);
		found = true;
	}
	while (found && (visited.find(fromNode) == visited.end()))
	{
		found=false;
		iter = connections.begin();
		while ((iter!=connections.end()) && (!found))
		{
			connection=(*iter).second;
			connection->FindInt32(P_C_NODE_CONNECTION_TO,&toNode);
			if (toNode == fromNode)
			{
				visited.insert(fromNode);
				nodeID=fromNode;
				fromNode=toNode;
				found=true;
			}
			iter++;
		}
	}
	iter = nodes.find(nodeID);
	//we didnt find a node while searching through the connections...
	if (iter == nodes.end()){
		iter=nodes.begin();
	}
	return (*iter).second;
}

status_t Converter::CreateNode(BMessage *nodeS,BMessage *connectionS,TiXmlElement *parent,int32 level, int32 thisLine)
{
	TiXmlNode		*node;
	BMessage		*pDocNode	= new BMessage(P_C_CLASS_TYPE);
	BMessage		*data		= new BMessage();
	BMessage		*pattern	= new BMessage();
	int32			line		= 0;
	for (node = parent->FirstChild("node"); node != NULL; node = node->NextSibling("node"))
	{
		status_t	err	= CreateConnection(connectionS, parent,node->ToElement());
		if (err == B_OK)
			err = CreateNode(nodeS,connectionS, node->ToElement(),level+1, line);
		if (err != B_OK)
			return err;
		line++;
	}
	if (parent->Attribute("TEXT"))
		data->AddString(P_C_NODE_NAME,parent->Attribute("TEXT"));
	else
		data->AddString(P_C_NODE_NAME,"Unnamed");
	if (parent->Attribute("ID") == NULL) {
		fprintf(stderr, "FreeMindTranslator: node \"%s\" has no ID\n",
			parent->Attribute("TEXT") ? parent->Attribute("TEXT") : "");
		return B_BAD_DATA;
	}
	pDocNode->AddInt32("this",GetID(parent->Attribute("ID")));
	if (parent->Attribute("CREATED"))
		pDocNode->AddInt32(P_C_NODE_CREATED,atoi(parent->Attribute("CREATED")));
	if (parent->Attribute("MODIFIED"))
		pDocNode->AddInt32(P_C_NODE_MODIFIED,atoi(parent->Attribute("MODIFIED")));
	if (parent->Attribute("BACKGROUND_COLOR"))
		pattern->AddInt32("FillColor",GetRGB(parent->Attribute("BACKGROUND_COLOR")));
	if (parent->Attribute("COLOR"))
		pattern->AddInt32("BorderColor",GetRGB(parent->Attribute("COLOR")));
	for (node = parent->FirstChild("arrowlink"); node != NULL; node = node->NextSibling("arrowlink"))
	{
		status_t	err	= CreateConnection(connectionS,parent,node->ToElement());
		if (err != B_OK)
			return err;
	}
	pDocNode->AddMessage(P_C_NODE_DATA,data);
	// without colors the editor's default pattern applies; a partial
	// pattern would leave PenSize at 0
	if (!pattern->IsEmpty()) {
		pattern->AddFloat("PenSize",1.0);
		pDocNode->AddMessage(P_C_NODE_PATTERN,pattern);
	}
	BRect	*nodeRect	= new BRect(100,100,200,150);
	if (pDocNode->FindRect(P_C_NODE_FRAME,nodeRect) !=  B_OK)
	{
		int32	left, top, right, bottom;
		if (level == 0)
		{
			left	= X_START;
			top		= Y_START;
		}
		else
		{
			if (parent->Attribute("POSITION"))
			{
				if (strcmp(parent->Attribute("POSITION"),"left") != 0)
					left	= (level*(NODE_WIDTH+10))+X_START;
				else
					left	= X_START-(level*(NODE_WIDTH+10));
			}
			top		= (thisLine*(NODE_HEIGHT+10))+ 10;
		}
		right	= left + NODE_WIDTH;
		bottom	= top + NODE_HEIGHT;
		nodeRect->Set(left,top, right, bottom);
		pDocNode->AddRect(P_C_NODE_FRAME,*nodeRect);
	}
	nodeS->AddMessage("node",pDocNode);
	delete pDocNode;
	delete data;
	delete pattern;
	delete nodeRect;
	return B_OK;
}

status_t Converter::CreateConnection(BMessage *container,TiXmlElement *start,TiXmlElement *end)
{
	if ((start == NULL) || (end == NULL))
		return B_BAD_DATA;
	BMessage	connection(P_C_CONNECTION_TYPE);
	BMessage	data;
	const char	*idFrom	= start->Attribute("ID");
	const char	*idTo;
	if (strcmp(end->Value(),"node") == 0) {
		idTo	= end->Attribute("ID");
		data.AddString(P_C_NODE_NAME,"Unnamed");
	} else {
		idTo	= end->Attribute("DESTINATION");
		data.AddString(P_C_NODE_NAME,
			end->Attribute("TEXT") ? end->Attribute("TEXT") : "Unnamed");
	}
	if ((idFrom == NULL) || (idTo == NULL)) {
		fprintf(stderr, "FreeMindTranslator: connection without node ID\n");
		return B_BAD_DATA;
	}
	connection.AddInt32(P_C_NODE_CONNECTION_FROM,GetID(idFrom));
	connection.AddInt32(P_C_NODE_CONNECTION_TO,GetID(idTo));
	connection.AddMessage(P_C_NODE_DATA,&data);
	// PDocLoader::ReIndexConnections looks for the field "node" (singular)
	container->AddMessage("node", &connection);
	return B_OK;
}

int32 Converter::GetID(const char *idString)
{
	// FreeMind ids are strings and may come before or after the node they
	// name (arrow links), so map each one to a number on first sight
	std::map<BString,int32>::iterator	found	= importIds.find(BString(idString));
	if (found != importIds.end())
		return found->second;
	int32	id	= (int32)importIds.size() + 1;
	importIds[BString(idString)]	= id;
	return id;
}

int32 Converter::GetRGB(const char *rgbString)
{
	unsigned int	red = 0, green = 0, blue = 0;
	if (sscanf(rgbString,"#%2x%2x%2x",&red,&green,&blue) != 3)
		fprintf(stderr, "FreeMindTranslator: bad color \"%s\"\n", rgbString);
	rgb_color	rgb	= {(uint8)red, (uint8)green, (uint8)blue, 255};
	int32		packed;
	memcpy(&packed, &rgb, sizeof(packed));
	return packed;
}
