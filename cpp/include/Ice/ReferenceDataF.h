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

namespace _Ice
{

class ReferenceDataI;
void ICE_API _incRef(ReferenceDataI*);
void ICE_API _decRef(ReferenceDataI*);
typedef _Ice::Handle<ReferenceDataI> ReferenceData;

}

#endif
