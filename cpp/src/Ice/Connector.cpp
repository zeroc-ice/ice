// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Connector.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Connector* p) { p->__incRef(); }
void IceInternal::decRef(Connector* p) { p->__decRef(); }
