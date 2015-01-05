// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <Test.pb.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class AMI_MyClass_opMessageI : public Test::AMI_MyClass_opMessage, public CallbackBase
{
public:

    virtual void ice_response(const test::Message& r, const test::Message& o)
    {
        test(o.i() == 99);
        test(r.i() == 99);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }
};
typedef IceUtil::Handle<AMI_MyClass_opMessageI> AMI_MyClass_opMessageIPtr;

class AMI_MyClass_opMessageAMDI : public Test::AMI_MyClass_opMessageAMD, public CallbackBase
{
public:

    virtual void ice_response(const test::Message& r, const test::Message& o)
    {
        test(o.i() == 99);
        test(r.i() == 99);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }
};
typedef IceUtil::Handle<AMI_MyClass_opMessageAMDI> AMI_MyClass_opMessageAMDIPtr;


MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "test:default -p 12010";
    Ice::ObjectPrx baseProxy = communicator->stringToProxy(ref);
    MyClassPrx cl = MyClassPrx::checkedCast(baseProxy);
    test(cl);

    cout << "testing twoway operations... " << flush;
    {
        test::Message i;
        i.set_i(99);
        test::Message o;

        test::Message r = cl->opMessage(i, o);

        test(o.i() == 99);
        test(r.i() == 99);
    }
    {
        test::Message i;
        i.set_i(99);
        test::Message o;

        test::Message r = cl->opMessageAMD(i, o);

        test(o.i() == 99);
        test(r.i() == 99);
    }
    cout << "ok" << endl;

    cout << "testing twoway AMI operations... " << flush;
    {
        test::Message i;
        i.set_i(99);

        AMI_MyClass_opMessageIPtr cb = new AMI_MyClass_opMessageI();
        cl->opMessage_async(cb, i);
        cb->check();
    }
    {
        test::Message i;
        i.set_i(99);

        AMI_MyClass_opMessageAMDIPtr cb = new AMI_MyClass_opMessageAMDI();
        cl->opMessageAMD_async(cb, i);
        cb->check();
    }
    cout << "ok" << endl;

    return cl;
}
