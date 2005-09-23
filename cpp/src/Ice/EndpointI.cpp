// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointI.h>

using namespace Ice;

void IceInternal::incRef(EndpointI* p) { p->__incRef(); }
void IceInternal::decRef(EndpointI* p) { p->__decRef(); }
