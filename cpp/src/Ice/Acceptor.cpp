// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
