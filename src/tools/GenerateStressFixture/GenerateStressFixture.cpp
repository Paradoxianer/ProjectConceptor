// Stress-test fixture generator (250+ nodes) - builds a real document via
// the actual Indexer/PDocument::Archive() path and flattens it exactly like
// PDocument::Save()'s no-explicit-translator fallback does, rather than
// hand-writing the on-disk format (which is raw flattened BMessage/"HMF1",
// not XML - only ConfigManager's settings file uses MessageXmlWriter).
// See docs/notes.md for the headless-PDocument pattern this reuses.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <app/Application.h>
#include <app/Message.h>
#include <interface/GraphicsDefs.h>
#include <interface/Rect.h>
#include <storage/File.h>
#include <support/List.h>

#include "PDocument.h"
#include "PDocumentManager.h"
#include "ProjectConceptorDefs.h"

const char *GEN_APP_SIGNATURE = "application/x-vnd.ProjectConceptorStressGen";

// field values verified against a real node dumped by the running app
// (PrintToStream() output captured during this session), not guessed
static BMessage* NewNode(int32 index,BPoint origin)
{
	BMessage	*data	= new BMessage();
	char		name[32];
	sprintf(name,"Node %ld",(long)index);
	data->AddString(P_C_NODE_NAME,name);

	BMessage	*font	= new BMessage('fOTy');
	font->AddInt8("Font::Encoding",0);
	font->AddInt16("Font::Face",0x40);
	font->AddString("Font::Family","Noto Sans");
	font->AddInt32("Font::Flags",0);
	font->AddFloat("Font::Rotation",0.0);
	font->AddFloat("Font::Shear",90.0);
	font->AddFloat("Font::Size",12.0);
	font->AddInt8("Font::Spacing",2);
	font->AddString("Font::Style","Regular");
	font->AddInt32("Font::Color",(int32)0xffb5976f);

	BMessage	*pattern	= new BMessage();
	rgb_color	fillColor	= {152,180,190,255};
	pattern->AddInt32("FillColor",*(int32*)&fillColor);
	rgb_color	borderColor	= {0,0,0,255};
	pattern->AddInt32("BorderColor",*(int32*)&borderColor);
	pattern->AddFloat("PenSize",1.0);
	pattern->AddInt8("DrawingMode",B_OP_ALPHA);
	rgb_color	highColor	= {0,0,0,255};
	pattern->AddInt32("HighColor",*(int32*)&highColor);
	rgb_color	lowColor	= {128,128,128,255};
	pattern->AddInt32("LowColor",*(int32*)&lowColor);
	pattern->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH),false);

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddMessage(P_C_NODE_DATA,data);
	node->AddRect(P_C_NODE_FRAME,BRect(origin,origin+BPoint(100,40)));
	node->AddMessage(P_C_NODE_FONT,font);
	node->AddMessage(P_C_NODE_PATTERN,pattern);
	node->AddBool(P_C_NODE_SELECTED,false);
	return node;
}

static BMessage* NewConnection(BMessage *from,BMessage *to)
{
	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	connection->AddPointer(P_C_NODE_CONNECTION_FROM,from);
	connection->AddPointer(P_C_NODE_CONNECTION_TO,to);
	BMessage	*data	= new BMessage();
	data->AddString(P_C_NODE_NAME,"connection");
	connection->AddMessage(P_C_NODE_DATA,data);
	connection->AddInt8(P_C_NODE_CONNECTION_TYPE,1);
	return connection;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr,"usage: %s <output.pcd> [nodeCount]\n",argv[0]);
		return 1;
	}
	int32	nodeCount	= (argc > 2) ? atoi(argv[2]) : 2200;
	const int32	columns	= 20;

	BApplication		app(GEN_APP_SIGNATURE);
	PDocumentManager	*docManager	= new PDocumentManager(new BMessage());
	PDocument			*doc		= new PDocument(docManager,true);

	BList	*allNodes	= doc->GetAllNodes();
	for (int32 i = 0; i < nodeCount; i++) {
		BPoint	origin(120.0 * (i % columns),100.0 * (i / columns));
		allNodes->AddItem(NewNode(i,origin));
	}

	// each node to its row-neighbour, plus one link per row down to the
	// next row, so it's a connected graph rather than an inert grid
	BList	*allConnections	= doc->GetAllConnections();
	for (int32 i = 0; i < nodeCount - 1; i++) {
		if ((i % columns) == (columns - 1))
			continue;
		allConnections->AddItem(NewConnection((BMessage*)allNodes->ItemAt(i),(BMessage*)allNodes->ItemAt(i + 1)));
	}
	for (int32 i = columns; i < nodeCount; i += columns)
		allConnections->AddItem(NewConnection((BMessage*)allNodes->ItemAt(i - columns),(BMessage*)allNodes->ItemAt(i)));

	// a handful of groups, each claiming one row of nodes - exercises the
	// P_C_NODE_PARENT/P_C_NODE_ALLNODES bookkeeping at scale (Indexer's
	// grouped-node roundtrip, issue #68)
	int32	groupCount	= 0;
	for (int32 g = 0; (g * columns) < nodeCount; g++) {
		BMessage	*groupData	= new BMessage();
		char		groupName[32];
		sprintf(groupName,"Group %ld",(long)g);
		groupData->AddString(P_C_NODE_NAME,groupName);
		BMessage	*group	= new BMessage(P_C_GROUP_TYPE);
		group->AddMessage(P_C_NODE_DATA,groupData);
		group->AddRect(P_C_NODE_FRAME,BRect(0,100.0 * g,columns * 120.0,100.0 * g + 60));
		BList	*groupAllNodes	= new BList();
		group->AddPointer(P_C_NODE_ALLNODES,groupAllNodes);
		int32	rowStart	= g * columns;
		int32	rowEnd		= (rowStart + columns < nodeCount) ? (rowStart + columns) : nodeCount;
		for (int32 i = rowStart; i < rowEnd; i++) {
			BMessage	*child	= (BMessage*)allNodes->ItemAt(i);
			child->AddPointer(P_C_NODE_PARENT,group);
			groupAllNodes->AddItem(child);
		}
		allNodes->AddItem(group);
		groupCount++;
	}

	BMessage	archive;
	doc->Archive(&archive,true);

	BFile		file(argv[1],B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
	status_t	err	= archive.Flatten(&file);
	if (err != B_OK) {
		fprintf(stderr,"Flatten failed: %s\n",strerror(err));
		return 1;
	}
	printf("Wrote %ld nodes, %ld connections, %ld groups to %s\n",
		(long)nodeCount,(long)allConnections->CountItems(),(long)groupCount,argv[1]);
	return 0;
}
