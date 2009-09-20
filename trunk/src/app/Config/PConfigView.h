#ifndef PCONFIGVIEW_H
#define PCONFIGVIEW_H

#include <View.h>
#include "PConfigurable.h"

class PConfigView : public PConfigurable , public BView {
    PConfigView (const char *name, uint32 flags, BLayout *layout=NULL): PConfigurable(), BView(name,flags,layout){}
}
#endif // PCONFIGVIEW_H
