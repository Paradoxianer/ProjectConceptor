#ifndef FONT_VIEW_H_
#define FONT_VIEW_H_

#include <ListView.h>
#include <ListItem.h>
#include <Font.h>
#include <InterfaceDefs.h>

#include "FontDefs.h"

#define	PREVIEW_STR "AaBbCcDdEeFfGg - 1234567890"


class FontView : public BListView
{
public:
				FontView(void);
				~FontView(void);
	
	void		AttachedToWindow(void);
	void		MessageReceived(BMessage *msg);

	void		SetFontSize(uint16 size);
	void		SetUnderline(bool underline);
	void		SetStrikeout(bool strkeout);
	void		SetItalic(bool italic);
	void		SetOutlined(int32 outline);

	void		SelectFont(BFont &font);
	void		SelectFont(font_family family, font_style style, float size);

	void		SetFilter(char *newFilter);
	void		ResetFilter(void);	
	
private:	
	char		*filterString;
	void		RescanForFonts(void);
	void		DeleteAll(void);
	
};

class FontItem : public BListItem
{
public:
					FontItem(font_family myFamily);
	
	virtual void	DrawItem(BView *owner,
    		        BRect frame,
            		bool complete = false);
            char*	GetFamily(void){return family;};
            void	SetSize(float size){font.SetSize(size);};
			void	SetShear(float shear){font.SetShear(shear);};
			void	SetRotation(float rotation){font.SetRotation(rotation);};
			void	SetFalseBoldWidth(float width){font.SetFalseBoldWidth(width);};
			void	SetSpacing(uint8 spacing){font.SetSpacing(spacing);};
			void	SetEncoding(uint8 encoding){font.SetEncoding(encoding);};
			void	SetFace(uint16 face){font.SetFace(face);};
			void	SetFlags(uint32 flags){font.SetFlags(flags);};

    		float	Height() const;
			float	Width() const;

private:
		font_family	family;
		BFont		font;    
};


#endif
