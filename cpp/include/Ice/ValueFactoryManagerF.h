// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_MANAGER_F_H
#define ICE_VALUE_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class ValueFactoryManagerI;
void incRef(ValueFactoryManagerI*);
void decRef(ValueFactoryManagerI*);
typedef __Ice::Handle<ValueFactoryManagerI> ValueFactoryManager;

}

#endif
