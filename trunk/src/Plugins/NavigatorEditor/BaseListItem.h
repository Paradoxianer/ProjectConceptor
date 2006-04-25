#ifndef BASE_LIST_ITEM_H
#define BASE_LIST_ITEM_H

#include <interface/ListItem.h>

class BaseListItem : public BListItem
{
public:
						BaseListItem(type_code supportetType, uint32 level = 0, bool expanded = true):BListItem(level,expanded){type=supportetType;};
	virtual type_code	GetSupportedType(void){return type;};
	virtual void		ValueChanged(void){};

protected:
			type_code	type;
};
#endif
