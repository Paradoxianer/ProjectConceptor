#ifndef PCONFIGURABLE_H
#define PCONFIGURABLE_H

class PConfigurable{
    virtual	BMessage*		GetConfiguration(void) = 0;
    virtual void			SetConfiguration(BMessage *message) = 0;
};

#endif // PCONFIGURABLE_H
