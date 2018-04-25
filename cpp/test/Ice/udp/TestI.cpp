// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
TestIntfI::ping(ICE_IN(Test::PingReplyPrxPtr) reply, const Current&)
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
TestIntfI::sendByteSeq(ICE_IN(Test::ByteSeq), ICE_IN(Test::PingReplyPrxPtr) reply, const Current&)
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
TestIntfI::pingBiDir(ICE_IN(Ice::Identity) id, const Ice::Current& current)
{
    try
    {
        //
        // Ensure sending too much data doesn't cause the UDP connection
        // to be closed.
        //
        try
        {
            Test::ByteSeq seq;
            seq.resize(32 * 1024);
            ICE_UNCHECKED_CAST(Test::TestIntfPrx, current.con->createProxy(id))->sendByteSeq(seq, 0);
        }
        catch(const DatagramLimitException&)
        {
            // Expected.
        }

        //
        // Send the reply through the incoming connection.
        //
#ifdef ICE_CPP11_MAPPING
        Ice::uncheckedCast<Test::PingReplyPrx>(current.con->createProxy(id))->replyAsync();
#else
        Test::PingReplyPrx::uncheckedCast(current.con->createProxy(id))->begin_reply();
#endif
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        assert(false);
    }
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
