// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Acceptor.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Acceptor* p) { p->__incRef(); }
void IceInternal::decRef(Acceptor* p) { p->__decRef(); }
