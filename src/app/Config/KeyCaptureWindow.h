#ifndef KEY_CAPTURE_WINDOW_H
#define KEY_CAPTURE_WINDOW_H

#include <interface/Window.h>

/** Small BAlert-like dialog (same blocking Go() idiom as InputRequest)
 * that captures the next key combination the user presses. Escape
 * cancels without changing anything.
 */
class KeyCaptureWindow : public BWindow
{
public:
						KeyCaptureWindow(const char *prompt);

	/** blocks the calling thread until a key is pressed or the dialog is
	 * cancelled; returns true and fills in key and modifiers on success
	 */
	virtual	bool		Go(int32 *key,int32 *modifiers);

			void		Capture(int32 key,int32 modifiers);
			void		Cancel(void);

private:
			int32		fKey;
			int32		fModifiers;
			bool		fDone;
			bool		fCancelled;
};
#endif
