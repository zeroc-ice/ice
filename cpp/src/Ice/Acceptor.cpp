// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Acceptor.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Acceptor::~Acceptor()
{
    // Out of line to avoid weak vtable
}

IceUtil::Shared* IceInternal::upCast(Acceptor* p) { return p; }
