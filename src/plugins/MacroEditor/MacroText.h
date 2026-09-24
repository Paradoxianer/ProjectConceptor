#ifndef MACRO_TEXT_H
#define MACRO_TEXT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <support/List.h>
#include <support/String.h>

class PCommandManager;

/**
 * Guided text DSL for recorded macros (#55) - pure functions, no BView/
 * plugin dependency, unit-testable the same way GroupBoundary.cpp/
 * SmartGuides.cpp are (see SmartGuidesTest.h).
 *
 * Operates on the "Macro::Commmand" list a macro BMessage carries (see
 * PCommandManager::PlayMacro()) - already-indexed command settings, node/
 * connection pointers already replaced by stable int32 ids, so this never
 * touches a live pointer.
 *
 * One BMessage entry per line - a command, one of its fields, or a nested
 * field's own field - indentation is nesting depth:
 *
 *   Insert
 *     node=@1
 *     frame=[0,0,80,40]
 *     ~included_node
 *       Node::Name="A"
 *       Node::Frame=[0,0,80,40]
 *   Group
 *     node=@2
 *     deselect=true
 *     Move
 *       dx=10.5
 *       dy=-3
 *
 * - A bare name is a command line - its own fields and, indented the same
 *   one level deeper, any "PCommand::subPCommand" children (the mechanism
 *   Batch relies on) follow below it.
 * - A "fieldName=value" line sets one field on whichever command or "~"
 *   block is open at that indent depth - it never has children of its
 *   own. Value syntax: true/false (bool); a plain integer (int32,
 *   default) or with an explicit i8/i16/i64 suffix; a number with a
 *   decimal point (float) or an explicit d suffix (double); "..." with
 *   \" and \\ escapes (string); (x,y) (BPoint); [l,t,r,b] (BRect); @<id>
 *   (the "node" field specifically - an already-indexed node/connection
 *   id, stored as int32, not a live pointer).
 * - A "~fieldName" line is a nested B_MESSAGE_TYPE field (e.g.
 *   Indexer-embedded "included_node", ChangeValue's "valueContainer") -
 *   its own fields follow indented one level deeper, in the same
 *   line-per-entry shape, and can nest further the same way. There is no
 *   schema for this content (it is arbitrary node/value data, not a
 *   command), so values are accepted by whatever their own syntax implies
 *   rather than cross-checked.
 * - Anything else (a type not in the list above, in either a command's
 *   own fields or a "~" block's) is preserved losslessly but opaquely as
 *   raw:<type_code>:<base64> - never silently dropped, not meant to be
 *   hand-authored.
 * - Every command name is checked against the real PCommandManager
 *   registry, and every field name/type against that command's own
 *   PropertyInfo() (ctypes[0].pairs[]) - the same schema the scripting
 *   suite publishes (see PDocument::GetSupportedSuites()).
 */

/** Appends one line/block per entry in `commands` (BList of BMessage*,
 * the same shape as a macro's "Macro::Commmand" entries) to `outText`. */
void		SerializeCommands(BList *commands, BString *outText);

/** Parses `text` back into a BList of freshly allocated BMessage* -
 * `outCommands` is appended to (empty it first if replacing). `registry`
 * is used to validate every command name and its fields' names/types.
 * On any error, `outCommands` is left untouched (nothing partially
 * added) and B_BAD_VALUE (or B_NAME_NOT_FOUND for an unknown command) is
 * returned with `*errorOut` naming the offending line/command/field -
 * callers must check the return value, not assume outCommands is usable
 * on failure.
 */
status_t	ParseCommands(const BString &text, BList *outCommands,
				PCommandManager *registry, BString *errorOut);

/** One short, working example of `commandName` in this DSL (NULL if there
 * is none) - shown under the command's description in the MacroEditor's
 * reference-list tooltip, since the description alone doesn't say how to
 * write it (user report). Kept next to the grammar it demonstrates; every
 * entry is checked against the real parser by MacroTextTest. */
const char*	CommandExampleText(const char *commandName);

/** Where (and how indented) a dragged-in command snippet goes: on a line
 * boundary of `text`, never mid-line - dropping onto the word "Find" used to
 * split it in two and break the command. `line` is the 0-based line under
 * the pointer; upper half of it (lowerHalf=false) -> before that line, lower
 * half -> after it and every deeper-indented line that belongs to it. The
 * snippet is indented to the target line's own depth (2 spaces per level).
 * Returns the insert position and the exact text to insert (every line
 * indented, newline-terminated; a leading newline if the target is the very
 * last line and doesn't end in one). */
void		SnippetInsertion(const BString &text, int32 line, bool lowerHalf,
				const BString &snippet, int32 *outOffset, BString *outText);

/** How many characters to add (positive) or remove (negative) at the start of
 * `line` to move it `levels` indent levels (2 spaces each) in or out. An
 * outdent never removes more than the line's own leading spaces, so a line
 * already at depth 0 stays put. */
int32		IndentChange(const BString &line, int32 levels);

/** A complete "Insert" command text with a ready node prototype embedded
 * (name, font, colors, frame) - what dragging Insert into the editor drops,
 * so it isn't a bare "node=@1" pointing at nothing. */
void		InsertPrototypeText(BString *out);

/** Highest node/connection id ("@N" reference or "this=N" field) in `text`,
 * -1 if there is none. */
int32		HighestReferencedId(const BString &text);

/** Renumbers an Insert prototype (InsertPrototypeText()) - its "node=@1" and
 * "this=1" lines - to `newId`, so several dropped prototypes don't all claim
 * to be node 1. */
void		RenumberInsertPrototype(BString *snippet, int32 newId);

#endif
