// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Transceiver.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Transceiver* p) { return p; }

EndpointIPtr
IceInternal::Transceiver::bind()
{
    assert(false);
    return 0;
}
