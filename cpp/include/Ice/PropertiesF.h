// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROPERTIES_F_H
#define ICE_PROPERTIES_F_H

#include <Ice/Handle.h>

namespace Ice { class Properties; }

namespace __Ice
{

void ICE_API incRef(::Ice::Properties*);
void ICE_API decRef(::Ice::Properties*);

}

namespace Ice
{

typedef __Ice::Handle<Properties> Properties_ptr;

}

#endif
