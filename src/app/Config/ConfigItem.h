#ifndef CONFIG_ITEM_H
#define CONFIG_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */


#include <interface/Rect.h>
#include <interface/View.h>

class ConfigItem
{

public:
						ConfigItem(float labelWidht){lWidth=labelWidht;}
	virtual	void*		GetValue(void)				= 0;
	virtual void		SetValue(void* newValue)	= 0;
	
	virtual BRect		GetFrame(void)				= 0;
	virtual void		SetFrame(void)				= 0;
	
	virtual	void		SetLabelWidth(float lWidht)	= 0;
	virtual	void		GetLabelWidth(void)			= 0;
	
	virtual	status_t	AddToView(BView *)			= 0;
	virtual	status_t	RemoveFromView(BView *)		= 0;

protected:
			float		lWidth;

private:
};
#endif
