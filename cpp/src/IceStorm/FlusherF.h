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

} // End namespace IceStorm

//
// This is necessary so that FlusherPtr can be forward declared
//
namespace IceInternal
{

void incRef(IceStorm::Flusher*);
void decRef(IceStorm::Flusher*);

} // End namespace IceInternal

#endif
