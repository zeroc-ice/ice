// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_DATA_F_H
#define ICE_REFERENCE_DATA_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class ReferenceDataI;
void ICE_API incRef(ReferenceDataI*);
void ICE_API decRef(ReferenceDataI*);
typedef __Ice::Handle<ReferenceDataI> ReferenceData;

}

#endif
