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
#include <StateChanger.h>

using namespace std;

namespace
{

struct Cookie : public Ice::LocalObject
{
    Cookie(int i) : val(i)
    {
    }
    int val;
};

typedef IceUtil::Handle<Cookie> CookiePtr;

class CallbackBase : public Ice::LocalObject
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }
    
protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

typedef IceUtil::Handle<CallbackBase> CallbackBasePtr;

class NoEndpointCallback : public CallbackBase
{
public:

    NoEndpointCallback()
    {
    }

    void completed(const Ice::AsyncResultPtr& r)
    {
        try
	{
	    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(r->getProxy());
	    p->end_opVoid(r);
            called();
	}
	catch(const Ice::NoEndpointException&)
	{
	    test(false);
	}
    }

    void completedEx(const Ice::AsyncResultPtr& r)
    {
        try
	{
	    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(r->getProxy());
	    p->end_opVoid(r);
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	    called();
	}
    }

    void successNC()
    {
        test(false);
    }

    void success(const CookiePtr&)
    {
        test(false);
    }

    void exCBNC(const Ice::Exception& ex)
    {
	test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
	called();
    }

    void exCB(const Ice::Exception& ex, const CookiePtr&)
    {
	test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
	called();
    }

    void sentCB(const Ice::AsyncResultPtr&)
    {
	test(false);
    }

    void sentCBNC()
    {
	test(false);
    }
};

typedef IceUtil::Handle<NoEndpointCallback> NoEndpointCallbackPtr;

class Callback : public CallbackBase
{
public:

    Callback()
        : _communicator(0)
    {
    }

    Callback(const Ice::CommunicatorPtr& communicator)
        : _communicator(communicator)
    {
    }

    Callback(int l)
        : _l(l)
    {
    }

    Callback(const vector<string>& ids)
        : _ids(ids)
    {
    }

    Callback(const string& id)
        : _id(id)
    {
    }

    void opVoid(const Ice::AsyncResultPtr& result)
    {
	CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
	if(cookie)
	{
	    test(cookie->val == 99);
	}
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	p->end_opVoid(result);
        called();
    }

    void opVoidNC()
    {
        called();
    }

    void opVoidWC(const CookiePtr& cookie)
    {
	test(cookie->val == 99);
        called();
    }

    void opContextNC(const Ice::Context&)
    {
        called();
    }

    void opContext(const Ice::Context&, const CookiePtr&)
    {
        called();
    }

    void opByteAsync(const Ice::AsyncResultPtr& result)
    {
	CookiePtr c = CookiePtr::dynamicCast(result->getCookie());
	test(c->val == 78);
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	Ice::Byte b;
	Ice::Byte r = p->end_opByte(b, result);
        test(b == Ice::Byte(0xf0));
        test(r == Ice::Byte(0xff));
        called();
    }

    void opByteNC(Ice::Byte r, Ice::Byte b)
    {
        test(b == Ice::Byte(0xf0));
        test(r == Ice::Byte(0xff));
        called();
    }

    void opByte(Ice::Byte r, Ice::Byte b, const CookiePtr& cookie)
    {
	test(cookie->val == 78);
        test(b == Ice::Byte(0xf0));
        test(r == Ice::Byte(0xff));
        called();
    }

    void opBool(bool r, bool b)
    {
        test(b);
        test(!r);
        called();
    }

    void ice_isAAsync(const Ice::AsyncResultPtr& result)
    {
	CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
	if(cookie)
	{
	    test(cookie->val == 99);
	}
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	test(p->end_ice_isA(result));
        called();
    }

    void ice_isANC(bool r)
    {
	test(r);
        called();
    }

    void ice_isA(bool r, const CookiePtr&)
    {
	test(r);
        called();
    }

    void ice_pingAsync(const Ice::AsyncResultPtr& result)
    {
	CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
	if(cookie)
	{
	    test(cookie->val == 99);
	}
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	p->end_ice_ping(result);
        called();
    }

    void ice_pingNC()
    {
        called();
    }

    void ice_ping(const CookiePtr&)
    {
        called();
    }

    void ice_idsAsync(const Ice::AsyncResultPtr& result)
    {
	CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
	if(cookie)
	{
	    test(cookie->val == 99);
	}
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	test(p->end_ice_ids(result) == p->ice_ids());
        called();
    }

    void ice_idsNC(const vector<string>& ids)
    {
	test(ids == _ids);
        called();
    }

    void ice_ids(const vector<string>& ids, const CookiePtr&)
    {
	test(ids == _ids);
        called();
    }

    void ice_idAsync(const Ice::AsyncResultPtr& result)
    {
	CookiePtr cookie = CookiePtr::dynamicCast(result->getCookie());
	if(cookie)
	{
	    test(cookie->val == 99);
	}
	Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(result->getProxy());
	test(p->end_ice_id(result) == p->ice_id());
        called();
    }

    void ice_idNC(const string& id)
    {
	test(id == _id);
        called();
    }

    void ice_id(const string& id, const CookiePtr&)
    {
	test(id == _id);
        called();
    }

    void opShortIntLong(Ice::Long r, Ice::Short s, Ice::Int i, Ice::Long l)
    {
        test(s == 10);
        test(i == 11);
        test(l == 12);
        test(r == 12);
        called();
    }

    void opFloatDouble(Ice::Double r, Ice::Float f, Ice::Double d)
    {
        test(f == Ice::Float(3.14));
        test(d == Ice::Double(1.1E10));
        test(r == Ice::Double(1.1E10));
        called();
    }

    void opString(const ::std::string& r, const ::std::string& s)
    {
        test(s == "world hello");
        test(r == "hello world");
        called();
    }

    void opMyEnum(Test::MyEnum r, Test::MyEnum e)
    {
        test(e == Test::enum2);
        test(r == Test::enum3);
        called();
    }

    void opMyClass(const Test::MyClassPrx& r, const Test::MyClassPrx& c1, const Test::MyClassPrx& c2)
    {
        test(c1->ice_getIdentity() == _communicator->stringToIdentity("test"));
        test(c2->ice_getIdentity() == _communicator->stringToIdentity("noSuchIdentity"));
        test(r->ice_getIdentity() == _communicator->stringToIdentity("test"));

	//
        // We can't do the callbacks below in connection serialization mode.
	//
        if(_communicator->getProperties()->getPropertyAsInt("Ice.ThreadPool.Client.Serialize"))
        {
            r->opVoid();
            c1->opVoid();
            try
            {
                c2->opVoid();
                test(false);
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }
        }
        called();
    }

    void opStruct(const Test::Structure& rso, const Test::Structure& so)
    {
        test(rso.p == 0);
        test(rso.e == Test::enum2);
        test(rso.s.s == "def");
        test(so.e == Test::enum3);
        test(so.s.s == "a new string");

	//
        // We can't do the callbacks below in connection serialization mode.
	//
        if(_communicator->getProperties()->getPropertyAsInt("Ice.ThreadPool.Client.Serialize"))
        {
            so.p->opVoid();
        }
        called();
    }

    void opByteS(const Test::ByteS& rso, const Test::ByteS& bso)
    {
        test(bso.size() == 4);
        test(bso[0] == Ice::Byte(0x22));
        test(bso[1] == Ice::Byte(0x12));
        test(bso[2] == Ice::Byte(0x11));
        test(bso[3] == Ice::Byte(0x01));
        test(rso.size() == 8);
        test(rso[0] == Ice::Byte(0x01));
        test(rso[1] == Ice::Byte(0x11));
        test(rso[2] == Ice::Byte(0x12));
        test(rso[3] == Ice::Byte(0x22));
        test(rso[4] == Ice::Byte(0xf1));
        test(rso[5] == Ice::Byte(0xf2));
        test(rso[6] == Ice::Byte(0xf3));
        test(rso[7] == Ice::Byte(0xf4));
        called();
    }

    void opBoolS(const Test::BoolS& rso, const Test::BoolS& bso)
    {
        test(bso.size() == 4);
        test(bso[0]);
        test(bso[1]);
        test(!bso[2]);
        test(!bso[3]);
        test(rso.size() == 3);
        test(!rso[0]);
        test(rso[1]);
        test(rso[2]);
        called();
    }

    void opShortIntLongS(const Test::LongS& rso, const Test::ShortS& sso, const Test::IntS& iso, const Test::LongS& lso)
    {
        test(sso.size() == 3);
        test(sso[0] == 1);
        test(sso[1] == 2);
        test(sso[2] == 3);
        test(iso.size() == 4);
        test(iso[0] == 8);
        test(iso[1] == 7);
        test(iso[2] == 6);
        test(iso[3] == 5);
        test(lso.size() == 6);
        test(lso[0] == 10);
        test(lso[1] == 30);
        test(lso[2] == 20);
        test(lso[3] == 10);
        test(lso[4] == 30);
        test(lso[5] == 20);
        test(rso.size() == 3);
        test(rso[0] == 10);
        test(rso[1] == 30);
        test(rso[2] == 20);
        called();
    }

    void opFloatDoubleS(const Test::DoubleS& rso, const Test::FloatS& fso, const Test::DoubleS& dso)
    {
        test(fso.size() == 2);
        test(fso[0] == ::Ice::Float(3.14));
        test(fso[1] == ::Ice::Float(1.11));
        test(dso.size() == 3);
        test(dso[0] == ::Ice::Double(1.3E10));
        test(dso[1] == ::Ice::Double(1.2E10));
        test(dso[2] == ::Ice::Double(1.1E10));
        test(rso.size() == 5);
        test(rso[0] == ::Ice::Double(1.1E10));
        test(rso[1] == ::Ice::Double(1.2E10));
        test(rso[2] == ::Ice::Double(1.3E10));
        test(::Ice::Float(rso[3]) == ::Ice::Float(3.14));
        test(::Ice::Float(rso[4]) == ::Ice::Float(1.11));
        called();
    }

    void opStringS(const Test::StringS& rso, const Test::StringS& sso)
    {
        test(sso.size() == 4);
        test(sso[0] == "abc");
        test(sso[1] == "de");
        test(sso[2] == "fghi");
        test(sso[3] == "xyz");
        test(rso.size() == 3);
        test(rso[0] == "fghi");
        test(rso[1] == "de");
        test(rso[2] == "abc");
        called();
    }

    void opByteSS(const Test::ByteSS& rso, const Test::ByteSS& bso)
    {
        test(bso.size() == 2);
        test(bso[0].size() == 1);
        test(bso[0][0] == Ice::Byte(0xff));
        test(bso[1].size() == 3);
        test(bso[1][0] == Ice::Byte(0x01));
        test(bso[1][1] == Ice::Byte(0x11));
        test(bso[1][2] == Ice::Byte(0x12));
        test(rso.size() == 4);
        test(rso[0].size() == 3);
        test(rso[0][0] == Ice::Byte(0x01));
        test(rso[0][1] == Ice::Byte(0x11));
        test(rso[0][2] == Ice::Byte(0x12));
        test(rso[1].size() == 1);
        test(rso[1][0] == Ice::Byte(0xff));
        test(rso[2].size() == 1);
        test(rso[2][0] == Ice::Byte(0x0e));
        test(rso[3].size() == 2);
        test(rso[3][0] == Ice::Byte(0xf2));
        test(rso[3][1] == Ice::Byte(0xf1));
        called();
    }

    void opFloatDoubleSS(const Test::DoubleSS& rso, const Test::FloatSS& fso, const Test::DoubleSS& dso)
    {
        test(fso.size() == 3);
        test(fso[0].size() == 1);
        test(fso[0][0] == ::Ice::Float(3.14));
        test(fso[1].size() == 1);
        test(fso[1][0] == ::Ice::Float(1.11));
        test(fso[2].size() == 0);
        test(dso.size() == 1);
        test(dso[0].size() == 3);
        test(dso[0][0] == ::Ice::Double(1.1E10));
        test(dso[0][1] == ::Ice::Double(1.2E10));
        test(dso[0][2] == ::Ice::Double(1.3E10));
        test(rso.size() == 2);
        test(rso[0].size() == 3);
        test(rso[0][0] == ::Ice::Double(1.1E10));
        test(rso[0][1] == ::Ice::Double(1.2E10));
        test(rso[0][2] == ::Ice::Double(1.3E10));
        test(rso[1].size() == 3);
        test(rso[1][0] == ::Ice::Double(1.1E10));
        test(rso[1][1] == ::Ice::Double(1.2E10));
        test(rso[1][2] == ::Ice::Double(1.3E10));
        called();
    }

    void opStringSS(const Test::StringSS& rso, const Test::StringSS& sso)
    {
        test(sso.size() == 5);
        test(sso[0].size() == 1);
        test(sso[0][0] == "abc");
        test(sso[1].size() == 2);
        test(sso[1][0] == "de");
        test(sso[1][1] == "fghi");
        test(sso[2].size() == 0);
        test(sso[3].size() == 0);
        test(sso[4].size() == 1);
        test(sso[4][0] == "xyz");
        test(rso.size() == 3);
        test(rso[0].size() == 1);
        test(rso[0][0] == "xyz");
        test(rso[1].size() == 0);
        test(rso[2].size() == 0);
        called();
    }

    void opByteBoolD(const Test::ByteBoolD& ro, const Test::ByteBoolD& _do)
    {
        Test::ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find(10) != ro.end());
        test(ro.find(10)->second == true);
        test(ro.find(11) != ro.end());
        test(ro.find(11)->second == false);
        test(ro.find(100) != ro.end());
        test(ro.find(100)->second == false);
        test(ro.find(101) != ro.end());
        test(ro.find(101)->second == true);
        called();
    }

    void opShortIntD(const Test::ShortIntD& ro, const Test::ShortIntD& _do)
    {
        Test::ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find(110) != ro.end());
        test(ro.find(110)->second == -1);
        test(ro.find(111) != ro.end());
        test(ro.find(111)->second == -100);
        test(ro.find(1100) != ro.end());
        test(ro.find(1100)->second == 123123);
        test(ro.find(1101) != ro.end());
        test(ro.find(1101)->second == 0);
        called();
    }

    void opLongFloatD(const Test::LongFloatD& ro, const Test::LongFloatD& _do)
    {
        Test::LongFloatD di1;
        di1[999999110] = Ice::Float(-1.1);
        di1[999999111] = Ice::Float(123123.2);
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find(999999110) != ro.end());
        test(ro.find(999999110)->second == Ice::Float(-1.1));
        test(ro.find(999999120) != ro.end());
        test(ro.find(999999120)->second == Ice::Float(-100.4));
        test(ro.find(999999111) != ro.end());
        test(ro.find(999999111)->second == Ice::Float(123123.2));
        test(ro.find(999999130) != ro.end());
        test(ro.find(999999130)->second == Ice::Float(0.5));
        called();
    }

    void opStringStringD(const Test::StringStringD& ro, const Test::StringStringD& _do)
    {
        Test::StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find("foo") != ro.end());
        test(ro.find("foo")->second == "abc -1.1");
        test(ro.find("FOO") != ro.end());
        test(ro.find("FOO")->second == "abc -100.4");
        test(ro.find("bar") != ro.end());
        test(ro.find("bar")->second == "abc 123123.2");
        test(ro.find("BAR") != ro.end());
        test(ro.find("BAR")->second == "abc 0.5");
        called();
    }

    void opStringMyEnumD(const Test::StringMyEnumD& ro, const Test::StringMyEnumD& _do)
    {
        Test::StringMyEnumD di1;
        di1["abc"] = Test::enum1;
        di1[""] = Test::enum2;
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find("abc") != ro.end());
        test(ro.find("abc")->second == Test::enum1);
        test(ro.find("qwerty") != ro.end());
        test(ro.find("qwerty")->second == Test::enum3);
        test(ro.find("") != ro.end());
        test(ro.find("")->second == Test::enum2);
        test(ro.find("Hello!!") != ro.end());
        test(ro.find("Hello!!")->second == Test::enum2);
        called();
    }

    void opMyStructMyEnumD(const Test::MyStructMyEnumD& ro, const Test::MyStructMyEnumD& _do)
    {
        Test::MyStruct s11 = { 1, 1 };
        Test::MyStruct s12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[s11] = Test::enum1;
        di1[s12] = Test::enum2;
        test(_do == di1);
        Test::MyStruct s22 = { 2, 2 };
        Test::MyStruct s23 = { 2, 3 };
        test(ro.size() == 4);
        test(ro.find(s11) != ro.end());
        test(ro.find(s11)->second == Test::enum1);
        test(ro.find(s12) != ro.end());
        test(ro.find(s12)->second == Test::enum2);
        test(ro.find(s22) != ro.end());
        test(ro.find(s22)->second == Test::enum3);
        test(ro.find(s23) != ro.end());
        test(ro.find(s23)->second == Test::enum2);
        called();
    }

    void opIntS(const Test::IntS& r)
    {
        test(r.size() == static_cast<size_t>(_l));
        for(int j = 0; j < _l; ++j)
        {
            test(r[j] == -j);
        }
        called();
    }

    void opDoubleMarshaling()
    {
        called();
    }

    void opDerived()
    {
        called();
    }

    void exCBNC(const Ice::Exception& ex)
    {
	test(false);
    }

    void exCB(const Ice::Exception& ex, const CookiePtr& cookie)
    {
	test(false);
    }

private:

    Ice::CommunicatorPtr _communicator;
    int _l;
    vector<string> _ids;
    string _id;
};

typedef IceUtil::Handle<Callback> CallbackPtr;

enum ThrowType { LocalException, UserException, StandardException, OtherException };

class Thrower : public CallbackBase
{
public:

    Thrower(ThrowType t)
        : _t(t)
    {
    }

    void opVoidNC()
    {
        // No call to called() here!
    }

    void opVoidThrow(const Ice::AsyncResultPtr& r)
    {
	called();
	throwEx();
    }

    void opVoidThrowNC()
    {
	called();
	throwEx();
    }

    void exCBNC(const Ice::Exception& ex)
    {
	test(false);
    }

    void exCBThrowNC(const Ice::Exception& ex)
    {
	called();
	throwEx();
    }

private:

    void throwEx()
    {
	switch(_t)
	{
	    case LocalException:
	    {
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
		break;
	    }
	    case UserException:
	    {
		throw Test::SomeException();
		break;
	    }
	    case StandardException:
	    {
		throw ::std::bad_alloc();
		break;
	    }
	    case OtherException:
	    {
	    	throw 99;
		break;
	    }
	    default:
	    {
	        assert(false);
		break;
	    }
	}
    }

    ThrowType _t;
};

typedef IceUtil::Handle<Thrower> ThrowerPtr;

class SentCounter : public CallbackBase
{
public:

    SentCounter() : _queuedCount(0), _completedCount(0)
    {
    }

    void opVoid(const Ice::AsyncResultPtr& r)
    {
        test(r->isCompleted());
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        ++_completedCount;
	_m.notify();
    }

    void sentCB(const Ice::AsyncResultPtr& r)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        ++_queuedCount;
    }

    int queuedCount()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        return _queuedCount;
    }

    void check(int size)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(_completedCount != size)
	{
	     _m.wait();
	}
    }

private:

    int _queuedCount;
    int _completedCount;
    IceUtil::ThreadControl::ID _id;
    IceUtil::Monitor<IceUtil::Mutex> _m;
};

typedef IceUtil::Handle<SentCounter> SentCounterPtr;

}

void
twowaysNewAMI(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrx& p)
{
    {
	//
        // Check that a call to a void operation raises NoEndpointException
        // in the end_ method instead of at the point of call.
	//
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy"));
	Ice::AsyncResultPtr r;

	r = indirect->begin_opVoid();
	try
	{
	    indirect->end_opVoid(r);
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}

	//
	// Check that a second call to the end_ method throws IllegalArgumentException.
	//
	try
	{
	    indirect->end_opVoid(r);
	    test(false);
	}
	catch(const IceUtil::IllegalArgumentException&)
	{
	}

	//
        // Use type-unsafe and type-safe variations of the callback, and
	// variations of the callback (with and without success callback).
	// Also test that the sent callback is not called in this case.
	//
        NoEndpointCallbackPtr cb1 = new NoEndpointCallback;
	NoEndpointCallbackPtr cb2 = new NoEndpointCallback;
	NoEndpointCallbackPtr cb3 = new NoEndpointCallback;

        Ice::CallbackPtr callback = Ice::newCallback(cb1, 
                                                     &NoEndpointCallback::completedEx,
                                                     &NoEndpointCallback::sentCB);
        indirect->begin_opVoid(callback);
        
        Test::Callback_MyClass_opVoidPtr callback2 = Test::newCallback_MyClass_opVoid(cb2,
                                                                                      &NoEndpointCallback::successNC,
                                                                                      &NoEndpointCallback::exCBNC,
                                                                                      &NoEndpointCallback::sentCBNC);
        indirect->begin_opVoid(callback2);
        
        Ice::CallbackPtr callback3 = Ice::newCallback(cb3, 
                                                      &NoEndpointCallback::exCBNC, 
                                                      &NoEndpointCallback::sentCBNC);
        indirect->begin_opVoid(callback3);
        
	cb1->check();
	cb2->check();
	cb3->check();
    }
    
    {
        //
	// Check that calling the end_ method with a different proxy or for a different operation than the begin_
	// method throws IllegalArgumentException. If the test throws as expected, we never call the end_ method,
	// so this also tests that it is safe to throw the AsyncResult away without calling the end_ method.
	//
        Test::MyClassPrx indirect1 = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy"));
        Test::MyClassPrx indirect2 = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy2"));

	Ice::AsyncResultPtr r1 = indirect1->begin_opVoid();
	Ice::AsyncResultPtr r2 = indirect2->begin_opVoid();

	try
	{
	    indirect1->end_opVoid(r2); // Wrong proxy
	    test(false);
	}
	catch(const IceUtil::IllegalArgumentException&)
	{
	}

	try
	{
	    indirect1->end_shutdown(r1); // Wrong operation
	    test(false);
	}
	catch(const IceUtil::IllegalArgumentException&)
	{
	}
    }

    {
        //
	// Check that calling the end_ method with a null result throws IllegalArgumentException.
	//
	try
	{
	    p->end_opVoid(0);
	    test(false);
	}
	catch(const IceUtil::IllegalArgumentException&)
	{
	}
    }

    {
	//
	// Check that passing a null callback instance throws IllegalArgumentException.
	//
	CallbackPtr cb;
	try
	{
            Ice::newCallback(cb, &Callback::exCBNC);
	    test(false);
	}
	catch(IceUtil::IllegalArgumentException&)
	{
	}
    }

    //
    // Check optional callbacks on callback creation.
    //
    {
        NoEndpointCallbackPtr cb = new NoEndpointCallback;

        Ice::CallbackPtr del1 = Ice::newCallback(cb, &NoEndpointCallback::completed);
        p->begin_opVoid(del1);
        cb->check();

        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy"));
            
        Ice::CallbackPtr del2 = Ice::newCallback(cb, &NoEndpointCallback::exCBNC);
        indirect->begin_opVoid(del2);
        cb->check();
            
        Ice::CallbackPtr del3 = Ice::newCallback(cb, &NoEndpointCallback::exCB);
        indirect->begin_opVoid(del3, 0);
        cb->check();

        Ice::CallbackPtr del4 = Ice::newCallback(cb, &NoEndpointCallback::sentCBNC);
        indirect->begin_opVoid(del4);

        try
        {
            void (NoEndpointCallback::*nullCallback)(const Ice::AsyncResultPtr&) = 0;
            Ice::newCallback(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }        

        try
        {
            void (NoEndpointCallback::*nullCallback)(const Ice::Exception&) = 0;
            Ice::newCallback(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (NoEndpointCallback::*nullCallback)(const Ice::Exception&, const CookiePtr&) = 0;
            Ice::newCallback(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    //
    // Check optional exception and sent callbacks on delegation creation.
    //
    {
	CallbackPtr cb = new Callback;

        Test::Callback_MyClass_opVoidPtr del1 = Test::newCallback_MyClass_opVoid(cb, &Callback::opVoidNC);
        p->begin_opVoid(del1);
        cb->check();
            
        Test::Callback_MyClass_opVoidPtr del2 = Test::newCallback_MyClass_opVoid(cb, &Callback::opVoidWC);
        p->begin_opVoid(del2, new Cookie(99));
        cb->check();
            
        Test::Callback_MyClass_opContextPtr del3 = Test::newCallback_MyClass_opContext(cb, &Callback::opContextNC);
        p->begin_opContext(del3);
        cb->check();
            
        Test::Callback_MyClass_opContextPtr del4 = Test::newCallback_MyClass_opContext(cb, &Callback::opContext);
        p->begin_opContext(del4, 0);
        cb->check();

        try
        {
            void (Callback::*nullCallback)() = 0;
            Test::newCallback_MyClass_opVoid(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (Callback::*nullCallback)(const CookiePtr&) = 0;
            Test::newCallback_MyClass_opVoid(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (Callback::*nullCallback)(const Ice::Context&) = 0;
            Test::newCallback_MyClass_opContext(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }

        try
        {
            void (Callback::*nullCallback)(const Ice::Context&, const CookiePtr&) = 0;
            Test::newCallback_MyClass_opContext(cb, nullCallback);
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        //
	// Check that throwing an exception from the success callback doesn't cause problems.
	//
	{
	    ThrowerPtr cb = new Thrower(LocalException);
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::opVoidThrow);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(UserException);
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::opVoidThrow);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(StandardException);
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::opVoidThrow);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(OtherException);
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::opVoidThrow);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(LocalException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidThrowNC,
                                                                                         &Thrower::exCBNC);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(UserException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidThrowNC,
                                                                                         &Thrower::exCBNC);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(StandardException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidThrowNC,
                                                                                         &Thrower::exCBNC);
	    p->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(OtherException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidThrowNC,
                                                                                         &Thrower::exCBNC);
	    p->begin_opVoid(callback);
	    cb->check();
	}
    }

    {
        //
	// Check that throwing an exception from the exception callback doesn't cause problems.
	//
        Test::MyClassPrx indirect = Test::MyClassPrx::uncheckedCast(p->ice_adapterId("dummy"));

	{
	    ThrowerPtr cb = new Thrower(LocalException);
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Thrower::opVoidThrow);
	    indirect->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(LocalException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidNC,
                                                                                         &Thrower::exCBThrowNC);
	    indirect->begin_opVoid(callback);
	    cb->check();
	}
        
	{
	    ThrowerPtr cb = new Thrower(UserException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidNC,
                                                                                         &Thrower::exCBThrowNC);
	    indirect->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(StandardException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidNC,
                                                                                         &Thrower::exCBThrowNC);
	    indirect->begin_opVoid(callback);
	    cb->check();
	}

	{
	    ThrowerPtr cb = new Thrower(OtherException);
	    Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                         &Thrower::opVoidNC,
                                                                                         &Thrower::exCBThrowNC);
	    indirect->begin_opVoid(callback);
	    cb->check();
	}
    }

    {
	//
        // Check that CommunicatorDestroyedException is raised directly.
	//
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        Ice::CommunicatorPtr ic = Ice::initialize(initData);
        Ice::ObjectPrx obj = ic->stringToProxy(p->ice_toString());
        Test::MyClassPrx p2 = Test::MyClassPrx::checkedCast(obj);

        ic->destroy();
    
        try
        {
	    p2->begin_opVoid();
            test(false);
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // Expected.
        }
    }

    //
    // Test that marshaling works as expected, and that the callbacks for each type of callback work.
    //

    {
	Ice::AsyncResultPtr r = p->begin_opVoid();
	p->end_opVoid(r);
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                     &Callback::opVoidNC,
                                                                                     &Callback::exCBNC);
	p->begin_opVoid(callback);
	cb->check();
    }

    {
	Ice::AsyncResultPtr r = p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f));
	Ice::Byte p3;
	Ice::Byte ret = p->end_opByte(p3, r);
        test(p3 == Ice::Byte(0xf0));
        test(ret == Ice::Byte(0xff));
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opBytePtr callback = Test::newCallback_MyClass_opByte(cb,
                                                                                     &Callback::opByteNC,
                                                                                     &Callback::exCBNC);
	p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opBoolPtr callback = Test::newCallback_MyClass_opBool(cb,
                                                                                     &Callback::opBool,
                                                                                     &Callback::exCBNC);
	p->begin_opBool(true, false, callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opShortIntLongPtr callback = 
            Test::newCallback_MyClass_opShortIntLong(cb, &Callback::opShortIntLong, &Callback::exCBNC);
	p->begin_opShortIntLong(10, 11, 12, callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opFloatDoublePtr callback = 
            Test::newCallback_MyClass_opFloatDouble(cb, &Callback::opFloatDouble, &Callback::exCBNC);
	p->begin_opFloatDouble(Ice::Float(3.14), Ice::Double(1.1E10), callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opStringPtr callback = Test::newCallback_MyClass_opString(cb,
                                                                                         &Callback::opString,
                                                                                         &Callback::exCBNC);
	p->begin_opString("hello", "world", callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opMyEnumPtr callback = Test::newCallback_MyClass_opMyEnum(cb,
                                                                                         &Callback::opMyEnum,
                                                                                         &Callback::exCBNC);
	p->begin_opMyEnum(Test::enum2, callback);
	cb->check();
    }

    {
	CallbackPtr cb = new Callback(communicator);
	Test::Callback_MyClass_opMyClassPtr callback = Test::newCallback_MyClass_opMyClass(cb,
                                                                                           &Callback::opMyClass,
                                                                                           &Callback::exCBNC);
	p->begin_opMyClass(p, callback);
	cb->check();
    }

    {
        Test::Structure si1;
        si1.p = p;
        si1.e = Test::enum3;
        si1.s.s = "abc";
        Test::Structure si2;
        si2.p = 0;
        si2.e = Test::enum2;
        si2.s.s = "def";
        
	CallbackPtr cb = new Callback(communicator);
	Test::Callback_MyClass_opStructPtr callback = Test::newCallback_MyClass_opStruct(cb,
                                                                                         &Callback::opStruct,
                                                                                         &Callback::exCBNC);
	p->begin_opStruct(si1, si2, callback);
	cb->check();
    }

    {
        Test::ByteS bsi1;
        Test::ByteS bsi2;

        bsi1.push_back(Ice::Byte(0x01));
        bsi1.push_back(Ice::Byte(0x11));
        bsi1.push_back(Ice::Byte(0x12));
        bsi1.push_back(Ice::Byte(0x22));

        bsi2.push_back(Ice::Byte(0xf1));
        bsi2.push_back(Ice::Byte(0xf2));
        bsi2.push_back(Ice::Byte(0xf3));
        bsi2.push_back(Ice::Byte(0xf4));

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opByteSPtr callback = Test::newCallback_MyClass_opByteS(cb,
                                                                                       &Callback::opByteS,
                                                                                       &Callback::exCBNC);
	p->begin_opByteS(bsi1, bsi2, callback);
	cb->check();
    }

    {
        Test::BoolS bsi1;
        Test::BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opBoolSPtr callback = Test::newCallback_MyClass_opBoolS(cb,
                                                                                       &Callback::opBoolS,
                                                                                       &Callback::exCBNC);
	p->begin_opBoolS(bsi1, bsi2, callback);
	cb->check();
    }

    {
        Test::ShortS ssi;
        Test::IntS isi;
        Test::LongS lsi;

        ssi.push_back(1);
        ssi.push_back(2);
        ssi.push_back(3);

        isi.push_back(5);
        isi.push_back(6);
        isi.push_back(7);
        isi.push_back(8);

        lsi.push_back(10);
        lsi.push_back(30);
        lsi.push_back(20);

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opShortIntLongSPtr callback = 
            Test::newCallback_MyClass_opShortIntLongS(cb, &Callback::opShortIntLongS, &Callback::exCBNC);
	p->begin_opShortIntLongS(ssi, isi, lsi, callback);
	cb->check();
    }

    {
        Test::FloatS fsi;
        Test::DoubleS dsi;

        fsi.push_back(Ice::Float(3.14));
        fsi.push_back(Ice::Float(1.11));

        dsi.push_back(Ice::Double(1.1E10));
        dsi.push_back(Ice::Double(1.2E10));
        dsi.push_back(Ice::Double(1.3E10));

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opFloatDoubleSPtr callback = 
            Test::newCallback_MyClass_opFloatDoubleS(cb, &Callback::opFloatDoubleS, &Callback::exCBNC);
	p->begin_opFloatDoubleS(fsi, dsi, callback);
	cb->check();
    }

    {
        Test::StringS ssi1;
        Test::StringS ssi2;

        ssi1.push_back("abc");
        ssi1.push_back("de");
        ssi1.push_back("fghi");

        ssi2.push_back("xyz");

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opStringSPtr callback = Test::newCallback_MyClass_opStringS(cb,
                                                                                           &Callback::opStringS,
                                                                                           &Callback::exCBNC);
	p->begin_opStringS(ssi1, ssi2, callback);
	cb->check();
    }

    {
        Test::ByteSS bsi1;
        bsi1.resize(2);
        Test::ByteSS bsi2;
        bsi2.resize(2);

        bsi1[0].push_back(Ice::Byte(0x01));
        bsi1[0].push_back(Ice::Byte(0x11));
        bsi1[0].push_back(Ice::Byte(0x12));
        bsi1[1].push_back(Ice::Byte(0xff));

        bsi2[0].push_back(Ice::Byte(0x0e));
        bsi2[1].push_back(Ice::Byte(0xf2));
        bsi2[1].push_back(Ice::Byte(0xf1));

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opByteSSPtr callback = Test::newCallback_MyClass_opByteSS(cb,
                                                                                         &Callback::opByteSS,
                                                                                         &Callback::exCBNC);
	p->begin_opByteSS(bsi1, bsi2, callback);
	cb->check();
    }

    {
        Test::FloatSS fsi;
        fsi.resize(3);
        Test::DoubleSS dsi;
        dsi.resize(1);

        fsi[0].push_back(Ice::Float(3.14));
        fsi[1].push_back(Ice::Float(1.11));

        dsi[0].push_back(Ice::Double(1.1E10));
        dsi[0].push_back(Ice::Double(1.2E10));
        dsi[0].push_back(Ice::Double(1.3E10));

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opFloatDoubleSSPtr callback = 
            Test::newCallback_MyClass_opFloatDoubleSS(cb, &Callback::opFloatDoubleSS, &Callback::exCBNC);
	p->begin_opFloatDoubleSS(fsi, dsi, callback);
	cb->check();
    }

    {
        Test::StringSS ssi1;
        ssi1.resize(2);
        Test::StringSS ssi2;
        ssi2.resize(3);

        ssi1[0].push_back("abc");
        ssi1[1].push_back("de");
        ssi1[1].push_back("fghi");

        ssi2[2].push_back("xyz");

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opStringSSPtr callback = 
            Test::newCallback_MyClass_opStringSS(cb, &Callback::opStringSS, &Callback::exCBNC);
	p->begin_opStringSS(ssi1, ssi2, callback);
	cb->check();
    }

    {
        Test::ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        Test::ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opByteBoolDPtr callback = 
            Test::newCallback_MyClass_opByteBoolD(cb, &Callback::opByteBoolD, &Callback::exCBNC);
	p->begin_opByteBoolD(di1, di2, callback);
	cb->check();
    }

    {
        Test::ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        Test::ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opShortIntDPtr callback = 
            Test::newCallback_MyClass_opShortIntD(cb, &Callback::opShortIntD, &Callback::exCBNC);
	p->begin_opShortIntD(di1, di2, callback);
	cb->check();
    }

    {
        Test::LongFloatD di1;
        di1[999999110] = Ice::Float(-1.1);
        di1[999999111] = Ice::Float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = Ice::Float(-1.1);
        di2[999999120] = Ice::Float(-100.4);
        di2[999999130] = Ice::Float(0.5);

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opLongFloatDPtr callback = 
            Test::newCallback_MyClass_opLongFloatD(cb, &Callback::opLongFloatD, &Callback::exCBNC);
	p->begin_opLongFloatD(di1, di2, callback);
	cb->check();
    }

    {
        Test::StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        Test::StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opStringStringDPtr callback = 
            Test::newCallback_MyClass_opStringStringD(cb, &Callback::opStringStringD, &Callback::exCBNC);
	p->begin_opStringStringD(di1, di2, callback);
	cb->check();
    }

    {
        Test::StringMyEnumD di1;
        di1["abc"] = Test::enum1;
        di1[""] = Test::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = Test::enum1;
        di2["qwerty"] = Test::enum3;
        di2["Hello!!"] = Test::enum2;

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opStringMyEnumDPtr callback = 
            Test::newCallback_MyClass_opStringMyEnumD(cb, &Callback::opStringMyEnumD, &Callback::exCBNC);
	p->begin_opStringMyEnumD(di1, di2, callback);
	cb->check();
    }

    {
        Test::MyStruct s11 = { 1, 1 };
        Test::MyStruct s12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[s11] = Test::enum1;
        di1[s12] = Test::enum2;

        Test::MyStruct s22 = { 2, 2 };
        Test::MyStruct s23 = { 2, 3 };
        Test::MyStructMyEnumD di2;
        di2[s11] = Test::enum1;
        di2[s22] = Test::enum3;
        di2[s23] = Test::enum2;

	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opMyStructMyEnumDPtr callback = 
            Test::newCallback_MyClass_opMyStructMyEnumD(cb, &Callback::opMyStructMyEnumD, &Callback::exCBNC);
	p->begin_opMyStructMyEnumD(di1, di2, callback);
	cb->check();
    }

    {
        const int lengths[] = { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

        for(unsigned int l = 0; l != sizeof(lengths) / sizeof(*lengths); ++l)
        {
            Test::IntS s;
            for(int i = 0; i < lengths[l]; ++i)
            {
                s.push_back(i);
            }
	    CallbackPtr cb = new Callback(lengths[l]);
	    Test::Callback_MyClass_opIntSPtr callback = 
                Test::newCallback_MyClass_opIntS(cb, &Callback::opIntS, &Callback::exCBNC);
	    p->begin_opIntS(s, callback);
	    cb->check();
        }
    }

    {
        Test::StringStringD ctx;
        ctx["one"] = "ONE";
        ctx["two"] = "TWO";
        ctx["three"] = "THREE";
        {
            test(p->ice_getContext().empty());
	    Ice::AsyncResultPtr r = p->begin_opContext();
	    Ice::Context c = p->end_opContext(r);
	    test(c != ctx);
        }
        {
            test(p->ice_getContext().empty());
	    Ice::AsyncResultPtr r = p->begin_opContext(ctx);
	    Ice::Context c = p->end_opContext(r);
	    test(c == ctx);
        }
        Test::MyClassPrx p2 = Test::MyClassPrx::checkedCast(p->ice_context(ctx));
        test(p2->ice_getContext() == ctx);
        {
	    Ice::AsyncResultPtr r = p2->begin_opContext();
	    Ice::Context c = p2->end_opContext(r);
	    test(c == ctx);
        }
        {
            Test::MyClassPrx p2 = Test::MyClassPrx::checkedCast(p->ice_context(ctx));
	    Ice::AsyncResultPtr r = p2->begin_opContext(ctx);
	    Ice::Context c = p2->end_opContext(r);
	    test(c == ctx);
        }

        {
            //
            // Test implicit context propagation
            //
            
            string impls[] = {"Shared", "PerThread"};
            for(int i = 0; i < 2; i++)
            {
                Ice::InitializationData initData;
                initData.properties = communicator->getProperties()->clone();
                initData.properties->setProperty("Ice.ImplicitContext", impls[i]);
                
                Ice::CommunicatorPtr ic = Ice::initialize(initData);

                Ice::Context ctx;
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";


                Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(
                                        ic->stringToProxy("test:default -p 12010"));
                
                
                ic->getImplicitContext()->setContext(ctx);
                test(ic->getImplicitContext()->getContext() == ctx);
                {
                    Ice::AsyncResultPtr r = p->begin_opContext();
		    Ice::Context c = p->end_opContext(r);
		    test(c == ctx);
                }

                ic->getImplicitContext()->put("zero", "ZERO");
          
                ctx = ic->getImplicitContext()->getContext();
                {
                    Ice::AsyncResultPtr r = p->begin_opContext();
		    Ice::Context c = p->end_opContext(r);
		    test(c == ctx);
                }
                
                Ice::Context prxContext;
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";
                
                Ice::Context combined = prxContext;
                combined.insert(ctx.begin(), ctx.end());
                test(combined["one"] == "UN");
                
                p = Test::MyClassPrx::uncheckedCast(p->ice_context(prxContext));
                
                ic->getImplicitContext()->setContext(Ice::Context());
                {
                    Ice::AsyncResultPtr r = p->begin_opContext();
		    Ice::Context c = p->end_opContext(r);
		    test(c == prxContext);
                }

                ic->getImplicitContext()->setContext(ctx);
                {
                    Ice::AsyncResultPtr r = p->begin_opContext();
		    Ice::Context c = p->end_opContext(r);
		    test(c == combined);
                }

                ic->getImplicitContext()->setContext(Ice::Context());
                ic->destroy();
            }
        }
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
	CallbackPtr cb = new Callback;
	Test::Callback_MyClass_opDoubleMarshalingPtr callback = 
            Test::newCallback_MyClass_opDoubleMarshaling(cb, &Callback::opDoubleMarshaling, &Callback::exCBNC);
	p->begin_opDoubleMarshaling(d, ds, callback);
	cb->check();
    }

    {
        Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(p);
        test(derived);
	CallbackPtr cb = new Callback;
	Test::Callback_MyDerivedClass_opDerivedPtr callback = 
            Test::newCallback_MyDerivedClass_opDerived(cb, &Callback::opDerived, &Callback::exCBNC);
	derived->begin_opDerived(callback);
	cb->check();
    }

    //
    // Test that cookies work. Because the same generated template is
    // instantiated each time, it is not necessary to test all
    // possible operations. Instead, we only need to test once for
    // each of the three begin_ methods that accept a cookie for a
    // oneway and a twoway operation.
    //

    {
	CallbackPtr cb = new Callback;
	CookiePtr cookie = new Cookie(99);
	Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::opVoid);
	p->begin_opVoid(callback, cookie);
    }

    {
	CallbackPtr cb = new Callback;
	CookiePtr cookie = new Cookie(99);
	Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                     &Callback::opVoidWC,
                                                                                     &Callback::exCB);
	p->begin_opVoid(callback, cookie);
    }

    {
        //
 	// Test that not passing a cookie when one is expected throws IlllegalArgumentException.
 	// (The opposite, passing a cookie when none is expected, causes a compile-time error.)
 	//
// 	CallbackPtr cb = new Callback;
// 	Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
//                                                                                      &Callback::opVoidWC,
//                                                                                      &Callback::exCB);
// 	try
// 	{
// 	    p->begin_opVoid(callback);
// 	    test(false);
// 	}
// 	catch(const IceUtil::IllegalArgumentException&)
// 	{
// 	}
    }

    {
	CallbackPtr cb = new Callback;
	CookiePtr cookie = new Cookie(78);
	Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::opByteAsync);
	p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), callback, cookie);
    }

    {
	CallbackPtr cb = new Callback;
	CookiePtr cookie = new Cookie(78);
	Test::Callback_MyClass_opBytePtr callback = Test::newCallback_MyClass_opByte(cb,
                                                                                     &Callback::opByte,
                                                                                     &Callback::exCB);
	p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), callback, cookie);
    }

    {
	//
        // Check that we can call operations on Object asynchronously.
	//
	{
	    Ice::AsyncResultPtr r = p->begin_ice_isA(p->ice_staticId());
	    try
	    {
		test(p->end_ice_isA(r));
	    }
	    catch(...)
	    {
		test(false);
	    }
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_isAAsync);
	    p->begin_ice_isA(p->ice_staticId(), callback);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::Callback_Object_ice_isAPtr callback = Ice::newCallback_Object_ice_isA(cb,
                                                                                       &Callback::ice_isANC,
                                                                                       &Callback::exCBNC);
	    p->begin_ice_isA(p->ice_staticId(), callback);
	    cb->check();
	}

	{
	    Ice::AsyncResultPtr r = p->begin_ice_ping();
	    try
	    {
		p->end_ice_ping(r);
	    }
	    catch(...)
	    {
		test(false);
	    }
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_pingAsync);
	    p->begin_ice_ping(callback);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::Callback_Object_ice_pingPtr callback = Ice::newCallback_Object_ice_ping(cb,
                                                                                         &Callback::ice_pingNC,
                                                                                         &Callback::exCBNC);
	    p->begin_ice_ping(callback);
	    cb->check();
	}

	{
	    Ice::AsyncResultPtr r = p->begin_ice_ids();
	    try
	    {
		test(p->end_ice_ids(r) == p->ice_ids());
	    }
	    catch(...)
	    {
		test(false);
	    }
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_idsAsync);
	    p->begin_ice_ids(callback);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback(p->ice_ids());
	    Ice::Callback_Object_ice_idsPtr callback = Ice::newCallback_Object_ice_ids(cb,
                                                                                       &Callback::ice_idsNC,
                                                                                       &Callback::exCBNC);
	    p->begin_ice_ids(callback);
	    cb->check();
	}

	{
	    Ice::AsyncResultPtr r = p->begin_ice_id();
	    try
	    {
		test(p->end_ice_id(r) == p->ice_id());
	    }
	    catch(...)
	    {
		test(false);
	    }
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_idAsync);
	    p->begin_ice_id(callback);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback(p->ice_id());
	    Ice::Callback_Object_ice_idPtr callback = Ice::newCallback_Object_ice_id(cb,
                                                                                     &Callback::ice_idNC,
                                                                                     &Callback::exCBNC);
	    p->begin_ice_id(callback);
	    cb->check();
	}

	//
        // Check that we can call operations on Object asynchronously with a cookie.
	//
	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_isAAsync);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_isA(p->ice_staticId(), callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::Callback_Object_ice_isAPtr callback = Ice::newCallback_Object_ice_isA(cb,
                                                                                       &Callback::ice_isA,
                                                                                       &Callback::exCB);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_isA(p->ice_staticId(), callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_pingAsync);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_ping(callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::Callback_Object_ice_pingPtr callback = Ice::newCallback_Object_ice_ping(cb,
                                                                                         &Callback::ice_ping,
                                                                                         &Callback::exCB);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_ping(callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_idsAsync);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_ids(callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback(p->ice_ids());
	    Ice::Callback_Object_ice_idsPtr callback = Ice::newCallback_Object_ice_ids(cb,
                                                                                       &Callback::ice_ids,
                                                                                       &Callback::exCB);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_ids(callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback;
	    Ice::CallbackPtr callback = Ice::newCallback(cb, &Callback::ice_idAsync);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_id(callback, cookie);
	    cb->check();
	}

	{
	    CallbackPtr cb = new Callback(p->ice_id());
	    Ice::Callback_Object_ice_idPtr callback = Ice::newCallback_Object_ice_id(cb,
                                                                                     &Callback::ice_id,
                                                                                     &Callback::exCB);
	    CookiePtr cookie = new Cookie(99);
	    p->begin_ice_id(callback, cookie);
	    cb->check();
	}
    }

    //
    // Test that queuing indication works.
    //
    {
	Ice::AsyncResultPtr r = p->begin_delay(100);
	test(!r->isCompleted());
	p->end_delay(r);
	test(r->isCompleted());
    }

    //
    // Put the server's adapter into the holding state and pump out requests until one is queued.
    // Then activate the adapter again and pump out more until one isn't queued again.
    // Check that all the callbacks arrive after calling the end_ method for each request.
    // We fill a context with a few kB of data to make sure we don't queue up too many requests.
    //
    {
	const int contextSize = 10; // Kilobytes
	string s(1024, 'a');

	Ice::Context ctx;
	for(int i = 0; i < contextSize; ++i)
	{
	    ostringstream ss;
	    ss << "i" << i;
	    ctx[ss.str()] = s;
	}

	//
	// Keep all the AsyncResults we get from the begin_ calls, so we can call end_ for each of them.
	//
	vector<Ice::AsyncResultPtr> results;

	int queuedCount = 0;

	SentCounterPtr cb = new SentCounter;
	Ice::CallbackPtr callback = Ice::newCallback(cb, &SentCounter::opVoid, &SentCounter::sentCB);
	Ice::AsyncResultPtr r;

	Test::StateChangerPrx state = Test::StateChangerPrx::checkedCast(
					communicator->stringToProxy("hold:default -p 12011"));
	state->hold(3);

	do
	{
	    r = p->begin_opVoid(ctx, callback);
	    results.push_back(r);
	    if(!r->sentSynchronously())
	    {
		++queuedCount;
	    }
	}
	while(r->sentSynchronously());

	vector<Ice::AsyncResultPtr>::size_type numRequests = results.size();
	test(numRequests > 1); // Something is wrong if we didn't get something out without queueing.

	//
	// Re-enable the adapter.
	//
	state->activate(3);

	//
	// Fire off a bunch more requests until we get one that wasn't queued.
	// We sleep in between calls to allow the queued requests to drain.
	//
	do
	{
	    r = p->begin_opVoid(callback);
	    results.push_back(r);
	    if(!r->sentSynchronously())
	    {
		++queuedCount;
	    }
	    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
	}
	while(!r->sentSynchronously());
	test(results.size() > numRequests); // Something is wrong if we didn't queue additional requests.

	//
	// Now make all the outstanding calls to the end_ method.
	//
	for(vector<Ice::AsyncResultPtr>::iterator q = results.begin(); q != results.end(); ++q)
	{
	    p->end_opVoid(*q);
	}

	//
	// Check that all the callbacks have arrived and that we got a sent callback for each queued request.
	//
	cb->check(static_cast<int>(results.size()));
	test(cb->queuedCount() == queuedCount);
    }
}
