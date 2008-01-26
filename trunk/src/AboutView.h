#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <interface/View.h>
#include <String.h>
 
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
			int kW, kH;
			BString fRevNumbRaw, fRevNumb;
			int kStrLength;


};

#endif
