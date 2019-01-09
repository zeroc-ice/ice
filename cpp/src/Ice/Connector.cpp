// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
