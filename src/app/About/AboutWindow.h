#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include "AboutView.h"

#include <interface/Window.h>

/**
 * @class AboutWindow
 * @brief Implementaion of the About window from ProjectConceptor
 */
class AboutWindow: public BWindow
{

	public:
						AboutWindow();
						~AboutWindow();

	protected:
			void		ChangeLanguage(void);

	private:
			AboutView*	fAboutView;
};
#endif
