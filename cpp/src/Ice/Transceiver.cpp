// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Transceiver.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Transceiver* p) { p->__incRef(); }
void IceInternal::decRef(Transceiver* p) { p->__decRef(); }
