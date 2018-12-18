// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Connector.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Connector::~Connector()
{
    // Out of line to avoid weak vtable
}

IceUtil::Shared* IceInternal::upCast(Connector* p) { return p; }
