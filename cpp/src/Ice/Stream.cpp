// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(InputStream* p) { p->__incRef(); }
void IceInternal::decRef(InputStream* p) { p->__decRef(); }
void IceInternal::incRef(OutputStream* p) { p->__incRef(); }
void IceInternal::decRef(OutputStream* p) { p->__decRef(); }
