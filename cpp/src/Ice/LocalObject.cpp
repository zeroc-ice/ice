// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LocalObject.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(LocalObject* p) { p->__incRef(); }
void IceInternal::decRef(LocalObject* p) { p->__decRef(); }
