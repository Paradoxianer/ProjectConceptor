#ifndef CONFIG_ITEM_H
#define CONFIG_ITEM_H

/*
 * @author Paradoxon powered by Jesus Christ
 */
class ConfigItem
{

public:
						ConfigItem(type_code supportedType){type=supportedType;};
	virtual	BView*		ViewForValue(void)		= 0;
	virtual	type_code	GetSupportedType(void){return type;};

	virtual	void*		GetValue(void)				= 0;
	virtual void		SetValue(void* newValue)	= 0;
	
//	virtual	void		ValueChanged(void)		= 0;

protected:
		type_code	type;

private:
};
#endif
