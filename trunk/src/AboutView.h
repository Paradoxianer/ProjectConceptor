/*
 * Copyright 2008 ProjectConceptor All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ralf Schülke, teammaui@web.de
 */

#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <View.h>
#include <TextView.h>
#include <String.h>
#include <Bitmap.h>

#include "AboutLogo_64.h"
#include "AboutURLView.h"
 
class AboutView: public BView
{

	public:
			AboutView(BRect frame);
			~AboutView();
			void Draw(BRect updateRect);
			
	protected:
			//
			
	private:
			//
			float kW, kH;
			BBitmap* fBitmapType;
			BTextView* fTitleView;
			BFont fTitleFont;
			BString fRevNumb;
			BTextView* fSvnInfoView;
			BFont fSvnInfoFont;
			
			BString fDeveloperTitleString;
			BString fContributorTitleString;
			BString fTranslatorTitleString;
			BString fWebsiteTitleString;
			int kDL, kCL, kTL, kWL;
			
			BTextView* fDeveloperTitleView;
			BFont fDeveloperTitleFont;
			
			BString fDeveloperNameString;
			BTextView* fDeveloperNameView;
			BFont fDeveloperNameFont;
			
			BTextView* fContributorTitleView;
			BFont fContributorTitleFont;
			
			BString fContributorNameString;
			BTextView* fContributorNameView;
			BFont fContributorNameFont;
			
			BTextView* fTranslatorTitleView;
			BFont fTranslatorTitleFont;
			
			BString fTranslatorNameString;
			BTextView* fTranslatorNameView;
			BFont fTranslatorNameFont;
			
			BTextView* fWebsiteTitleView;
			BFont fWebsiteTitleFont;
			
			BString fWebsiteNameString;
			BTextView* fWebsiteNameView;
			BFont fWebsiteNameFont;
			
			BString fContactTitleString;
			BTextView* fContactTitleView;
			BFont fContactTitleFont;
			
			URLView* fWebView;
			URLView* fMailView;
			
};

#endif
