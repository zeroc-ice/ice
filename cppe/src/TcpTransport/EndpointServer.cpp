
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Endpoint.h>
#include <Ice/Network.h>
#include <Ice/Acceptor.h>
#include <IceUtil/SafeStdio.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

AcceptorPtr
IceInternal::Endpoint::acceptor(EndpointPtr& endp) const
{
    Acceptor* p = new Acceptor(_instance, _host, _port);
    endp = new Endpoint(_instance, _host, p->effectivePort(), _timeout);
    return p;
}

bool
IceInternal::Endpoint::equivalent(const AcceptorPtr& acceptor) const
{
    return acceptor->equivalent(_host, _port);
}

