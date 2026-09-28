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

#include "InputRequest.h"
#include "MacroEditor.h"
#include "MacroText.h"
#include "PCommand.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MacroEditor"

const char* const kCommandSnippetDragMarker	= "pc:command_snippet";
const char* const kCommandMoveDragMarker		= "pc:command_move";

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
	int32				ValueStart(void) const {return fValueStart;}

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


/** A command row's own "chip" shape - a ribbon with a pointed notch cut
 * into both ends ("<===>", the user's own suggestion) - visually distinct
 * from a data chip's rounded pill, so every row with its own expand
 * triangle (a command or a data chip) has an unmistakable shape that
 * triangle belongs to, instead of the triangle floating next to plain
 * text. */
static void DrawChevronShape(BView *owner, BRect frame, rgb_color fill)
{
	float	notch	= frame.Height()/2.2f;
	if (notch > 11)
		notch	= 11;
	BPoint	points[6];
	points[0]	= BPoint(frame.left,(frame.top+frame.bottom)/2);
	points[1]	= BPoint(frame.left+notch,frame.top);
	points[2]	= BPoint(frame.right-notch,frame.top);
	points[3]	= BPoint(frame.right,(frame.top+frame.bottom)/2);
	points[4]	= BPoint(frame.right-notch,frame.bottom);
	points[5]	= BPoint(frame.left+notch,frame.bottom);
	owner->SetHighColor(fill);
	owner->FillPolygon(points,6);
}


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

	if (fKind == kRowCommand) {
		rgb_color	fill	= {237,214,168,255};
		float	width	= owner->StringWidth(Text());
		BRect	shape(frame.left+2,frame.top+1,frame.left+26+width,frame.bottom-1);
		DrawChevronShape(owner,shape,fill);
		BFont	font(be_plain_font);
		font.SetFace(B_BOLD_FACE);
		owner->SetFont(&font);
		owner->SetHighColor(textColor);
		owner->DrawString(Text(),BPoint(shape.left+13,baseline));
		owner->SetFont(be_plain_font);
		return;
	}

	BFont	font(be_plain_font);
	if (fKind == kRowBlock)
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


/** How a field's value looks for display in the tree and for the inline
 * overlay's starting text - FormatFieldValue() (the DSL's own quoted/
 * suffixed syntax) for everything except strings, which show/edit as their
 * own plain text with no quotes to type or strip (user report: quoting a
 * string by hand felt like DSL leftovers, not a real text field). */
static void FormatValueForEditing(BMessage *msg, const char *field, type_code storedType,
	int32 index, BString *out)
{
	if (storedType == B_STRING_TYPE) {
		const char	*value	= NULL;
		msg->FindString(field,index,&value);
		*out << (value ? value : "");
		return;
	}
	FormatFieldValue(msg,field,storedType,index,out);
}


/** Pixel x where `item`'s own value text starts within `frame` - matches
 * MacroRowItem::DrawItem()'s split exactly (same font, same "label" prefix
 * length), so the overlay lands exactly on top of the value it replaces,
 * and a click past that x is unambiguously "on the value" (see MouseDown()). */
static float ValuePixelX(MacroRowItem *item, BRect frame)
{
	if (item->ValueStart() <= 0)
		return frame.left+2;
	BString	label(item->Text());
	label.Truncate(item->ValueStart());
	BFont	font(be_plain_font);
	return frame.left+2+font.StringWidth(label.String());
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
	:BOutlineListView(frame,name,B_MULTIPLE_SELECTION_LIST,resizingMode),
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


/** BOutlineListView::AddUnder() inserts right after `superitem` itself,
 * not after its existing last child (confirmed against Haiku's own
 * OutlineListView.cpp: `fFullList.AddItem(item, FullListIndexOf(superItem)
 * + 1)`) - calling it once per child in build order therefore builds every
 * level's children in REVERSE order (each new one pushed in right after
 * the parent, ahead of its already-added older siblings). This appends
 * `item` after superitem's entire current subtree instead, so children
 * come out in the order BuildChildren() actually adds them. `item`'s own
 * level must already be set correctly (its constructor already does
 * this - one more than `superitem`'s). */
void MacroOutlineView::AppendUnder(MacroRowItem *item, MacroRowItem *superitem)
{
	if (superitem == NULL) {
		AddItem(item);
		return;
	}
	int32	insertAt	= FullListIndexOf(superitem)+1+CountItemsUnder(superitem,false);
	BOutlineListView::AddItem(item,insertAt);
}


int32 MacroOutlineView::RowIndexForCommand(int32 topLevel, const MacroPath &selfPath)
{
	for (int32 i = 0; i < FullListCountItems(); i++) {
		MacroRowItem	*item	= (MacroRowItem*)FullListItemAt(i);
		if ((item->Kind() != kRowCommand) || (item->TopLevelIndex() != topLevel))
			continue;
		MacroPath	itemSelf	= item->SelfPath();
		if (itemSelf.size() != selfPath.size())
			continue;
		bool	same	= true;
		for (size_t s = 0; same && (s < selfPath.size()); s++)
			if ((itemSelf[s].field != selfPath[s].field) || (itemSelf[s].index != selfPath[s].index))
				same	= false;
		if (same)
			return i;
	}
	return -1;
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
				AppendUnder(row,superitem);
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
		if ((fn == "Command::Name") || (fn == "PCommand::bindings") || (fn == "this") ||
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
				AppendUnder(row,superitem);
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
				AppendUnder(row,superitem);
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
			FormatValueForEditing(container,fieldName,type,j,&label);
			MacroRowItem	*row	= new MacroRowItem(label.String(),level,true,kRowField,
				topLevelIndex,containerPath,fieldName,j,schemaType,(schemaCommand != NULL),
				NULL,valueStart);
			AppendUnder(row,superitem);
		}
	}

	if (schemaCommand != NULL) {
		int32	propCount	= 0;
		schemaCommand->PropertyInfo(&propCount);
		if (propCount > 0) {
			MacroRowItem	*addRow	= new MacroRowItem(B_TRANSLATE("+ Feld hinzufügen"),level,true,
				kRowAddField,topLevelIndex,containerPath,"",-1,B_ANY_TYPE,false,schemaCommand,-1);
			AppendUnder(addRow,superitem);
		}
	} else {
		// a generic block/chip has no schema to offer field names from,
		// but a user-named custom field is still meaningful there (user
		// report, forward-looking - see ShowFreeformAddFieldMenu()) -
		// command==NULL on this row is exactly what tells
		// ShowAddFieldMenu() which of the two flows to open
		MacroRowItem	*addRow	= new MacroRowItem(B_TRANSLATE("+ Feld hinzufügen"),level,true,
			kRowAddField,topLevelIndex,containerPath,"",-1,B_ANY_TYPE,false,NULL,-1);
		AppendUnder(addRow,superitem);
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
	// a string field takes the overlay's text verbatim, no quotes to type
	// or strip - matches how it was shown (see FormatValueForEditing()).
	// Still checked for "$name" first, same as every other field, so a
	// string field stays bindable.
	bool	isBoundLiteral	= item->AllowBinding() && text.StartsWith("$");
	if ((item->FieldType() == B_STRING_TYPE) && !isBoundLiteral) {
		owner.AddString(item->FieldName().String(),text);
	} else {
		BString	error;
		if (ParseFieldValue(&owner,item->FieldName().String(),item->FieldType(),text,
				item->AllowBinding(),&error) != B_OK) {
			// invalid input - row keeps its old value, nothing is written back
			if (fEditor != NULL)
				fEditor->ShowOutlineError(error.String());
			return;
		}
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
	FormatValueForEditing(&owner,item->FieldName().String(),storedType,item->FieldIndex(),&text);

	// starts exactly where the value is already drawn (user report: it
	// used to span the whole row, starting at the label instead of the
	// value it replaces)
	BRect	frame	= ItemFrame(rowIndex);
	frame.left	= ValuePixelX(item,frame);
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


/** Every field/pathField+pathIndex pair topLevelIndex/path expands to on a
 * drag/add-field message - the write side of ReadFieldTargetMessage(). One
 * spelling of this, shared by every place that builds such a message
 * ('mvAF'/'mvFT' menu items here, InitiateDrag()'s own move message). */
static void AddFieldTargetFields(BMessage *msg, int32 topLevelIndex, const MacroPath &path)
{
	msg->AddInt32("topLevel",topLevelIndex);
	for (size_t s = 0; s < path.size(); s++) {
		msg->AddString("pathField",path[s].field);
		msg->AddInt32("pathIndex",path[s].index);
	}
}


/** The read side of AddFieldTargetFields() - topLevel/path back out of a
 * message 'mvAF'/'mvFT' built. */
static int32 ReadFieldTargetMessage(BMessage *message, MacroPath *outPath)
{
	int32	topLevel	= 0;
	message->FindInt32("topLevel",&topLevel);
	BString	pathField;
	int32	pathIndex;
	for (int32 i = 0; message->FindString("pathField",i,&pathField) == B_OK; i++) {
		message->FindInt32("pathIndex",i,&pathIndex);
		MacroPathStep	step; step.field = pathField; step.index = pathIndex;
		outPath->push_back(step);
	}
	return topLevel;
}


void MacroOutlineView::AddNamedField(int32 topLevel, const MacroPath &path,
	const char *field, type_code type)
{
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
		AddZeroValue(&owner,field,type);
	WriteContainer(&fCommands,topLevel,path,owner);
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::ShowAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
	const MacroPath &path, PCommand *command)
{
	if (command == NULL) {
		ShowFreeformAddFieldMenu(screenWhere,topLevelIndex,path);
		return;
	}
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
				// (no schema for its own content - see MacroText.h). Every
				// command with a "node" field declares "included_node" too
				// (Indexer bookkeeping - it carries the referenced node's
				// data so its @id resolves at replay time, never read by
				// the command's own Do() - see Insert.h's kInsertProperties
				// comment), but "Insert" is the one command actually meant
				// to introduce a new node by hand (user report/single
				// source of truth) - offering it here on e.g. AddAttribute
				// or Select would let a macro claim to "insert" a node
				// nobody would ever expect it to.
				if (type == B_MESSAGE_TYPE) {
					bool	isInsertNode	= (strcmp(fieldName,"included_node") == 0)
						&& (strcmp(command->Name(),"Insert") == 0);
					if (!isInsertNode)
						continue;
				}
				BString	label(fieldName);
				label	<< " (" << TypeDisplayName(type) << ")";
				BMessage	*msg	= new BMessage('mvAF');
				msg->AddString("field",fieldName);
				msg->AddInt32("type",(int32)type);
				AddFieldTargetFields(msg,topLevelIndex,path);
				menu->AddItem(new BMenuItem(label.String(),msg));
			}
		}
	}
	menu->SetTargetForItems(this);
	menu->Go(screenWhere,true,true,true);
}


void MacroOutlineView::ShowFreeformAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
	const MacroPath &path)
{
	// a generic block (~valueContainer, an included_node's own Node::Data,
	// ...) has no schema (see MacroText.h) to offer field names from - the
	// type is still a fixed, safe choice (only the types FormatFieldValue()/
	// ParseFieldValue() actually round-trip), but the name is typed in
	// (see MessageReceived()'s 'mvFT' case) - forward-looking (user report):
	// today's "add a custom attribute" tool is AddAttribute, but a node's
	// own data being free-form BMessage content already meant this was
	// always technically possible, just not from inside the tree itself.
	static const struct { const char *name; type_code type; } kTypes[] = {
		{ "bool", B_BOOL_TYPE }, { "int8", B_INT8_TYPE }, { "int16", B_INT16_TYPE },
		{ "int32", B_INT32_TYPE }, { "int64", B_INT64_TYPE }, { "float", B_FLOAT_TYPE },
		{ "double", B_DOUBLE_TYPE }, { "string", B_STRING_TYPE },
		{ "point", B_POINT_TYPE }, { "rect", B_RECT_TYPE },
	};
	BPopUpMenu	*menu	= new BPopUpMenu("addFreeformField",false,false);
	for (size_t i = 0; i < sizeof(kTypes)/sizeof(kTypes[0]); i++) {
		BMessage	*msg	= new BMessage('mvFT');
		msg->AddInt32("type",(int32)kTypes[i].type);
		AddFieldTargetFields(msg,topLevelIndex,path);
		menu->AddItem(new BMenuItem(kTypes[i].name,msg));
	}
	menu->SetTargetForItems(this);
	menu->Go(screenWhere,true,true,true);
}


/** The actual removal, shared by DeleteRow() (one row, via Delete key or
 * the context menu) and DeleteSelectedRows() (every selected row, via a
 * multi-selection Delete). Does not rebuild/commit itself - callers batch
 * that once after however many identities they remove, since a multi-
 * delete rebuilding after every single one would repeatedly resolve
 * against a tree its own later removals haven't happened in yet. */
void MacroOutlineView::DeleteIdentity(int32 topLevel, const MacroPath &containerPath,
	const BString &fieldName, int32 fieldIndex, bool isTopLevelCommand)
{
	if (isTopLevelCommand) {
		BMessage	*old	= (BMessage*)fCommands.RemoveItem(topLevel);
		delete old;
		return;
	}
	BMessage	owner	= MacroOutlineView_ResolveContainer(&fCommands,topLevel,containerPath);
	std::vector<BMessage>	entries;
	ExtractMessageEntries(&owner,fieldName.String(),&entries);
	if (entries.empty()) {
		// a scalar field - RemoveData() shifts same-named entries down
		// exactly like ExtractMessageEntries()+erase()+re-add would,
		// cheaper (ExtractMessageEntries() only ever finds B_MESSAGE_TYPE
		// entries, so a scalar field's own "entries" is always empty here)
		owner.RemoveData(fieldName.String(),fieldIndex);
	} else if (fieldIndex < (int32)entries.size()) {
		entries.erase(entries.begin()+fieldIndex);
		ReplaceMessageEntries(&owner,fieldName.String(),entries);
	}
	WriteContainer(&fCommands,topLevel,containerPath,owner);
}


void MacroOutlineView::DeleteRow(int32 rowIndex)
{
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(rowIndex);
	if ((item == NULL) || (item->Kind() == kRowAddField))
		return;
	bool	isTopLevelCommand	= (item->Kind() == kRowCommand) && item->ContainerPath().empty()
		&& (item->FieldName().Length() == 0);
	DeleteIdentity(item->TopLevelIndex(),item->ContainerPath(),item->FieldName(),item->FieldIndex(),isTopLevelCommand);
	RebuildAllRows();
	if (fEditor != NULL)
		fEditor->CommitOutlineChange();
}


void MacroOutlineView::DeleteSelectedRows(void)
{
	struct RowIdentity {
		int32		topLevel;
		MacroPath	containerPath;
		BString		fieldName;
		int32		fieldIndex;
		bool		isTopLevelCommand;
		MacroPath	selfPath;	// containerPath + {fieldName,fieldIndex}, empty for a top-level command
	};

	std::vector<RowIdentity>	selected;
	for (int32 s = 0; ; s++) {
		int32	fullIndex	= CurrentSelection(s);
		if (fullIndex < 0)
			break;
		MacroRowItem	*item	= (MacroRowItem*)FullListItemAt(fullIndex);
		if ((item == NULL) || (item->Kind() == kRowAddField))
			continue;
		RowIdentity	id;
		id.topLevel			= item->TopLevelIndex();
		id.containerPath	= item->ContainerPath();
		id.fieldName		= item->FieldName();
		id.fieldIndex		= item->FieldIndex();
		id.isTopLevelCommand	= (item->Kind() == kRowCommand) && id.containerPath.empty()
			&& (id.fieldName.Length() == 0);
		id.selfPath	= item->SelfPath();
		selected.push_back(id);
	}
	if (selected.empty())
		return;

	// drop anything that's inside another selected row's own subtree -
	// removing the ancestor already removes it; processing both would
	// resolve the descendant against a container that's already gone
	std::vector<RowIdentity>	toRemove;
	for (size_t i = 0; i < selected.size(); i++) {
		bool	isNested	= false;
		for (size_t j = 0; (j < selected.size()) && !isNested; j++) {
			if ((i == j) || (selected[i].topLevel != selected[j].topLevel))
				continue;
			const MacroPath	&ancestor	= selected[j].selfPath;
			const MacroPath	&mine		= selected[i].selfPath;
			if (ancestor.size() < mine.size()) {
				bool	within	= true;
				for (size_t k = 0; within && (k < ancestor.size()); k++)
					if ((mine[k].field != ancestor[k].field) || (mine[k].index != ancestor[k].index))
						within	= false;
				if (within)
					isNested	= true;
			}
		}
		if (!isNested)
			toRemove.push_back(selected[i]);
	}

	// remove highest (topLevel, then fieldIndex) first - the only two ways
	// removing one row shifts another's index (see MoveCommandRow()'s own
	// comment on the same two cases) - so nothing removed here ever needs
	// its own already-captured identity corrected afterward
	while (!toRemove.empty()) {
		size_t	best	= 0;
		for (size_t i = 1; i < toRemove.size(); i++) {
			if ((toRemove[i].topLevel > toRemove[best].topLevel) ||
					((toRemove[i].topLevel == toRemove[best].topLevel)
						&& (toRemove[i].fieldIndex > toRemove[best].fieldIndex)))
				best	= i;
		}
		DeleteIdentity(toRemove[best].topLevel,toRemove[best].containerPath,
			toRemove[best].fieldName,toRemove[best].fieldIndex,toRemove[best].isTopLevelCommand);
		toRemove.erase(toRemove.begin()+best);
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


/** Resolves the row at `targetRow` into where a drop there should land -
 * shared by DropCommandSnippet() and MoveCommandRow(). Reads the row as it
 * currently stands (its caller's job to have already adjusted `targetRow`/
 * the row's own state for anything removed earlier in the same operation -
 * see MoveCommandRow()). */
static MacroTargetPosition ResolveTargetPosition(MacroOutlineView *view, int32 targetRow, BPoint where)
{
	MacroTargetPosition	pos;
	MacroRowItem	*target	= ((targetRow >= 0) && (targetRow < view->FullListCountItems()))
		? (MacroRowItem*)view->ItemAt(targetRow) : NULL;
	pos.valid	= (target != NULL);
	if (!pos.valid)
		return pos;
	pos.isCommandRow		= (target->Kind() == kRowCommand);
	pos.isTopLevelCommand	= pos.isCommandRow && target->ContainerPath().empty()
		&& (target->FieldName().Length() == 0);
	pos.topLevel		= target->TopLevelIndex();
	pos.containerPath	= target->ContainerPath();
	pos.fieldName		= target->FieldName();
	pos.fieldIndex		= target->FieldIndex();
	BRect	frame	= view->ItemFrame(targetRow);
	pos.lowerHalf	= where.y > frame.top+frame.Height()/2;
	return pos;
}


void MacroOutlineView::InsertCommandAt(BMessage *command, const MacroTargetPosition &pos)
{
	std::vector<BMessage>	one;
	one.push_back(*command);
	delete command;
	InsertCommandsAt(one,pos);
}


/** The actual insertion InsertCommandAt() (one freshly parsed command) and
 * MoveCommandRows() (several existing commands, moved together) share -
 * `commands` is inserted as one contiguous, order-preserved block at `pos`,
 * a single BList/vector splice either way so their relative order among
 * each other survives regardless of which of the four positions below they
 * land at. */
void MacroOutlineView::InsertCommandsAt(std::vector<BMessage> &commands, const MacroTargetPosition &pos)
{
	if (commands.empty())
		return;
	if (!pos.valid) {
		for (size_t i = 0; i < commands.size(); i++)
			fCommands.AddItem(new BMessage(commands[i]));
	} else if (pos.isCommandRow) {
		MacroPath	selfPath(pos.containerPath);
		if (pos.fieldName.Length() > 0) {
			MacroPathStep	step; step.field = pos.fieldName; step.index = pos.fieldIndex;
			selfPath.push_back(step);
		}
		BMessage	ownData	= MacroOutlineView_ResolveContainer(&fCommands,pos.topLevel,selfPath);
		const char	*targetName	= NULL;
		ownData.FindString("Command::Name",&targetName);

		if (IsContainerCommandName(targetName)) {
			// dropped onto a container command - becomes its LAST
			// subcommand(s) (user report - runs after whatever's already
			// there, not before)
			BMessage	container	= MacroOutlineView_ResolveContainer(&fCommands,pos.topLevel,selfPath);
			std::vector<BMessage>	entries;
			ExtractMessageEntries(&container,"PCommand::subPCommand",&entries);
			entries.insert(entries.end(),commands.begin(),commands.end());
			ReplaceMessageEntries(&container,"PCommand::subPCommand",entries);
			WriteContainer(&fCommands,pos.topLevel,selfPath,container);
		} else if (pos.isTopLevelCommand) {
			// sibling(s) of a top-level command
			int32	at	= pos.topLevel + (pos.lowerHalf ? 1 : 0);
			for (size_t i = 0; i < commands.size(); i++)
				fCommands.AddItem(new BMessage(commands[i]),at+(int32)i);
		} else if (pos.fieldName == "PCommand::subPCommand") {
			// sibling(s) of a subcommand
			BMessage	container	= MacroOutlineView_ResolveContainer(&fCommands,pos.topLevel,pos.containerPath);
			std::vector<BMessage>	entries;
			ExtractMessageEntries(&container,"PCommand::subPCommand",&entries);
			int32	at	= pos.fieldIndex + (pos.lowerHalf ? 1 : 0);
			if (at > (int32)entries.size())
				at	= (int32)entries.size();
			entries.insert(entries.begin()+at,commands.begin(),commands.end());
			ReplaceMessageEntries(&container,"PCommand::subPCommand",entries);
			WriteContainer(&fCommands,pos.topLevel,pos.containerPath,container);
		} else {
			for (size_t i = 0; i < commands.size(); i++)
				fCommands.AddItem(new BMessage(commands[i]),pos.topLevel+1+(int32)i);
		}
	} else {
		// dropped on a field/chip/block row - falls back to the top of
		// that row's own top-level command
		for (size_t i = 0; i < commands.size(); i++)
			fCommands.AddItem(new BMessage(commands[i]),pos.topLevel+1+(int32)i);
	}

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
	InsertCommandAt(newCommand,ResolveTargetPosition(this,targetRow,where));
}


void MacroOutlineView::MoveCommandRow(int32 sourceTopLevel, const MacroPath &sourcePath,
	int32 targetRow, BPoint where)
{
	MacroTargetPosition	pos	= ResolveTargetPosition(this,targetRow,where);

	// refuse a drop onto the source's own current row, or into one of its
	// own subcommands - either a no-op or actual self-nesting corruption
	if (pos.valid && (pos.topLevel == sourceTopLevel)) {
		MacroPath	targetSelf(pos.containerPath);
		if (pos.fieldName.Length() > 0) {
			MacroPathStep	step; step.field = pos.fieldName; step.index = pos.fieldIndex;
			targetSelf.push_back(step);
		}
		bool	within	= targetSelf.size() >= sourcePath.size();
		for (size_t i = 0; within && (i < sourcePath.size()); i++)
			if ((targetSelf[i].field != sourcePath[i].field) || (targetSelf[i].index != sourcePath[i].index))
				within	= false;
		if (within) {
			beep();
			return;
		}
	}

	BMessage	sourceCopy	= MacroOutlineView_ResolveContainer(&fCommands,sourceTopLevel,sourcePath);

	if (sourcePath.empty()) {
		// a top-level command - removing it shifts every later top-level
		// index (including the target's, if it's one) down by one
		BMessage	*old	= (BMessage*)fCommands.RemoveItem(sourceTopLevel);
		delete old;
		if (pos.topLevel > sourceTopLevel)
			pos.topLevel--;
	} else {
		MacroPath	parentPath(sourcePath.begin(),sourcePath.end()-1);
		const MacroPathStep	&lastStep	= sourcePath.back();
		BMessage	parent	= MacroOutlineView_ResolveContainer(&fCommands,sourceTopLevel,parentPath);
		std::vector<BMessage>	entries;
		ExtractMessageEntries(&parent,lastStep.field.String(),&entries);
		if (lastStep.index < (int32)entries.size())
			entries.erase(entries.begin()+lastStep.index);
		ReplaceMessageEntries(&parent,lastStep.field.String(),entries);
		WriteContainer(&fCommands,sourceTopLevel,parentPath,parent);
		// a sibling still under the very same parent shifts down by one
		if ((pos.topLevel == sourceTopLevel) && (pos.fieldName == lastStep.field)
				&& (pos.containerPath.size() == parentPath.size())) {
			bool	sameParent	= true;
			for (size_t i = 0; sameParent && (i < parentPath.size()); i++)
				if ((pos.containerPath[i].field != parentPath[i].field) || (pos.containerPath[i].index != parentPath[i].index))
					sameParent	= false;
			if (sameParent && (pos.fieldIndex > lastStep.index))
				pos.fieldIndex--;
		}
	}

	InsertCommandAt(new BMessage(sourceCopy),pos);
}


/** True if `path` (at `topLevel`) is `ancestorPath` itself or nested inside
 * it - shared by MoveCommandRows()' two uses of the same check (dropping
 * one selected command into another, and refusing a drop into any of
 * their own subtrees). `ancestorPath` empty always matches (a top-level
 * command's subtree is everything under it). */
static bool IsWithin(int32 topLevel, const MacroPath &path, int32 ancestorTopLevel, const MacroPath &ancestorPath)
{
	if (topLevel != ancestorTopLevel)
		return false;
	if (ancestorPath.size() > path.size())
		return false;
	for (size_t i = 0; i < ancestorPath.size(); i++)
		if ((path[i].field != ancestorPath[i].field) || (path[i].index != ancestorPath[i].index))
			return false;
	return true;
}


void MacroOutlineView::MoveCommandRows(std::vector<std::pair<int32,MacroPath> > &sources,
	int32 targetRow, BPoint where)
{
	// a source nested inside another selected source is skipped - moving
	// the ancestor already carries it along, and resolving it separately
	// afterward would read from a parent that's already been relocated
	std::vector<std::pair<int32,MacroPath> >	kept;
	for (size_t i = 0; i < sources.size(); i++) {
		bool	nested	= false;
		for (size_t j = 0; (j < sources.size()) && !nested; j++)
			if ((i != j) && (sources[j].second.size() < sources[i].second.size())
					&& IsWithin(sources[i].first,sources[i].second,sources[j].first,sources[j].second))
				nested	= true;
		if (!nested)
			kept.push_back(sources[i]);
	}
	if (kept.empty())
		return;

	MacroTargetPosition	pos	= ResolveTargetPosition(this,targetRow,where);

	// refuse a drop onto (or into) any kept source's own subtree - same
	// reasoning as the single-row guard in MoveCommandRow()
	if (pos.valid) {
		MacroPath	targetSelf(pos.containerPath);
		if (pos.fieldName.Length() > 0) {
			MacroPathStep	step; step.field = pos.fieldName; step.index = pos.fieldIndex;
			targetSelf.push_back(step);
		}
		for (size_t i = 0; i < kept.size(); i++)
			if (IsWithin(pos.topLevel,targetSelf,kept[i].first,kept[i].second)) {
				beep();
				return;
			}
	}

	// the moved commands' own relative order (lowest (topLevel,fieldIndex)
	// first) - captured from the still-intact tree, before anything below
	// removes/shifts any of it
	std::vector<BMessage>	orderedCopies;
	std::vector<std::pair<int32,MacroPath> >	byOriginalOrder(kept);
	while (!byOriginalOrder.empty()) {
		size_t	first	= 0;
		for (size_t i = 1; i < byOriginalOrder.size(); i++) {
			int32	firstTop	= byOriginalOrder[first].first;
			int32	iTop		= byOriginalOrder[i].first;
			int32	firstIdx	= byOriginalOrder[first].second.empty() ? -1 : byOriginalOrder[first].second.back().index;
			int32	iIdx		= byOriginalOrder[i].second.empty() ? -1 : byOriginalOrder[i].second.back().index;
			if ((iTop < firstTop) || ((iTop == firstTop) && (iIdx < firstIdx)))
				first	= i;
		}
		orderedCopies.push_back(MacroOutlineView_ResolveContainer(&fCommands,
			byOriginalOrder[first].first,byOriginalOrder[first].second));
		byOriginalOrder.erase(byOriginalOrder.begin()+first);
	}

	// remove every kept source, highest (topLevel,fieldIndex) first (see
	// DeleteSelectedRows() - the same removal order that never needs an
	// already-processed source's own identity corrected afterward),
	// adjusting `pos` after each removal exactly like MoveCommandRow()
	// does for its one source
	std::vector<std::pair<int32,MacroPath> >	toRemove(kept);
	while (!toRemove.empty()) {
		size_t	last	= 0;
		for (size_t i = 1; i < toRemove.size(); i++) {
			int32	lastTop	= toRemove[last].first;
			int32	iTop	= toRemove[i].first;
			int32	lastIdx	= toRemove[last].second.empty() ? -1 : toRemove[last].second.back().index;
			int32	iIdx	= toRemove[i].second.empty() ? -1 : toRemove[i].second.back().index;
			if ((iTop > lastTop) || ((iTop == lastTop) && (iIdx > lastIdx)))
				last	= i;
		}
		int32		srcTop	= toRemove[last].first;
		MacroPath	srcPath	= toRemove[last].second;

		if (srcPath.empty()) {
			BMessage	*old	= (BMessage*)fCommands.RemoveItem(srcTop);
			delete old;
			if (pos.topLevel > srcTop)
				pos.topLevel--;
		} else {
			MacroPath	parentPath(srcPath.begin(),srcPath.end()-1);
			const MacroPathStep	&lastStep	= srcPath.back();
			BMessage	parent	= MacroOutlineView_ResolveContainer(&fCommands,srcTop,parentPath);
			std::vector<BMessage>	entries;
			ExtractMessageEntries(&parent,lastStep.field.String(),&entries);
			if (lastStep.index < (int32)entries.size())
				entries.erase(entries.begin()+lastStep.index);
			ReplaceMessageEntries(&parent,lastStep.field.String(),entries);
			WriteContainer(&fCommands,srcTop,parentPath,parent);
			if ((pos.topLevel == srcTop) && (pos.fieldName == lastStep.field)
					&& (pos.containerPath.size() == parentPath.size())) {
				bool	sameParent	= true;
				for (size_t i = 0; sameParent && (i < parentPath.size()); i++)
					if ((pos.containerPath[i].field != parentPath[i].field) || (pos.containerPath[i].index != parentPath[i].index))
						sameParent	= false;
				if (sameParent && (pos.fieldIndex > lastStep.index))
					pos.fieldIndex--;
			}
		}
		toRemove.erase(toRemove.begin()+last);
	}

	InsertCommandsAt(orderedCopies,pos);
}


// ------------------------------------------------------------- BView etc --

void MacroOutlineView::MouseDown(BPoint where)
{
	uint32	buttons	= 0;
	if ((Window() != NULL) && (Window()->CurrentMessage() != NULL))
		Window()->CurrentMessage()->FindInt32("buttons",(int32*)&buttons);
	int32	index	= IndexOf(where);
	MacroRowItem	*item	= (index >= 0) ? (MacroRowItem*)ItemAt(index) : NULL;
	CloseOverlay(true);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) && (item != NULL) && (item->Kind() != kRowAddField)) {
		// right-clicking a row already part of a multi-selection keeps
		// that whole selection (so Delete below acts on all of it);
		// right-clicking outside it replaces the selection with just
		// this one row, same as a plain left click would
		if (!item->IsSelected())
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
		int32	selectedCount	= 0;
		for (int32 s = 0; CurrentSelection(s) >= 0; s++)
			selectedCount++;
		BString	deleteLabel(B_TRANSLATE("Delete"));
		if (selectedCount > 1)
			deleteLabel.SetToFormat(B_TRANSLATE("Delete %ld rows"),(long)selectedCount);
		BPopUpMenu	*menu	= new BPopUpMenu("rowMenu",false,false);
		BMenuItem	*up		= new BMenuItem(B_TRANSLATE("Move Up"),upMsg);
		BMenuItem	*down	= new BMenuItem(B_TRANSLATE("Move Down"),downMsg);
		// moving stays single-row (see MacroOutlineView.h) - disabled
		// outright on a multi-selection instead of silently only moving
		// the one row that happened to be right-clicked
		up->SetEnabled((selectedCount <= 1) && (i > 0));
		down->SetEnabled((selectedCount <= 1) && (i < count-1));
		menu->AddItem(up);
		menu->AddItem(down);
		menu->AddSeparatorItem();
		menu->AddItem(new BMenuItem(deleteLabel.String(),new BMessage('mvDl')));
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

	// Shift/Ctrl held means "extend the selection", not "edit this value" -
	// even when the click itself lands on the value portion of a field row
	// that's now part of a multi-selection
	int32	modifiers	= 0;
	if ((Window() != NULL) && (Window()->CurrentMessage() != NULL))
		Window()->CurrentMessage()->FindInt32("modifiers",&modifiers);
	bool	extendingSelection	= (modifiers & (B_SHIFT_KEY | B_CONTROL_KEY | B_COMMAND_KEY)) != 0;

	if ((item != NULL) && (item->Kind() == kRowField) && !extendingSelection) {
		if (item->FieldType() == B_BOOL_TYPE) {
			ToggleBoolField(index);
		} else if (where.x >= ValuePixelX(item,ItemFrame(index))) {
			// one click, right on the value itself, starts editing it in
			// place (user report: double-click-anywhere-on-the-row felt
			// disconnected from "edit this value" - clicking the label
			// part still just selects the row, same as any other row)
			BeginOverlayEdit(index);
		}
	}
}


void MacroOutlineView::KeyDown(const char *bytes, int32 numBytes)
{
	if ((numBytes == 1) && ((bytes[0] == B_DELETE) || (bytes[0] == B_BACKSPACE))) {
		DeleteSelectedRows();
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
	MacroRowItem	*item	= (MacroRowItem*)ItemAt(index);
	if ((item == NULL) || (item->Kind() != kRowCommand))
		return false;
	CloseOverlay(true);
	BMessage	drag(B_SIMPLE_DATA);
	drag.AddBool(kCommandMoveDragMarker,true);

	// dragging a row that's part of a multi-selection of commands carries
	// the whole selection along (user report); dragging one that isn't -
	// even while other, unrelated rows happen to be selected - only ever
	// moves that one, same as everywhere else in Haiku
	int32	sourceCount	= 0;
	if (item->IsSelected()) {
		for (int32 s = 0; ; s++) {
			int32	full	= CurrentSelection(s);
			if (full < 0)
				break;
			MacroRowItem	*selectedItem	= (MacroRowItem*)FullListItemAt(full);
			if ((selectedItem != NULL) && (selectedItem->Kind() == kRowCommand)) {
				BMessage	source;
				AddFieldTargetFields(&source,selectedItem->TopLevelIndex(),selectedItem->SelfPath());
				drag.AddMessage("source",&source);
				sourceCount++;
			}
		}
	}
	if (sourceCount == 0) {
		BMessage	source;
		AddFieldTargetFields(&source,item->TopLevelIndex(),item->SelfPath());
		drag.AddMessage("source",&source);
	}
	DragMessage(&drag,ItemFrame(index));
	return true;
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
		case 'mvDl':
			DeleteSelectedRows();
			return;
		case 'mvAF': {
			const char	*field	= NULL;
			int32		type	= 0;
			message->FindInt32("type",&type);
			MacroPath	path;
			int32		topLevel	= ReadFieldTargetMessage(message,&path);
			if (message->FindString("field",&field) == B_OK)
				AddNamedField(topLevel,path,field,(type_code)type);
			return;
		}
		case 'mvFT': {
			// freeform type picked (a generic block's own "+ Feld
			// hinzufügen" - see ShowAddFieldMenu()) - still needs a name,
			// which has no fixed list to offer here (no schema - see
			// MacroText.h), so it's typed in, same as renaming a macro
			int32		type	= 0;
			MacroPath	path;
			int32		topLevel	= ReadFieldTargetMessage(message,&path);
			message->FindInt32("type",&type);
			InputRequest	*request	= new InputRequest(B_TRANSLATE("Add field"),
				B_TRANSLATE("Name"),"",B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
			char	*input		= NULL;
			bool	accepted	= (request->Go(&input) < 1) && (input != NULL) && (input[0] != '\0');
			BString	name(accepted ? input : "");
			delete[] input;
			request->Lock();
			request->Quit();
			if (accepted)
				AddNamedField(topLevel,path,name.String(),(type_code)type);
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

	if (message->WasDropped() && message->HasBool(kCommandMoveDragMarker)) {
		std::vector<std::pair<int32,MacroPath> >	sources;
		BMessage	source;
		for (int32 i = 0; message->FindMessage("source",i,&source) == B_OK; i++) {
			MacroPath	path;
			int32		topLevel	= ReadFieldTargetMessage(&source,&path);
			sources.push_back(std::make_pair(topLevel,path));
			source.MakeEmpty();
		}
		if (sources.empty())
			return;
		BPoint	dropPoint	= message->DropPoint();
		ConvertFromScreen(&dropPoint);
		int32	target	= IndexOf(dropPoint);
		if (sources.size() == 1)
			MoveCommandRow(sources[0].first,sources[0].second,target,dropPoint);
		else
			MoveCommandRows(sources,target,dropPoint);
		return;
	}

	BOutlineListView::MessageReceived(message);
}
