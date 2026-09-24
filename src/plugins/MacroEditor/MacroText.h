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

#endif
