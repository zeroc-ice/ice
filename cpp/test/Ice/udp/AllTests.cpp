// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

class PingReplyI : public PingReply, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void
    reply(const Ice::Current&)
    {
        Lock sync(*this);
        ++_replies;
        notify();
    }

    void
    reset()
    {
         _replies = 0;
    }

    bool
    waitReply(int expectedReplies, const IceUtil::Time& timeout)
    {
        Lock sync(*this);
        IceUtil::Time end = IceUtil::Time::now() + timeout;
        while(_replies < expectedReplies)
        {
            IceUtil::Time delay = end - IceUtil::Time::now();
            if(delay > IceUtil::Time::seconds(0))
            {
                timedWait(delay);
            }
            else
            {
                break;
            }
        }
        return _replies == expectedReplies;
    }

private:

    int _replies;
};

typedef IceUtil::Handle<PingReplyI> PingReplyIPtr;

TestIntfPrx
allTests(const CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("ReplyAdapter.Endpoints", "udp -p 12030");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ReplyAdapter");
    PingReplyIPtr replyI = new PingReplyI;
    PingReplyPrx reply = PingReplyPrx::uncheckedCast(adapter->addWithUUID(replyI))->ice_datagram();
    adapter->activate();

    cout << "testing udp... " << flush;
    ObjectPrx base = communicator->stringToProxy("test:udp -p 12010")->ice_datagram();
    TestIntfPrx obj = TestIntfPrx::uncheckedCast(base);

    replyI->reset();
    obj->ping(reply);
    obj->ping(reply);
    obj->ping(reply);
    bool ret = replyI->waitReply(3, IceUtil::Time::seconds(2));
    test(ret == true);

    Test::ByteSeq seq;
    try
    {
        seq.resize(1024);
        while(true)
        {
            seq.resize(seq.size() * 2 + 10);
            replyI->reset();
            obj->sendByteSeq(seq, reply);
            replyI->waitReply(1, IceUtil::Time::seconds(10));
        }
    }
    catch(const DatagramLimitException&)
    {
        test(seq.size() > 16384);
    }

    communicator->getProperties()->setProperty("Ice.UDP.SndSize", "64000");
    seq.resize(50000);
    try
    {
        replyI->reset();
        obj->sendByteSeq(seq, reply);
        test(!replyI->waitReply(1, IceUtil::Time::milliSeconds(500)));
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    cout << "ok" << endl;

    cout << "testing udp multicast... " << flush;
    string host;
    if(communicator->getProperties()->getProperty("Ice.IPv6") == "1")
    {
        host = "\"ff01::1:1\"";
    }
    else
    {
        host = "239.255.1.1";
    }
    base = communicator->stringToProxy("test:udp -h " + host + " -p 12020")->ice_datagram();
    obj = TestIntfPrx::uncheckedCast(base);

    replyI->reset();
    obj->ping(reply);
    if(!replyI->waitReply(5, IceUtil::Time::seconds(2)))
    {
        cout << "failed (is a firewall enabled?)" << endl;
        return obj;
    }

    replyI->reset();
    obj->ping(reply);
    if(!replyI->waitReply(5, IceUtil::Time::seconds(2)))
    {
        cout << "failed (is a firewall enabled?)" << endl;
        return obj;
    }

    cout << "ok" << endl;

    return obj;
}
