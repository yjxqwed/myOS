#ifndef __PORT_H__
#define __PORT_H__

#include "types.h"

class Port {
protected:
    uint16_t port_number_;
public:
    Port(uint16_t port);
    virtual ~Port() = 0;
};

class Port8 : public Port {
public:
    void Write8();
};


#endif