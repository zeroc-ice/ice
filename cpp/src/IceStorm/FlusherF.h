// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FLUSHER_F_H
#define FLUSHER_F_H

#include <IceUtil/Handle.h>

namespace IceStorm
{

class Flusher;
typedef IceUtil::Handle<Flusher> FlusherPtr;

void incRef(Flusher*);
void decRef(Flusher*);

} // End namespace IceStorm

#endif
