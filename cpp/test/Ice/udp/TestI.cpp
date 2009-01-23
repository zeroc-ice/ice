// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

void
TestIntfI::ping(const Test::PingReplyPrx& reply, const Current& current)
{
    try
    {
        reply->reply();
    }
    catch(const Ice::Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::sendByteSeq(const Test::ByteSeq&, const Test::PingReplyPrx& reply, const Current& current)
{
    try
    {
        reply->reply();
    }
    catch(const Ice::Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
