#include "MacroOutlineView.h"

#include <Catalog.h>
#include <app/PropertyInfo.h>
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/ListItem.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <interface/Window.h>
#include <support/String.h>

#include <Beep.h>
#include <algorithm>
#include <set>
#include <stdlib.h>
#include <string.h>

#include "MacroEditor.h"
#include "MacroText.h"
#include "PCommand.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MacroEditor"

const char* const kCommandSnippetDragMarker	= "pc:command_snippet";

// Commands that hold their own children as "PCommand::subPCommand" -
// dropping a snippet onto one of these nests it, dropping onto anything
// else makes it a sibling. No reflective way to ask a PCommand "do you
// run subcommands" exists in this codebase, so this is the same small,
// explicit list BuildCommandSnippet()/CommandExampleText() already assume.
static const char *kContainerCommandNames[] = { "Batch", "Repeat", "ForEach", "If" };

static bool IsContainerCommandName(const char *name)
{
	if (name == NULL)
		return false;
	for (size_t i = 0; i < sizeof(kContainerCommandNames)/sizeof(kContainerCommandNames[0]); i++)
		if (strcmp(kContainerCommandNames[i],name) == 0)
			return true;
	return false;
}


/** The label an embedded node/connection chip shows - the node's own name,
 * or "Connection" - never blank, so a chip is never just an empty pill. */
static void ChipLabel(BMessage &nested, BString *label)
{
	if (nested.what == P_C_CONNECTION_TYPE) {
		*label	= B_TRANSLATE("Connection");
		return;
	}
	BMessage	data;
	if (nested.FindMessage(P_C_NODE_DATA,&data) == B_OK) {
		const char	*name	= NULL;
		if (data.FindString(P_C_NODE_NAME,&name) == B_OK) {
			*label	= name;
			return;
		}
	}
	*label	= "{...}";
}


static BString PathKey(int32 topLevelIndex, const MacroPath &path)
{
	BString	key;
	key << topLevelIndex;
	for (size_t i = 0; i < path.size(); i++)
		key << "/" << path[i].field << "#" << path[i].index;
	return key;
}


// -------------------------------------------------------------- row item --

enum RowKind { kRowCommand, kRowField, kRowChip, kRowBlock, kRowAddField };

/** One outline row - see MacroOutlineView.h's class comment for the four
 * kinds. Every row knows exactly where its own data lives (containerPath +
 * fieldName + fieldIndex, resolved against fCommands[topLevelIndex] - see
 * MacroOutlineView::ResolveContainer()), so committing an edit never needs
 * to re-derive that from the row's position in the list. */
class MacroRowItem : public BStringItem
{
public:
	MacroRowItem(const char *text, uint32 level, bool expanded, RowKind kind,
		int32 topLevelIndex, const MacroPath &containerPath, const char *fieldName,
		int32 fieldIndex, type_code fieldType, bool allowBinding, PCommand *command,
		int32 valueStart)
		: BStringItem(text,level,expanded),
			fKind(kind), fTopLevelIndex(topLevelIndex), fContainerPath(containerPath),
			fFieldName(fieldName ? fieldName : ""), fFieldIndex(fieldIndex),
			fFieldType(fieldType), fAllowBinding(allowBinding), fCommand(command),
			fValueStart(valueStart) {}

	RowKind				Kind(void) const {return fKind;}
	int32				TopLevelIndex(void) const {return fTopLevelIndex;}
	const MacroPath&	ContainerPath(void) const {return fContainerPath;}
	const BString&		FieldName(void) const {return fFieldName;}
	int32				FieldIndex(void) const {return fFieldIndex;}
	type_code			FieldType(void) const {return fFieldType;}
	bool				AllowBinding(void) const {return fAllowBinding;}
	PCommand*			Command(void) const {return fCommand;}

	/** The path this row's own children (if any) are rooted on - itself,
	 * as a container: containerPath plus this row's own field slot. Only
	 * meaningful for kRowCommand/kRowChip/kRowBlock. */
	MacroPath	SelfPath(void) const
	{
		MacroPath	path(fContainerPath);
		if (fFieldName.Length() > 0) {
			MacroPathStep	step;
			step.field	= fFieldName;
			step.index	= fFieldIndex;
			path.push_back(step);
		}
		return path;
	}

	virtual void	DrawItem(BView *owner, BRect frame, bool complete = false);

private:
	RowKind		fKind;
	int32		fTopLevelIndex;
	MacroPath	fContainerPath;
	BString		fFieldName;
	int32		fFieldIndex;
	type_code	fFieldType;
	bool		fAllowBinding;
	PCommand	*fCommand;		// not owned - the registry's own instance
	/** Offset into Text() where the value portion starts, for a kRowField
	 * row's two-tone "name: value" draw - -1 for every other kind. */
	int32		fValueStart;
};


void MacroRowItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	rgb_color	background	= IsSelected() ? ui_color(B_LIST_SELECTED_BACKGROUND_COLOR)
		: owner->ViewColor();
	owner->SetHighColor(background);
	owner->FillRect(frame);

	rgb_color	textColor	= IsSelected() ? ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR)
		: ui_color(B_LIST_ITEM_TEXT_COLOR);
	rgb_color	dimColor	= {120,120,120,255};

	font_height	fh;
	owner->GetFontHeight(&fh);
	float	baseline	= frame.top + (frame.Height()-(fh.ascent+fh.descent))/2 + fh.ascent;

	if (fKind == kRowChip) {
		rgb_color	fill	= {209,224,247,255};
		float	width	= owner->StringWidth(Text());
		BRect	pill(frame.left+2,frame.top+2,frame.left+18+width,frame.bottom-2);
		owner->SetHighColor(fill);
		owner->FillRoundRect(pill,6,6);
		owner->SetHighColor(textColor);
		owner->DrawString(Text(),BPoint(pill.left+8,baseline));
		return;
	}

	BFont	font(be_plain_font);
	if (fKind == kRowCommand)
		font.SetFace(B_BOLD_FACE);
	else if (fKind == kRowBlock)
		font.SetFace(B_ITALIC_FACE);
	owner->SetFont(&font);

	int32	textLen	= (Text() != NULL) ? (int32)strlen(Text()) : 0;
	if ((fKind == kRowField) && (fValueStart > 0) && (fValueStart <= textLen)) {
		BString	label(Text());
		BString	value;
		label.CopyInto(value,fValueStart,label.Length()-fValueStart);
		label.Truncate(fValueStart);
		owner->SetHighColor(IsSelected() ? textColor : dimColor);
		owner->DrawString(label.String(),BPoint(frame.left+2,baseline));
		owner->SetHighColor(textColor);
		owner->DrawString(value.String(),BPoint(frame.left+2+owner->StringWidth(label.String()),baseline));
	} else {
		owner->SetHighColor((fKind == kRowAddField) ? dimColor
			: ((fKind == kRowBlock) && !IsSelected()) ? dimColor : textColor);
		owner->DrawString(Text(),BPoint(frame.left+2,baseline));
	}
	owner->SetFont(be_plain_font);
}


// ------------------------------------------------------- BMessage helpers --

/** Every entry of `field` on `container`, in order - the only way to
 * change ORDER among same-named entries (add/remove/swap at a position),
 * since BMessage itself only appends. */
static void ExtractMessageEntries(BMessage *container, const char *field,
	std::vector<BMessage> *entries)
{
	BMessage	entry;
	for (int32 i = 0; container->FindMessage(field,i,&entry) == B_OK; i++) {
		entries->push_back(entry);
		entry.MakeEmpty();
	}
}


static void ReplaceMessageEntries(BMessage *container, const char *field,
	const std::vector<BMessage> &entries)
{
	container->RemoveName(field);
	for (size_t i = 0; i < entries.size(); i++)
		container->AddMessage(field,(BMessage*)&entries[i]);
}


/** field-per-field zero value for a freshly added scalar field. */
static void AddZeroValue(BMessage *msg, const char *field, type_code type)
{
	switch (type) {
		case B_BOOL_TYPE:		msg->AddBool(field,false); break;
		case B_INT8_TYPE:		msg->AddInt8(field,0); break;
		case B_INT16_TYPE:		msg->AddInt16(field,0); break;
		case B_INT32_TYPE:		msg->AddInt32(field,0); break;
		case B_INT64_TYPE:		msg->AddInt64(field,0); break;
		case B_FLOAT_TYPE:		msg->AddFloat(field,0.0f); break;
		case B_DOUBLE_TYPE:		msg->AddDouble(field,0.0); break;
		case B_POINT_TYPE:		msg->AddPoint(field,BPoint(0,0)); break;
		case B_RECT_TYPE:		msg->AddRect(field,BRect(0,0,0,0)); break;
		case B_POINTER_TYPE:	msg->AddInt32(field,0); break;
		case B_STRING_TYPE:
		default:				msg->AddString(field,""); break;
	}
}


static const char* TypeDisplayName(type_code type)
{
	switch (type) {
		case B_BOOL_TYPE:		return "bool";
		case B_INT8_TYPE:		return "int8";
		case B_INT16_TYPE:		return "int16";
		case B_INT32_TYPE:		return "int32";
		case B_INT64_TYPE:		return "int64";
		case B_FLOAT_TYPE:		return "float";
		case B_DOUBLE_TYPE:	return "double";
		case B_STRING_TYPE:	return "string";
		case B_POINT_TYPE:		return "point";
		case B_RECT_TYPE:		return "rect";
		case B_POINTER_TYPE:	return "@id";
		case B_MESSAGE_TYPE:	return "block";
		default:				return "raw";
	}
}


// -------------------------------------------------- overlay text control --

const uint32	kOverlayCommit	= 'mvOc';

/** The inline value-edit control MacroOutlineView overlays on a field row
 * (Tracker's icon-rename pattern) - commits on Enter (BTextControl's own
 * default) or on losing focus, cancels on Escape. */
class MacroOverlayControl : public BTextControl
{
public:
	MacroOverlayControl(BRect frame, const char *text, MacroOutlineView *outline)
		: BTextControl(frame,"overlay",NULL,text,new BMessage(kOverlayCommit)),
			fOutline(outline), fClosing(false)
	{
		SetTarget(outline);
	}

	virtual void MakeFocus(bool focused = true)
	{
		BTextControl::MakeFocus(focused);
		if (!focused && !fClosing) {
			fClosing	= true;
			Invoke();
		}
	}

	virtual void KeyDown(const char *bytes, int32 numBytes)
	{
		if ((numBytes == 1) && (bytes[0] == B_ESCAPE)) {
			fClosing	= true;
			fOutline->MessageReceived(new BMessage('mvCa'));
			return;
		}
		BTextControl::KeyDown(bytes,numBytes);
	}

private:
	MacroOutlineView	*fOutline;
	bool				fClosing;
};


// ------------------------------------------------------------- the view --

MacroOutlineView::MacroOutlineView(BRect frame, const char *name, uint32 resizingMode)
	:BOutlineListView(frame,name,B_SINGLE_SELECTION_LIST,resizingMode),
		fEditor(NULL), fOverlay(NULL), fOverlayRow(-1)
{
}


void MacroOutlineView::SetCommands(BList *commands)
{
	CloseOverlay(false);
	for (int32 i = 0; i < fCommands.CountItems(); i++)
		delete (BMessage*)fCommands.ItemAt(i);
	fCommands.MakeEmpty();
	if (commands != NULL)
		for (int32 i = 0; i < commands->CountItems(); i++) {
			BMessage	*cmd	= (BMessage*)commands->ItemAt(i);
			if (cmd != NULL)
				fCommands.AddItem(new BMessage(*cmd));
		}
	RebuildAllRows();
}


BMessage MacroOutlineView_ResolveContainer(BList *commands, int32 topLevelIndex, const MacroPath &path)
{
	BMessage	current	= *(BMessage*)commands->ItemAt(topLevelIndex);
	for (size_t i = 0; i < path.size(); i++) {
		BMessage	child;
		current.FindMessage(path[i].field.String(),path[i].index,&child);
		current	= child;
	}
	return current;
}


static void WriteContainer(BList *commands, int32 topLevelIndex, const MacroPath &path,
	const BMessage &newContent)
{
	BMessage	value	= newContent;
	for (int32 i = (int32)path.size()-1; i >= 0; i--) {
		MacroPath	ancestorPath(path.begin(),path.begin()+i);
		BMessage	ancestor	= MacroOutlineView_ResolveContainer(commands,topLevelIndex,ancestorPath);
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&ancestor,path[i].field.String(),&entries);
		if (path[i].index < (int32)entries.size())
			entries[path[i].index]	= value;
		else
			entries.push_back(value);
		ReplaceMessageEntries(&ancestor,path[i].field.String(),entries);
		value	= ancestor;
	}
	BMessage	*old	= (BMessage*)commands->ItemAt(topLevelIndex);
	commands->ReplaceItem(topLevelIndex,new BMessage(value));
	delete old;
}


void MacroOutlineView::RebuildAllRows(void)
{
	std::set<BString>	expanded;
	for (int32 i = 0; i < FullListCountItems(); i++) {
		MacroRowItem	*item	= (MacroRowItem*)FullListItemAt(i);
		if (((item->Kind() == kRowChip) || (item->Kind() == kRowBlock)) && item->IsExpanded())
			expanded.insert(PathKey(item->TopLevelIndex(),item->SelfPath()));
	}

	MakeEmpty();
	PCommandManager	*registry	= (fEditor != NULL) ? fEditor->CommandManagerForOutline() : NULL;

	for (int32 i = 0; i < fCommands.CountItems(); i++) {
		BMessage	*cmd	= (BMessage*)fCommands.ItemAt(i);
		if (cmd == NULL)
			continue;
		const char	*name	= NULL;
		cmd->FindString("Command::Name",&name);
		PCommand	*command	= (registry != NULL) ? registry->GetPCommand((char*)(name ? name : "")) : NULL;

		MacroPath	empty;
		MacroRowItem	*row	= new MacroRowItem(name ? name : "?",0,true,kRowCommand,
			i,empty,NULL,-1,B_ANY_TYPE,false,command,-1);
		AddItem(row);
		BuildChildren(cmd,i,empty,row,1,command,expanded);
	}
}


void MacroOutlineView::BuildChildren(BMessage *container, int32 topLevelIndex,
	const MacroPath &containerPath, MacroRowItem *superitem, uint32 level,
	PCommand *schemaCommand, const std::set<BString> &expandedKeys)
{
	// #135's bound fields ("PCommand::bindings") render as their own field
	// row showing "$variableName" - see MacroText.cpp's SerializeFieldLines
	// for the text-DSL's identical treatment.
	BMessage	bindings;
	bool		hasBindings	= container->FindMessage("PCommand::bindings",&bindings) == B_OK;
	std::set<BString>	boundNames;
	if (hasBindings) {
		char		*bindField;
		type_code	bindType;
		int32		bindCount;
		for (int32 i = 0; bindings.GetInfo(B_STRING_TYPE,i,&bindField,&bindType,&bindCount) == B_OK; i++) {
			const char	*variableName	= NULL;
			if (bindings.FindString(bindField,&variableName) == B_OK) {
				BString	label;
				label << bindField << ": $" << variableName;
				type_code	schemaType	= B_ANY_TYPE;
				if (schemaCommand != NULL)
					FindFieldType(schemaCommand,bindField,&schemaType);
				MacroRowItem	*row	= new MacroRowItem(label.String(),level,true,kRowField,
					topLevelIndex,containerPath,bindField,0,schemaType,true,NULL,
					BString(bindField).Length()+2);
				AddUnder(row,superitem);
				boundNames.insert(bindField);
			}
		}
	}

	char		*fieldName;
	type_code	type;
	int32		count;
	int32		i	= 0;
	const char	*commandName	= NULL;
	if (schemaCommand != NULL)
		container->FindString("Command::Name",&commandName);
	BString	undoFieldName;
	if (commandName != NULL)
		undoFieldName << commandName << "::Undo";

	while (container->GetInfo(B_ANY_TYPE,i,&fieldName,&type,&count) == B_OK) {
		BString	fn(fieldName);
		i++;
		if ((fn == "Command::Name") || (fn == "PCommand::bindings") ||
				(commandName != NULL && fn == undoFieldName) ||
				(boundNames.find(fn) != boundNames.end()))
			continue;

		if (fn == "PCommand::subPCommand") {
			for (int32 j = 0; j < count; j++) {
				BMessage	child;
				if (container->FindMessage(fieldName,j,&child) != B_OK)
					continue;
				const char	*childName	= NULL;
				child.FindString("Command::Name",&childName);
				PCommandManager	*registry	= (fEditor != NULL) ? fEditor->CommandManagerForOutline() : NULL;
				PCommand	*childCommand	= (registry != NULL)
					? registry->GetPCommand((char*)(childName ? childName : "")) : NULL;
				MacroPath	childPath(containerPath);
				MacroPathStep	step; step.field = "PCommand::subPCommand"; step.index = j;
				childPath.push_back(step);
				MacroRowItem	*row	= new MacroRowItem(childName ? childName : "?",level,true,
					kRowCommand,topLevelIndex,containerPath,"PCommand::subPCommand",j,
					B_ANY_TYPE,false,childCommand,-1);
				AddUnder(row,superitem);
				BuildChildren(&child,topLevelIndex,childPath,row,level+1,childCommand,expandedKeys);
			}
			continue;
		}

		if (type == B_MESSAGE_TYPE) {
			bool	isChip	= (fn == "included_node") || (fn == "included_connection");
			for (int32 j = 0; j < count; j++) {
				BMessage	child;
				if (container->FindMessage(fieldName,j,&child) != B_OK)
					continue;
				MacroPath	childPath(containerPath);
				MacroPathStep	step; step.field = fn; step.index = j;
				childPath.push_back(step);
				BString	label;
				if (isChip)
					ChipLabel(child,&label);
				else
					label	= fieldName;
				bool	expandThis	= expandedKeys.find(PathKey(topLevelIndex,childPath)) != expandedKeys.end();
				MacroRowItem	*row	= new MacroRowItem(label.String(),level,expandThis,
					isChip ? kRowChip : kRowBlock,topLevelIndex,containerPath,fieldName,j,
					B_ANY_TYPE,false,NULL,-1);
				AddUnder(row,superitem);
				// no schema inside a nested block/chip (see MacroText.h) -
				// its own children get no "+ Feld hinzufügen" of their own
				BuildChildren(&child,topLevelIndex,childPath,row,level+1,NULL,expandedKeys);
			}
			continue;
		}

		type_code	schemaType	= type;
		if (schemaCommand != NULL)
			FindFieldType(schemaCommand,fieldName,&schemaType);
		for (int32 j = 0; j < count; j++) {
			BString	label(fieldName);
			label	<< ": ";
			int32	valueStart	= label.Length();
			FormatFieldValue(container,fieldName,type,j,&label);
			MacroRowItem	*row	= new MacroRowItem(label.String(),level,true,kRowField,
				topLevelIndex,containerPath,fieldName,j,schemaType,(schemaCommand != NULL),
				NULL,valueStart);
			AddUnder(row,superitem);
		}
	}

	if (schemaCommand != NULL) {
		int32	propCount	= 0;
		schemaCommand->PropertyInfo(&propCount);
		if (propCount > 0) {
			MacroRowItem	*addRow	= new MacroRowItem(B_TRANSLATE("+ Feld hinzufügen"),level,true,
				kRowAddField,topLevelIndex,containerPath,"",-1,B_ANY_TYPE,false,schemaCommand,-1);
			AddUnder(addRow,superitem);
		}
	}
}


// -------------------------------------------------------------- editing --

void MacroOutlineView::CloseOverlay(bool commit)
{
	if (fOverlay == NULL)
		return;
	int32	rowIndex	= fOverlayRow;
	BString	text(fOverlay->Text());
	RemoveChild(fOverlay);
	delete fOverlay;
	fOverlay	= NULL;
	fOverlayRow	= -1;

	if (!commit || (rowIndex < 0) || (rowIndex >= FullListCountItems()))
		return;
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if ((item == NULL) || (item->Kind() != kRowField))
		return;

	BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
	owner.RemoveData(item->FieldName().String(),item->FieldIndex());
	BString	error;
	if (ParseFieldValue(&owner,item->FieldName().String(),item->FieldType(),text,
			item->AllowBinding(),&error) != B_OK) {
		// invalid input - row keeps its old value, nothing is written back
		if (fEditor != NULL)
			fEditor->ShowOutlineError(error.String());
		return;
	}
	WriteContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath(),owner);
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::BeginOverlayEdit(int32 rowIndex)
{
	CloseOverlay(true);
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if ((item == NULL) || (item->Kind() != kRowField) || (item->FieldType() == B_BOOL_TYPE))
		return;

	BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
	// the stored type (what GetInfo reports) - not necessarily the schema
	// type kept on the row for validation (e.g. "node" is schema
	// B_POINTER_TYPE but stored as a plain int32 - see MacroText.h's
	// FormatFieldValue doc comment)
	type_code	storedType	= B_ANY_TYPE;
	int32		storedCount	= 0;
	owner.GetInfo(item->FieldName().String(),&storedType,&storedCount);
	BString	text;
	FormatFieldValue(&owner,item->FieldName().String(),storedType,item->FieldIndex(),&text);

	BRect	frame	= ItemFrame(rowIndex);
	fOverlay	= new MacroOverlayControl(frame,text.String(),this);
	AddChild(fOverlay);
	fOverlayRow	= rowIndex;
	fOverlay->MakeFocus(true);
	fOverlay->TextView()->SelectAll();
}


void MacroOutlineView::ToggleBoolField(int32 rowIndex)
{
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if ((item == NULL) || (item->Kind() != kRowField) || (item->FieldType() != B_BOOL_TYPE))
		return;
	BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
	bool	value	= false;
	owner.FindBool(item->FieldName().String(),item->FieldIndex(),&value);
	owner.ReplaceBool(item->FieldName().String(),item->FieldIndex(),!value);
	WriteContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath(),owner);
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::ShowAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
	const MacroPath &path, PCommand *command)
{
	if (command == NULL)
		return;
	int32				propCount	= 0;
	const property_info	*props	= command->PropertyInfo(&propCount);
	BPopUpMenu	*menu	= new BPopUpMenu("addField",false,false);
	for (int32 p = 0; p < propCount; p++) {
		for (int32 c = 0; c < 3; c++) {
			for (int32 f = 0; f < 5; f++) {
				const char	*fieldName	= props[p].ctypes[c].pairs[f].name;
				if (fieldName == NULL)
					continue;
				type_code	type	= props[p].ctypes[c].pairs[f].type;
				// no safe generic zero value for an arbitrary nested block
				// (no schema for its own content - see MacroText.h) except
				// "included_node", which gets a real node prototype
				if ((type == B_MESSAGE_TYPE) && (strcmp(fieldName,"included_node") != 0))
					continue;
				BString	label(fieldName);
				label	<< " (" << TypeDisplayName(type) << ")";
				BMessage	*msg	= new BMessage('mvAF');
				msg->AddString("field",fieldName);
				msg->AddInt32("type",(int32)type);
				msg->AddInt32("topLevel",topLevelIndex);
				for (size_t s = 0; s < path.size(); s++) {
					msg->AddString("pathField",path[s].field);
					msg->AddInt32("pathIndex",path[s].index);
				}
				menu->AddItem(new BMenuItem(label.String(),msg));
			}
		}
	}
	menu->SetTargetForItems(this);
	menu->Go(screenWhere,true,true,true);
}


void MacroOutlineView::DeleteRow(int32 rowIndex)
{
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if (item == NULL)
		return;
	if ((item->Kind() == kRowCommand) && item->ContainerPath().empty() && (item->FieldName().Length() == 0)) {
		BMessage	*old	= (BMessage*)fCommands.RemoveItem(item->TopLevelIndex());
		delete old;
	} else if (item->Kind() != kRowAddField) {
		BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&owner,item->FieldName().String(),&entries);
		if ((item->Kind() == kRowField)) {
			// scalar - RemoveData() shifts same-named entries down exactly
			// like ExtractMessageEntries()+erase()+re-add would, cheaper
			owner.RemoveData(item->FieldName().String(),item->FieldIndex());
		} else if (item->FieldIndex() < (int32)entries.size()) {
			entries.erase(entries.begin()+item->FieldIndex());
			ReplaceMessageEntries(&owner,item->FieldName().String(),entries);
		}
		WriteContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath(),owner);
	}
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::MoveRow(int32 rowIndex, int32 direction)
{
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if (item == NULL)
		return;
	if ((item->Kind() == kRowCommand) && item->ContainerPath().empty() && (item->FieldName().Length() == 0)) {
		int32	i	= item->TopLevelIndex();
		int32	j	= i+direction;
		if ((j < 0) || (j >= fCommands.CountItems()))
			return;
		void	*a	= fCommands.ItemAt(i);
		void	*b	= fCommands.ItemAt(j);
		fCommands.ReplaceItem(i,b);
		fCommands.ReplaceItem(j,a);
		RebuildAllRows();
		if (fEditor != NULL)
			fEditor->CommitOutlineChange();
		return;
	}
	if (item->Kind() == kRowAddField)
		return;

	BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
	int32	i	= item->FieldIndex();
	int32	j	= i+direction;
	if (item->Kind() == kRowField) {
		int32		count	= 0;
		type_code	t;
		owner.GetInfo(item->FieldName().String(),&t,&count);
		if ((j < 0) || (j >= count))
			return;
		std::vector<BString>	texts;
		for (int32 k = 0; k < count; k++) {
			BString	v;
			FormatFieldValue(&owner,item->FieldName().String(),t,k,&v);
			texts.push_back(v);
		}
		std::swap(texts[i],texts[j]);
		owner.RemoveName(item->FieldName().String());
		BString	error;
		for (size_t k = 0; k < texts.size(); k++)
			ParseFieldValue(&owner,item->FieldName().String(),item->FieldType(),texts[k],
				item->AllowBinding(),&error);
	} else {
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&owner,item->FieldName().String(),&entries);
		if ((j < 0) || (j >= (int32)entries.size()))
			return;
		std::swap(entries[i],entries[j]);
		ReplaceMessageEntries(&owner,item->FieldName().String(),entries);
	}
	WriteContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath(),owner);
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::DropCommandSnippet(int32 targetRow, BPoint where,
	const char *snippet, int32 length)
{
	PCommandManager	*registry	= (fEditor != NULL) ? fEditor->CommandManagerForOutline() : NULL;
	if (registry == NULL)
		return;
	BString	text(snippet,length);
	BList		parsed;
	BString		error;
	if ((ParseCommands(text,&parsed,registry,&error) != B_OK) || (parsed.CountItems() != 1)) {
		for (int32 i = 0; i < parsed.CountItems(); i++)
			delete (BMessage*)parsed.ItemAt(i);
		beep();
		return;
	}
	BMessage	*newCommand	= (BMessage*)parsed.ItemAt(0);
	const char	*name	= NULL;
	newCommand->FindString("Command::Name",&name);
	if ((name != NULL) && (strcmp(name,"Insert") == 0)) {
		int32	newId	= HighestReferencedId(&fCommands)+1;
		if (newId < 1)
			newId	= 1;
		AssignInsertId(newCommand,newId);
	}

	MacroRowItem	*target	= ((targetRow >= 0) && (targetRow < FullListCountItems()))
		? (MacroRowItem*)ItemAt(targetRow) : NULL;
	if (target == NULL) {
		fCommands.AddItem(newCommand);
		RebuildAllRows();
		if (fEditor != NULL)
			fEditor->CommitOutlineChange();
		return;
	}

	BMessage	targetData	= MacroOutlineView_ResolveContainer(&fCommands,target->TopLevelIndex(),
		MacroPath(target->SelfPath()));
	const char	*targetName	= NULL;
	bool	isCommandRow	= (target->Kind() == kRowCommand);
	if (isCommandRow)
		targetData.FindString("Command::Name",&targetName);

	BRect	targetFrame	= ItemFrame(targetRow);
	bool	lowerHalf	= where.y > targetFrame.top+targetFrame.Height()/2;

	if (isCommandRow && IsContainerCommandName(targetName)) {
		// dropped onto a container command - becomes its first subcommand
		MacroPath	containerPath(target->SelfPath());
		BMessage	container	= MacroOutlineView_ResolveContainer(&fCommands,target->TopLevelIndex(),containerPath);
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&container,"PCommand::subPCommand",&entries);
		entries.insert(entries.begin(),*newCommand);
		ReplaceMessageEntries(&container,"PCommand::subPCommand",entries);
		WriteContainer(&fCommands,target->TopLevelIndex(),containerPath,container);
		delete newCommand;
	} else if (target->ContainerPath().empty() && (target->FieldName().Length() == 0) && isCommandRow) {
		// a sibling of a top-level command
		int32	at	= target->TopLevelIndex() + (lowerHalf ? 1 : 0);
		fCommands.AddItem(newCommand,at);
	} else if (isCommandRow && (target->FieldName() == "PCommand::subPCommand")) {
		// a sibling of a subcommand
		BMessage	container	= MacroOutlineView_ResolveContainer(&fCommands,target->TopLevelIndex(),target->ContainerPath());
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&container,"PCommand::subPCommand",&entries);
		int32	at	= target->FieldIndex() + (lowerHalf ? 1 : 0);
		if (at > (int32)entries.size())
			at	= (int32)entries.size();
		entries.insert(entries.begin()+at,*newCommand);
		ReplaceMessageEntries(&container,"PCommand::subPCommand",entries);
		WriteContainer(&fCommands,target->TopLevelIndex(),target->ContainerPath(),container);
		delete newCommand;
	} else {
		// dropped on a field/chip/block row - falls back to the top of
		// that row's own top-level command
		fCommands.AddItem(newCommand,target->TopLevelIndex()+1);
	}

	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


// ------------------------------------------------------------- BView etc --

void MacroOutlineView::MouseDown(BPoint where)
{
	int32	clicks	= 1;
	uint32	buttons	= 0;
	if ((Window() != NULL) && (Window()->CurrentMessage() != NULL)) {
		Window()->CurrentMessage()->FindInt32("clicks",&clicks);
		Window()->CurrentMessage()->FindInt32("buttons",(int32*)&buttons);
	}
	int32	index	= IndexOf(where);
	MacroRowItem	*item	= (index >= 0) ? (MacroRowItem*)ItemAt(index) : NULL;
	CloseOverlay(true);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) && (item != NULL) && (item->Kind() != kRowAddField)) {
		Select(index);
		bool	isTopLevel	= (item->Kind() == kRowCommand) && item->ContainerPath().empty()
			&& (item->FieldName().Length() == 0);
		int32	count, i;
		if (isTopLevel) {
			count	= fCommands.CountItems();
			i		= item->TopLevelIndex();
		} else {
			BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,item->TopLevelIndex(),item->ContainerPath());
			type_code	t;
			owner.GetInfo(item->FieldName().String(),&t,&count);
			i	= item->FieldIndex();
		}
		BMessage	*upMsg		= new BMessage('mvUp'); upMsg->AddInt32("row",index);
		BMessage	*downMsg	= new BMessage('mvDn'); downMsg->AddInt32("row",index);
		BMessage	*delMsg		= new BMessage('mvDl'); delMsg->AddInt32("row",index);
		BPopUpMenu	*menu	= new BPopUpMenu("rowMenu",false,false);
		BMenuItem	*up		= new BMenuItem(B_TRANSLATE("Move Up"),upMsg);
		BMenuItem	*down	= new BMenuItem(B_TRANSLATE("Move Down"),downMsg);
		up->SetEnabled(i > 0);
		down->SetEnabled(i < count-1);
		menu->AddItem(up);
		menu->AddItem(down);
		menu->AddSeparatorItem();
		menu->AddItem(new BMenuItem(B_TRANSLATE("Delete"),delMsg));
		menu->SetTargetForItems(this);
		menu->Go(ConvertToScreen(where),true,true,true);
		return;
	}

	if ((item != NULL) && (item->Kind() == kRowAddField)) {
		Select(index);
		ShowAddFieldMenu(ConvertToScreen(where),item->TopLevelIndex(),item->ContainerPath(),item->Command());
		return;
	}

	BOutlineListView::MouseDown(where);

	if ((item != NULL) && (item->Kind() == kRowField)) {
		if (item->FieldType() == B_BOOL_TYPE)
			ToggleBoolField(index);
		else if (clicks == 2)
			BeginOverlayEdit(index);
	}
}


void MacroOutlineView::KeyDown(const char *bytes, int32 numBytes)
{
	if ((numBytes == 1) && ((bytes[0] == B_DELETE) || (bytes[0] == B_BACKSPACE))) {
		int32	index	= CurrentSelection();
		if (index >= 0)
			DeleteRow(index);
		return;
	}
	BOutlineListView::KeyDown(bytes,numBytes);
}


void MacroOutlineView::SelectionChanged(void)
{
	BOutlineListView::SelectionChanged();
}


bool MacroOutlineView::InitiateDrag(BPoint where, int32 index, bool wasSelected)
{
	// no native drag-to-reorder in v1 (see MacroOutlineView.h) - Move Up/
	// Move Down cover reordering instead
	return false;
}


void MacroOutlineView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kOverlayCommit:
			CloseOverlay(true);
			return;
		case 'mvCa':
			CloseOverlay(false);
			delete message;
			return;
		case 'mvUp': {
			int32	row	= -1;
			message->FindInt32("row",&row);
			if (row >= 0)
				MoveRow(row,-1);
			return;
		}
		case 'mvDn': {
			int32	row	= -1;
			message->FindInt32("row",&row);
			if (row >= 0)
				MoveRow(row,1);
			return;
		}
		case 'mvDl': {
			int32	row	= -1;
			message->FindInt32("row",&row);
			if (row >= 0)
				DeleteRow(row);
			return;
		}
		case 'mvAF': {
			const char	*field	= NULL;
			int32		type	= 0;
			int32		topLevel	= 0;
			message->FindString("field",&field);
			message->FindInt32("type",&type);
			message->FindInt32("topLevel",&topLevel);
			MacroPath	path;
			BString	pathField;
			int32	pathIndex;
			for (int32 i = 0; message->FindString("pathField",i,&pathField) == B_OK; i++) {
				message->FindInt32("pathIndex",i,&pathIndex);
				MacroPathStep	step; step.field = pathField; step.index = pathIndex;
				path.push_back(step);
			}
			if (field != NULL) {
				BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,topLevel,path);
				if (strcmp(field,"included_node") == 0) {
					BMessage	*proto	= BuildInsertPrototype();
					BMessage	node;
					proto->FindMessage("included_node",&node);
					int32	newId	= HighestReferencedId(&fCommands)+1;
					if (newId < 1)
						newId	= 1;
					node.RemoveName("this");
					node.AddInt32("this",newId);
					owner.AddMessage(field,&node);
					delete proto;
				} else
					AddZeroValue(&owner,field,(type_code)type);
				WriteContainer(&fCommands,topLevel,path,owner);
				RebuildAllRows();
				if (fEditor != NULL)
					fEditor->CommitOutlineChange();
			}
			return;
		}
		default:
			break;
	}

	if (message->WasDropped() && message->HasBool(kCommandSnippetDragMarker)) {
		const void	*data	= NULL;
		ssize_t		length	= 0;
		if ((message->FindData("text/plain",B_MIME_TYPE,&data,&length) == B_OK) && (length > 0)) {
			BPoint	dropPoint	= message->DropPoint();
			ConvertFromScreen(&dropPoint);
			int32	target	= IndexOf(dropPoint);
			DropCommandSnippet(target,dropPoint,(const char*)data,(int32)length);
		}
		return;
	}

	BOutlineListView::MessageReceived(message);
}
