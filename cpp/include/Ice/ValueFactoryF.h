// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_F_H
#define ICE_VALUE_FACTORY_F_H

#include <Ice/Handle.h>

namespace Ice { class ValueFactoryI; }

namespace __Ice
{

void ICE_API incRef(::Ice::ValueFactoryI*);
void ICE_API decRef(::Ice::ValueFactoryI*);

}

namespace Ice
{

typedef __Ice::Handle<ValueFactoryI> ValueFactory;

}

#endif
