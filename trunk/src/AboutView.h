#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <interface/View.h>
 
class AboutView: public BView
{

	public:
			AboutView(BRect frame);
			~AboutView();
			void Draw(BRect updated);
			
	protected:
			//
			
	private:
			//


};

#endif
