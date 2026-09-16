#include "MacroText.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <app/PropertyInfo.h>
#include <interface/Point.h>
#include <interface/Rect.h>

#include "PCommand.h"
#include "PCommandManager.h"


// ---------------------------------------------------------------- base64 --

static const char kBase64Chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void Base64Encode(const uint8 *data, size_t len, BString *out)
{
	size_t	i	= 0;
	while (i+2 < len) {
		uint32	n	= ((uint32)data[i]<<16) | ((uint32)data[i+1]<<8) | data[i+2];
		out->Append(kBase64Chars[(n>>18)&0x3F],1);
		out->Append(kBase64Chars[(n>>12)&0x3F],1);
		out->Append(kBase64Chars[(n>>6)&0x3F],1);
		out->Append(kBase64Chars[n&0x3F],1);
		i	+= 3;
	}
	size_t	rest	= len-i;
	if (rest == 1) {
		uint32	n	= ((uint32)data[i]<<16);
		out->Append(kBase64Chars[(n>>18)&0x3F],1);
		out->Append(kBase64Chars[(n>>12)&0x3F],1);
		out->Append("==",2);
	} else if (rest == 2) {
		uint32	n	= ((uint32)data[i]<<16) | ((uint32)data[i+1]<<8);
		out->Append(kBase64Chars[(n>>18)&0x3F],1);
		out->Append(kBase64Chars[(n>>12)&0x3F],1);
		out->Append(kBase64Chars[(n>>6)&0x3F],1);
		out->Append("=",1);
	}
}

static int8 Base64Value(char c)
{
	if ((c >= 'A') && (c <= 'Z')) return c-'A';
	if ((c >= 'a') && (c <= 'z')) return c-'a'+26;
	if ((c >= '0') && (c <= '9')) return c-'0'+52;
	if (c == '+') return 62;
	if (c == '/') return 63;
	return -1;
}

static status_t Base64Decode(const char *in, int32 inLen, uint8 **outData, size_t *outLen)
{
	std::vector<uint8>	bytes;
	int32				i		= 0;
	while (i < inLen) {
		int8	v[4]	= {-1,-1,-1,-1};
		int32	got		= 0;
		while ((i < inLen) && (got < 4)) {
			char	c	= in[i++];
			if (c == '=')
				break;
			int8	val	= Base64Value(c);
			if (val < 0)
				return B_BAD_VALUE;
			v[got++]	= val;
		}
		if (got < 2)
			break;
		bytes.push_back((uint8)((v[0]<<2) | (v[1]>>4)));
		if (got >= 3)
			bytes.push_back((uint8)(((v[1]&0xF)<<4) | (v[2]>>2)));
		if (got >= 4)
			bytes.push_back((uint8)(((v[2]&0x3)<<6) | v[3]));
	}
	*outLen		= bytes.size();
	*outData	= new uint8[bytes.size() ? bytes.size() : 1];
	for (size_t j = 0; j < bytes.size(); j++)
		(*outData)[j]	= bytes[j];
	return B_OK;
}


// ---------------------------------------------------------------- format --

static void FormatDouble(double v, BString *out)
{
	char	buf[64];
	snprintf(buf,sizeof(buf),"%g",v);
	BString	s(buf);
	if ((s.FindFirst('.') < 0) && (s.FindFirst('e') < 0) && (s.FindFirst('E') < 0))
		s << ".0";
	*out << s;
}

static void AppendEscapedString(const char *value, BString *out)
{
	*out << "\"";
	for (const char *p = value; *p != '\0'; p++) {
		if ((*p == '"') || (*p == '\\'))
			*out << "\\";
		out->Append(*p,1);
	}
	*out << "\"";
}


// ------------------------------------------------------------- serialize --

static void SerializeValue(BMessage *msg, const char *fieldName, type_code type,
	int32 index, BString *out)
{
	switch (type) {
		case B_BOOL_TYPE: {
			bool	v	= false;
			msg->FindBool(fieldName,index,&v);
			*out << (v ? "true" : "false");
			break;
		}
		case B_INT8_TYPE: {
			int8	v	= 0;
			msg->FindInt8(fieldName,index,&v);
			*out << (int32)v << "i8";
			break;
		}
		case B_INT16_TYPE: {
			int16	v	= 0;
			msg->FindInt16(fieldName,index,&v);
			*out << (int32)v << "i16";
			break;
		}
		case B_INT32_TYPE: {
			int32	v	= 0;
			msg->FindInt32(fieldName,index,&v);
			// "node" is the one field name every command uses for an
			// already-indexed node/connection id (see Indexer::IndexCommand())
			// - marked with @ so it reads as a reference, not a plain number.
			if (strcmp(fieldName,"node") == 0)
				*out << "@" << v;
			else
				*out << v;
			break;
		}
		case B_INT64_TYPE: {
			int64	v	= 0;
			msg->FindInt64(fieldName,index,&v);
			*out << v << "i64";
			break;
		}
		case B_FLOAT_TYPE: {
			float	v	= 0;
			msg->FindFloat(fieldName,index,&v);
			FormatDouble((double)v,out);
			break;
		}
		case B_DOUBLE_TYPE: {
			double	v	= 0;
			msg->FindDouble(fieldName,index,&v);
			FormatDouble(v,out);
			*out << "d";
			break;
		}
		case B_STRING_TYPE: {
			const char	*v	= NULL;
			msg->FindString(fieldName,index,&v);
			AppendEscapedString(v ? v : "",out);
			break;
		}
		case B_POINT_TYPE: {
			BPoint	p;
			msg->FindPoint(fieldName,index,&p);
			BString	x,y;
			FormatDouble(p.x,&x);
			FormatDouble(p.y,&y);
			*out << "(" << x << "," << y << ")";
			break;
		}
		case B_RECT_TYPE: {
			BRect	r;
			msg->FindRect(fieldName,index,&r);
			BString	l,t,rr,b;
			FormatDouble(r.left,&l);
			FormatDouble(r.top,&t);
			FormatDouble(r.right,&rr);
			FormatDouble(r.bottom,&b);
			*out << "[" << l << "," << t << "," << rr << "," << b << "]";
			break;
		}
		default: {
			// Escape hatch - nested BMessage fields (e.g. ChangeValue's
			// valueContainer) and any type not listed above. Lossless,
			// opaque, not meant to be hand-authored - never silently drops
			// data (see MacroText.h).
			const void	*data	= NULL;
			ssize_t		size	= 0;
			if (msg->FindData(fieldName,type,index,&data,&size) == B_OK) {
				BString	b64;
				Base64Encode((const uint8*)data,(size_t)size,&b64);
				*out << "raw:" << (uint32)type << ":" << b64;
			}
			break;
		}
	}
}


static void SerializeCommand(BMessage *command, int depth, BString *out)
{
	BString	indent;
	for (int d = 0; d < depth; d++)
		indent << "  ";

	const char	*name	= NULL;
	command->FindString("Command::Name",&name);
	if (name == NULL)
		name	= "";
	*out << indent << name;

	BString	undoFieldName;
	undoFieldName << name << "::Undo";

	BList		children;	// BMessage* - subPCommand entries, rendered after this line
	char		*fieldName;
	type_code	type;
	int32		count;
	int32		i		= 0;
	while (command->GetInfo(B_ANY_TYPE,i,&fieldName,&type,&count) == B_OK) {
		BString	fn(fieldName);
		if ((fn == "Command::Name") || (fn == undoFieldName)) {
			i++;
			continue;
		}
		if (fn == "PCommand::subPCommand") {
			for (int32 j = 0; j < count; j++) {
				BMessage	*child	= new BMessage();
				if (command->FindMessage(fieldName,j,child) == B_OK)
					children.AddItem(child);
				else
					delete child;
			}
			i++;
			continue;
		}
		for (int32 j = 0; j < count; j++) {
			*out << " " << fieldName << "=";
			SerializeValue(command,fieldName,type,j,out);
		}
		i++;
	}
	*out << "\n";

	for (int32 c = 0; c < children.CountItems(); c++) {
		BMessage	*child	= (BMessage*)children.ItemAt(c);
		SerializeCommand(child,depth+1,out);
		delete child;
	}
}


void SerializeCommands(BList *commands, BString *outText)
{
	outText->SetTo("");
	for (int32 i = 0; i < commands->CountItems(); i++) {
		BMessage	*cmd	= (BMessage*)commands->ItemAt(i);
		if (cmd != NULL)
			SerializeCommand(cmd,0,outText);
	}
}


// ------------------------------------------------------------------ parse --

static void Tokenize(const BString &rest, BList *tokens)
{
	int32		i	= 0;
	int32		len	= rest.Length();
	const char	*s	= rest.String();
	while (i < len) {
		while ((i < len) && isspace((unsigned char)s[i]))
			i++;
		if (i >= len)
			break;
		int32	start		= i;
		bool	inQuotes	= false;
		while (i < len) {
			char	c	= s[i];
			if (inQuotes) {
				if ((c == '\\') && (i+1 < len)) { i += 2; continue; }
				if (c == '"') { inQuotes = false; i++; continue; }
				i++;
			} else {
				if (isspace((unsigned char)c))
					break;
				if (c == '"') { inQuotes = true; i++; continue; }
				i++;
			}
		}
		BString	*tok	= new BString();
		rest.CopyInto(*tok,start,i-start);
		tokens->AddItem(tok);
	}
}


static void DeleteStringList(BList *list)
{
	for (int32 i = 0; i < list->CountItems(); i++)
		delete (BString*)list->ItemAt(i);
	list->MakeEmpty();
}


/** Looks up fieldName in command's own PropertyInfo() ctypes - returns
 * true and the declared type_code if found. */
static bool FindFieldType(PCommand *command, const char *fieldName, type_code *outType)
{
	int32				count	= 0;
	const property_info	*props	= command->PropertyInfo(&count);
	for (int32 p = 0; p < count; p++) {
		for (int32 c = 0; c < 3; c++) {
			for (int32 f = 0; f < 5; f++) {
				const char	*pairName	= props[p].ctypes[c].pairs[f].name;
				if (pairName == NULL)
					continue;
				if (strcmp(pairName,fieldName) == 0) {
					*outType	= props[p].ctypes[c].pairs[f].type;
					return true;
				}
			}
		}
	}
	return false;
}


static bool UnescapeQuoted(const BString &token, BString *outValue)
{
	int32	len	= token.Length();
	if ((len < 2) || (token[0] != '"') || (token[len-1] != '"'))
		return false;
	const char	*s	= token.String();
	outValue->SetTo("");
	for (int32 i = 1; i < len-1; i++) {
		if ((s[i] == '\\') && (i+1 < len-1)) {
			outValue->Append(s[i+1],1);
			i++;
		} else
			outValue->Append(s[i],1);
	}
	return true;
}


static status_t ParseAndAddValue(BMessage *cmd, const char *fieldName, type_code expectedType,
	const BString &valueText, BString *errorOut)
{
	if (valueText.Length() == 0) {
		*errorOut	<< "field \"" << fieldName << "\" has no value";
		return B_BAD_VALUE;
	}

	if (expectedType == B_POINTER_TYPE) {
		if ((valueText.Length() < 2) || (valueText[0] != '@')) {
			*errorOut	<< "field \"" << fieldName << "\" needs an @<id> reference, got \"" << valueText << "\"";
			return B_BAD_VALUE;
		}
		BString	digits;
		valueText.CopyInto(digits,1,valueText.Length()-1);
		cmd->AddInt32(fieldName,(int32)atol(digits.String()));
		return B_OK;
	}

	if (valueText.StartsWith("raw:")) {
		int32	firstColon	= valueText.FindFirst(':',4);
		if (firstColon < 0) {
			*errorOut	<< "malformed raw: token for field \"" << fieldName << "\"";
			return B_BAD_VALUE;
		}
		BString	typeText;
		valueText.CopyInto(typeText,4,firstColon-4);
		uint32	typeCode	= (uint32)strtoul(typeText.String(),NULL,10);
		if (typeCode != expectedType) {
			*errorOut	<< "field \"" << fieldName << "\": raw type does not match schema";
			return B_BAD_VALUE;
		}
		BString	b64;
		valueText.CopyInto(b64,firstColon+1,valueText.Length()-firstColon-1);
		uint8	*data	= NULL;
		size_t	size	= 0;
		if (Base64Decode(b64.String(),b64.Length(),&data,&size) != B_OK) {
			*errorOut	<< "field \"" << fieldName << "\": malformed base64";
			return B_BAD_VALUE;
		}
		cmd->AddData(fieldName,(type_code)typeCode,data,(ssize_t)size);
		delete[] data;
		return B_OK;
	}

	if (valueText[0] == '"') {
		if (expectedType != B_STRING_TYPE) {
			*errorOut	<< "field \"" << fieldName << "\" is not a string field";
			return B_BAD_VALUE;
		}
		BString	value;
		if (!UnescapeQuoted(valueText,&value)) {
			*errorOut	<< "field \"" << fieldName << "\": unterminated string";
			return B_BAD_VALUE;
		}
		cmd->AddString(fieldName,value);
		return B_OK;
	}

	if ((valueText == "true") || (valueText == "false")) {
		if (expectedType != B_BOOL_TYPE) {
			*errorOut	<< "field \"" << fieldName << "\" is not a bool field";
			return B_BAD_VALUE;
		}
		cmd->AddBool(fieldName,valueText == "true");
		return B_OK;
	}

	if ((valueText.Length() >= 2) && (valueText[0] == '(') && (valueText[valueText.Length()-1] == ')')) {
		if (expectedType != B_POINT_TYPE) {
			*errorOut	<< "field \"" << fieldName << "\" is not a point field";
			return B_BAD_VALUE;
		}
		BString	inner;
		valueText.CopyInto(inner,1,valueText.Length()-2);
		int32	comma	= inner.FindFirst(',');
		if (comma < 0) {
			*errorOut	<< "field \"" << fieldName << "\": malformed point";
			return B_BAD_VALUE;
		}
		BString	xs,ys;
		inner.CopyInto(xs,0,comma);
		inner.CopyInto(ys,comma+1,inner.Length()-comma-1);
		cmd->AddPoint(fieldName,BPoint(atof(xs.String()),atof(ys.String())));
		return B_OK;
	}

	if ((valueText.Length() >= 2) && (valueText[0] == '[') && (valueText[valueText.Length()-1] == ']')) {
		if (expectedType != B_RECT_TYPE) {
			*errorOut	<< "field \"" << fieldName << "\" is not a rect field";
			return B_BAD_VALUE;
		}
		BString	inner;
		valueText.CopyInto(inner,1,valueText.Length()-2);
		float	parts[4];
		int32	partIndex	= 0;
		int32	start		= 0;
		for (int32 i = 0; (i <= inner.Length()) && (partIndex < 4); i++) {
			if ((i == inner.Length()) || (inner[i] == ',')) {
				BString	part;
				inner.CopyInto(part,start,i-start);
				parts[partIndex++]	= atof(part.String());
				start	= i+1;
			}
		}
		if (partIndex != 4) {
			*errorOut	<< "field \"" << fieldName << "\": malformed rect (need 4 values)";
			return B_BAD_VALUE;
		}
		cmd->AddRect(fieldName,BRect(parts[0],parts[1],parts[2],parts[3]));
		return B_OK;
	}

	// numeric: optional trailing i8/i16/i64/d suffix, else int32 (no dot)
	// or float (has a dot)
	BString	numText(valueText);
	if (numText.EndsWith("i64")) {
		numText.Truncate(numText.Length()-3);
		if (expectedType != B_INT64_TYPE) { *errorOut << "field \"" << fieldName << "\" is not an int64 field"; return B_BAD_VALUE; }
		cmd->AddInt64(fieldName,strtoll(numText.String(),NULL,10));
		return B_OK;
	}
	if (numText.EndsWith("i16")) {
		numText.Truncate(numText.Length()-3);
		if (expectedType != B_INT16_TYPE) { *errorOut << "field \"" << fieldName << "\" is not an int16 field"; return B_BAD_VALUE; }
		cmd->AddInt16(fieldName,(int16)atol(numText.String()));
		return B_OK;
	}
	if (numText.EndsWith("i8")) {
		numText.Truncate(numText.Length()-2);
		if (expectedType != B_INT8_TYPE) { *errorOut << "field \"" << fieldName << "\" is not an int8 field"; return B_BAD_VALUE; }
		cmd->AddInt8(fieldName,(int8)atol(numText.String()));
		return B_OK;
	}
	if (numText.EndsWith("d")) {
		numText.Truncate(numText.Length()-1);
		if (expectedType != B_DOUBLE_TYPE) { *errorOut << "field \"" << fieldName << "\" is not a double field"; return B_BAD_VALUE; }
		cmd->AddDouble(fieldName,strtod(numText.String(),NULL));
		return B_OK;
	}
	if (numText.FindFirst('.') >= 0) {
		if (expectedType != B_FLOAT_TYPE) { *errorOut << "field \"" << fieldName << "\" is not a float field"; return B_BAD_VALUE; }
		cmd->AddFloat(fieldName,(float)strtod(numText.String(),NULL));
		return B_OK;
	}
	// plain integer -> int32
	bool	looksNumeric	= numText.Length() > 0;
	for (int32 i = 0; i < numText.Length(); i++) {
		char	c	= numText[i];
		if (!(isdigit((unsigned char)c) || ((i == 0) && (c == '-'))))
			looksNumeric	= false;
	}
	if (!looksNumeric) {
		*errorOut	<< "field \"" << fieldName << "\": unrecognized value \"" << valueText << "\"";
		return B_BAD_VALUE;
	}
	if (expectedType != B_INT32_TYPE) {
		*errorOut	<< "field \"" << fieldName << "\" is not an int32 field";
		return B_BAD_VALUE;
	}
	cmd->AddInt32(fieldName,(int32)atol(numText.String()));
	return B_OK;
}


struct MacroStackFrame {
	BMessage	*cmd;
	int			depth;
	BList		*children;
};


static void CleanupFrames(std::vector<MacroStackFrame> &stack, BList *built)
{
	for (size_t i = 0; i < stack.size(); i++) {
		for (int32 j = 0; j < stack[i].children->CountItems(); j++)
			delete (BMessage*)stack[i].children->ItemAt(j);
		delete stack[i].children;
		delete stack[i].cmd;
	}
	stack.clear();
	for (int32 i = 0; i < built->CountItems(); i++)
		delete (BMessage*)built->ItemAt(i);
	built->MakeEmpty();
}


status_t ParseCommands(const BString &text, BList *outCommands, PCommandManager *registry,
	BString *errorOut)
{
	BList						built;
	std::vector<MacroStackFrame>	stack;

	int32	pos		= 0;
	int32	len		= text.Length();
	int32	lineNo	= 0;

	while (pos <= len) {
		int32	lineStart	= pos;
		int32	lineEnd		= lineStart;
		while ((lineEnd < len) && (text[lineEnd] != '\n'))
			lineEnd++;
		BString	line;
		text.CopyInto(line,lineStart,lineEnd-lineStart);
		pos	= lineEnd+1;
		lineNo++;
		if ((line.Length() > 0) && (line[line.Length()-1] == '\r'))
			line.Truncate(line.Length()-1);

		if (pos-1 > len)
			break;

		// blank or comment line - skip, no depth/tree effect
		int32	firstNonSpace	= 0;
		while ((firstNonSpace < line.Length()) && (line[firstNonSpace] == ' '))
			firstNonSpace++;
		if ((firstNonSpace >= line.Length()) || (line[firstNonSpace] == '#')) {
			if (lineEnd >= len)
				break;
			continue;
		}

		int32	depth	= firstNonSpace/2;

		BString	rest;
		line.CopyInto(rest,firstNonSpace,line.Length()-firstNonSpace);

		BList	tokens;
		Tokenize(rest,&tokens);
		if (tokens.CountItems() == 0) {
			if (lineEnd >= len)
				break;
			continue;
		}

		BString	commandName(*(BString*)tokens.ItemAt(0));
		PCommand	*command	= registry->GetPCommand((char*)commandName.String());
		if (command == NULL) {
			errorOut->SetTo("");
			*errorOut	<< "line " << lineNo << ": unknown command \"" << commandName << "\"";
			DeleteStringList(&tokens);
			CleanupFrames(stack,&built);
			return B_NAME_NOT_FOUND;
		}

		BMessage	*cmd	= new BMessage();
		cmd->AddString("Command::Name",commandName);

		status_t	fieldErr	= B_OK;
		for (int32 t = 1; t < tokens.CountItems(); t++) {
			BString	*tok	= (BString*)tokens.ItemAt(t);
			int32	eq		= tok->FindFirst('=');
			if (eq < 0) {
				errorOut->SetTo("");
				*errorOut	<< "line " << lineNo << ": malformed token \"" << *tok << "\" (expected field=value)";
				fieldErr	= B_BAD_VALUE;
				break;
			}
			BString	fieldName,valueText;
			tok->CopyInto(fieldName,0,eq);
			tok->CopyInto(valueText,eq+1,tok->Length()-eq-1);

			type_code	expectedType;
			if (!FindFieldType(command,fieldName.String(),&expectedType)) {
				errorOut->SetTo("");
				*errorOut	<< "line " << lineNo << ": " << commandName << " has no field \"" << fieldName << "\"";
				fieldErr	= B_BAD_VALUE;
				break;
			}
			BString	valueError;
			if (ParseAndAddValue(cmd,fieldName.String(),expectedType,valueText,&valueError) != B_OK) {
				errorOut->SetTo("");
				*errorOut	<< "line " << lineNo << ": " << valueError;
				fieldErr	= B_BAD_VALUE;
				break;
			}
		}
		DeleteStringList(&tokens);
		if (fieldErr != B_OK) {
			delete cmd;
			CleanupFrames(stack,&built);
			return fieldErr;
		}

		// pop frames back to (and including) the first one whose depth is
		// >= this line's depth - each popped frame's pending children get
		// attached now (AddMessage() copies, so a frame's children must
		// all be known before it is attached anywhere itself)
		while ((!stack.empty()) && (stack.back().depth >= depth)) {
			MacroStackFrame	frame	= stack.back();
			stack.pop_back();
			for (int32 c = 0; c < frame.children->CountItems(); c++) {
				BMessage	*child	= (BMessage*)frame.children->ItemAt(c);
				frame.cmd->AddMessage("PCommand::subPCommand",child);
				delete child;
			}
			delete frame.children;
			if (stack.empty())
				built.AddItem(frame.cmd);
			else
				stack.back().children->AddItem(frame.cmd);
		}

		int32	expectedDepth	= stack.empty() ? 0 : stack.back().depth+1;
		if (depth != expectedDepth) {
			errorOut->SetTo("");
			*errorOut	<< "line " << lineNo << ": unexpected indent";
			delete cmd;
			CleanupFrames(stack,&built);
			return B_BAD_VALUE;
		}

		MacroStackFrame	newFrame;
		newFrame.cmd		= cmd;
		newFrame.depth		= depth;
		newFrame.children	= new BList();
		stack.push_back(newFrame);

		if (lineEnd >= len)
			break;
	}

	while (!stack.empty()) {
		MacroStackFrame	frame	= stack.back();
		stack.pop_back();
		for (int32 c = 0; c < frame.children->CountItems(); c++) {
			BMessage	*child	= (BMessage*)frame.children->ItemAt(c);
			frame.cmd->AddMessage("PCommand::subPCommand",child);
			delete child;
		}
		delete frame.children;
		if (stack.empty())
			built.AddItem(frame.cmd);
		else
			stack.back().children->AddItem(frame.cmd);
	}

	for (int32 i = 0; i < built.CountItems(); i++)
		outCommands->AddItem(built.ItemAt(i));
	return B_OK;
}
