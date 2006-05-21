#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include <interface/Window.h>

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

/**
 * @class AboutWindow
 * @brief Implementaion of the About window from ProjectConceptor
 * @todo implemt the hole class
 * @bug dont work :-)
 */
class AboutWindow: public BWindow
{

public:
						AboutWindow();
						~AboutWindow();
protected:
		void			ChangeLanguage(void);
private:
};
#endif
