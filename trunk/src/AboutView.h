#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <interface/View.h>
#include <TextView.h>
#include <String.h>
#include <Bitmap.h>
#include "AboutLogo_64.h"
 
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
			BBitmap* fBitmapType;
			BTextView* TitleView;
			BFont TitleFont;
			BString fRevNumb;
			BTextView* SvnInfoView;
			BFont SvnInfoFont;
			


};

#endif
