#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "PCommand.h"
#include "BasePlugin.h"
#include "Indexer.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <support/List.h>
#include <support/String.h>

#include <map>
using namespace std;

class PDocument;


/**
 * @class PCommandManager
 * @brief  a Class wich manage all PCommands in a Document
 * 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/31
 * @Contact: mail@projectconceptor.de
 *
 * Created on: Wed Jun 05  2005
 *
 *
 */

class PCommandManager 
{

public:
						PCommandManager(PDocument *initDoc);
	virtual				~PCommandManager(void);

			void		StartMacro(void);
			void		StopMacro(void);
			/** NULL when not currently recording; otherwise the in-progress
			 * macro (same object StopMacro() would name and add to
			 * macroList) - lets a test (or a "still recording" UI
			 * indicator) inspect what's been captured so far without
			 * StopMacro()'s own blocking name-prompt dialog. */
			BMessage*	GetRecording(void){return recording;};
			/** Plays every command of `makro`. B_OK only if all of them ran and
			 * every node reference resolved; *report (if given) then says what
			 * happened in words. The same text goes to the editors
			 * (P_C_MACRO_PLAYED), and a failure also gets a non-blocking alert
			 * when the document has a UI - a macro that silently does nothing
			 * is the worst outcome. */
			status_t	PlayMacro(BMessage *makro, BString *report = NULL);
			/** looks up a macro by its "Name" field in macroList and plays
			 * it if found; logs and does nothing otherwise - a document not
			 * having a macro under an app-wide shortcut's name is a normal
			 * case, not an error. Interactive (see PlayMacroInteractive()),
			 * same as the menu's "Macro > Play" - an app-wide shortcut is
			 * just another GUI trigger.
			 */
			void		PlayMacroByName(const char *name);
			/** The GUI/interactive counterpart to PlayMacro() - same
			 * "Macro::Commmand" list, same Execute()/report/P_C_MACRO_PLAYED
			 * outcome, but spread across one P_C_MACRO_PLAY_STEP dispatch
			 * per top-level command instead of one synchronous call that
			 * holds the document locked (via BLooper::DispatchMessage()'s
			 * own implicit lock around the whole thing) for the entire
			 * macro - that's what lets GraphEditor redraw between steps.
			 * PlayMacro() itself stays exactly as it was
			 * (tests and any future programmatic caller keep its simple
			 * synchronous contract); this is only for the two places that
			 * trigger a play from outside already-running command code
			 * (PDocument's P_C_MACRO_TYPE/P_C_PLAY_MACRO_BY_NAME handlers).
			 * Refuses (beeps, logs) if a previous interactive play is
			 * still in progress - never stomps a live one's state. */
			void		PlayMacroInteractive(BMessage *makro);
			/** Runs exactly the next command of an in-progress
			 * PlayMacroInteractive() call, then posts another
			 * P_C_MACRO_PLAY_STEP for the one after (or finishes - same
			 * report/broadcast/alert PlayMacro() itself ends with - if that
			 * was the last one). A no-op if nothing is currently playing
			 * interactively (a stray/duplicate message, e.g. arriving after
			 * a failure already ended the run). Called from PDocument's own
			 * P_C_MACRO_PLAY_STEP handler - not meant to be called directly
			 * from anywhere else. */
			void		PlayMacroStep(void);
	virtual	status_t	RegisterPCommand(BasePlugin *commandPlugin);
	virtual	void		UnregisterPCommand(char *name);

	virtual status_t	Archive(BMessage *archive, bool deep = true);
	virtual	status_t	SetMacroList(BList *newMacroList);
	virtual	status_t	SetUndoList(BList *newUndoList);

	virtual void		SetUndoIndex(uint32 newIndex){undoStatus=newIndex;};
	virtual	PCommand*	GetPCommand(char* name);
	virtual BList*		GetUndoList(void){return undoList;};
	virtual BList*		GetMacroList(void){return macroList;};
	virtual int32		GetUndoIndex(void){return undoStatus;};

	virtual	void		Undo(BMessage *undo);
	virtual	void		Redo(BMessage *redo);

	virtual	status_t	Execute(BMessage *settings);

	/** NULL outside macro playback; otherwise a flat BMessage of
	 * named values (any BMessage-native type - a repeated pointer
	 * field for a remembered selection, a string for an Ask() answer,
	 * an int32 loop counter, ...) that Ask/Remember/Repeat/ForEach
	 * (#135) write into as a macro plays, and ResolveBindings() reads
	 * from. Scoped to one PlayMacro() call - see there. */
	virtual	BMessage*	GetValueContext(void){return valueContext;};

	/** #135: if `settings` carries a "PCommand::bindings" submessage
	 * (built by MacroText.cpp's "fieldName=$variableName" syntax -
	 * see ParseCommands()), replaces each named field's own content
	 * with whatever the matching variable currently holds in
	 * GetValueContext(), copied verbatim regardless of type. A no-op
	 * outside macro playback (GetValueContext() == NULL) - bindings
	 * only mean anything during PlayMacro(). Called on every
	 * command's settings right before its own Do() - both here, for
	 * a command reached via Execute(), and in PCommand::
	 * RunSubCommandsOnce(), for one reached as somebody else's
	 * subPCommand child - so a bound field resolves correctly no
	 * matter how deeply nested the command carrying it is. */
	virtual	void		ResolveBindings(BMessage *settings, PCommand *forCommand = NULL);
	/** Replaces "${name}" inside every string field of `message` - nested
	 * blocks included, except "PCommand::subPCommand" (a loop body is a
	 * template, interpolated per run) and "PCommand::bindings" - with the
	 * value context's "name" as text: strings as-is, whole numbers without
	 * decimals, other numbers via %g, bools as true/false. "$${" is a
	 * literal "${". An unknown, multi-valued or non-text variable is left
	 * as written and reported in the playback result. A no-op outside
	 * playback, like ResolveBindings(). Returns true if anything changed. */
	virtual	bool		InterpolateStrings(BMessage *message);
	/** true if any string field of `message` (recursively, same skips as
	 * InterpolateStrings()) contains a "${" placeholder. */
	virtual	bool		HasInterpolation(const BMessage *message);
	/** a node registered under some id during playback was replaced by a
	 * copy (see Insert::Do()) - later "@id" references reach the copy.
	 * A no-op outside playback. */
	virtual	void		RepointReplayNode(BMessage *from, BMessage *to);

	virtual	int32		CountPCommand(void){return commandMap.size();};
	virtual	PCommand*	PCommandAt(int32 index);
	/**
	 * Concatenates every registered command's own PropertyInfo() entries
	 * into one BPropertyInfo, for #55's scripting suite (PDocument) and
	 * the MacroEditor's DSL field validation - the single canonical
	 * schema source, not duplicated between the two.
	 *
	 * Caller owns and must `delete` the returned BPropertyInfo, but must
	 * NOT try to free the underlying property_info array itself - it's
	 * kept alive on this object (fPropertyInfoArray, rebuilt on every
	 * call) since every command's name/usage/field-name strings inside
	 * it are pointers into that command's own `static const` array
	 * (string literals), not individually heap-allocated. The returned
	 * BPropertyInfo is therefore constructed with freeOnDelete=false -
	 * BPropertyInfo's freeOnDelete=true calls plain free() on every one
	 * of those string pointers individually (matching how Unflatten()
	 * malloc()s them), which crashes on a string literal's address.
	 */
	virtual	BPropertyInfo	*BuildPropertyInfo(void);
	
	virtual PDocument*	BelongTo(void){return doc;};

protected:
	virtual void		Init(void);
			/** Shared by PlayMacro() and PlayMacroStep()'s own finish -
			 * same report text, same P_C_MACRO_PLAYED broadcast, same
			 * failure alert, built from whichever of the two actually
			 * drove the playback. Returns the overall status (what
			 * PlayMacro() itself returns). */
			status_t	FinishMacroPlayback(int32 playedCount, status_t err,
							const BString &failedCommand, int32 unresolvedCount,
							BString *report);
			bool		InterpolateText(const BString &text, BString *result);
			bool		FormatVariable(const char *name, BString *text, BString *error);
			/** see InterpolateStrings() - reset when a playback starts,
			 * reported by FinishMacroPlayback() */
			int32		interpolationErrors;
			BString		firstInterpolationError;

			BList		*undoList;
			BList		*macroList;
			/** owned by this object, not by any BPropertyInfo wrapper -
			 * see BuildPropertyInfo(). Freed and rebuilt on every
			 * BuildPropertyInfo() call, freed once more in ~PCommandManager(). */
			property_info	*fPropertyInfoArray;
			int32		undoStatus;
			map<BString, PCommand*>	 commandMap;
			PDocument	*doc;
			BMessage	*recording;
			Indexer		*macroIndexer;
			/** see GetValueContext() - owned and scoped entirely by
			 * PlayMacro(), NULL the rest of the time. */
			BMessage	*valueContext;
			/** the current PlayMacro() call's own Indexer, not owned here
			 * (PlayMacro() keeps the real pointer on its own stack, this is
			 * only ever a borrowed reference to it) - saved/restored around
			 * a nested PlayMacro() the same way GetValueContext() is, not
			 * cleared to NULL first the way that one is (a nested call gets
			 * its own fresh Indexer either way, nothing to guard against
			 * beyond restoring the outer one correctly). NULL outside macro
			 * playback. ResolveBindings() uses this to translate a bound
			 * "node" field's already-computed target id (see Calculate)
			 * into the live node/connection pointer the command's own Do()
			 * actually needs - the same translation Indexer::
			 * DeIndexCommand() does for a *literal* "node=@id" already in
			 * the macro, needed again here because a *bound* field never
			 * goes through DeIndexCommand() at all (see ResolveBindings()'s
			 * own comment on when it runs). */
			Indexer		*replayIndexer;

			/** Interactive (stepwise) playback state - see
			 * PlayMacroInteractive()/PlayMacroStep(). All NULL/zero
			 * whenever no interactive play is in progress; that's also
			 * how PlayMacroStep() recognizes a stray message and how
			 * PlayMacroInteractive() recognizes (and refuses) a second
			 * play trying to start while one is still running. */
			BMessage	*playingMacro;			// this run's own private copy
			int32		playingIndex;
			Indexer		*playingIndexer;
			status_t	playingErr;
			BString		playingFailedCommand;
private:

};
#endif
