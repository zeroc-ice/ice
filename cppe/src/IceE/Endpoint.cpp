// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Endpoint.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Endpoint* p) { p->__incRef(); }
void IceInternal::decRef(Endpoint* p) { p->__decRef(); }
