// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

//
// Work-around for GCC warning bug
//
#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif


//
// Disable VC++ warning
// 4503: decorated name length exceeded, name was truncated
//
#if defined(_MSC_VER)
#   pragma warning(disable: 4503)
#endif

using namespace std;

namespace
{

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

class Callback : public CallbackBase
{
public:

    Callback()
    {
    }

    Callback(const Ice::CommunicatorPtr& communicator)
        : _communicator(communicator)
    {
    }

    void ping()
    {
        called();
    }

    void isA(bool result)
    {
        test(result);
        called();
    }

    void id(const string& id)
    {
        test(id == Test::MyDerivedClass::ice_staticId());
        called();
    }

    void ids(const Ice::StringSeq& ids)
    {
        test(ids.size() == 3);
        test(ids[0] == "::Ice::Object");
        test(ids[1] == "::Test::MyClass");
        test(ids[2] == "::Test::MyDerivedClass");
        called();
    }

    void opVoid()
    {
        called();
    }

    void opContext(const Ice::Context&)
    {
        called();
    }

    void opByte(Ice::Byte r, Ice::Byte b)
    {
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

    void opBoolSS(const Test::BoolSS& rso, const Test::BoolSS& bso)
    {
        test(bso.size() == 4);
        test(bso[0].size() == 1);
        test(bso[0][0]);
        test(bso[1].size() == 1);
        test(!bso[1][0]);
        test(bso[2].size() == 2);
        test(bso[2][0]);
        test(bso[2][1]);
        test(bso[3].size() == 3);
        test(!bso[3][0]);
        test(!bso[3][1]);
        test(bso[3][2]);
        test(rso.size() == 3);
        test(rso[0].size() == 2);
        test(rso[0][0]);
        test(rso[0][1]);
        test(rso[1].size() == 1);
        test(!rso[1][0]);
        test(rso[2].size() == 1);
        test(rso[2][0]);
        called();
    }

    void opShortIntLongSS(const Test::LongSS& rso,
                          const Test::ShortSS& sso,
                          const Test::IntSS& iso,
                          const Test::LongSS& lso)
    {
        test(rso.size() == 1);
        test(rso[0].size() == 2);
        test(rso[0][0] == 496);
        test(rso[0][1] == 1729);
        test(sso.size() == 3);
        test(sso[0].size() == 3);
        test(sso[0][0] == 1);
        test(sso[0][1] == 2);
        test(sso[0][2] == 5);
        test(sso[1].size() == 1);
        test(sso[1][0] == 13);
        test(sso[2].size() == 0);
        test(iso.size() == 2);
        test(iso[0].size() == 1);
        test(iso[0][0] == 42);
        test(iso[1].size() == 2);
        test(iso[1][0] == 24);
        test(iso[1][1] == 98);
        test(lso.size() == 2);
        test(lso[0].size() == 2);
        test(lso[0][0] == 496);
        test(lso[0][1] == 1729);
        test(lso[1].size() == 2);
        test(lso[1][0] == 496);
        test(lso[1][1] == 1729);
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

    void opByteBoolDS(const Test::ByteBoolDS& ro, const Test::ByteBoolDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find(10) != ro[0].end());
        test(ro[0].find(10)->second == true);
        test(ro[0].find(11) != ro[0].end());
        test(ro[0].find(11)->second == false);
        test(ro[0].find(101) != ro[0].end());
        test(ro[0].find(101)->second == true);
        test(ro[1].size() == 2);
        test(ro[1].find(10) != ro[1].end());
        test(ro[1].find(10)->second == true);
        test(ro[1].find(100) != ro[1].end());
        test(ro[1].find(100)->second == false);
        test(_do.size() == 3);
        test(_do[0].size() == 2);
        test(_do[0].find(100) != _do[0].end());
        test(_do[0].find(100)->second == false);
        test(_do[0].find(101) != _do[0].end());
        test(_do[0].find(101)->second == false);
        test(_do[1].size() == 2);
        test(_do[1].find(10) != _do[1].end());
        test(_do[1].find(10)->second == true);
        test(_do[1].find(100) != _do[1].end());
        test(_do[1].find(100)->second == false);
        test(_do[2].size() == 3);
        test(_do[2].find(10) != _do[2].end());
        test(_do[2].find(10)->second == true);
        test(_do[2].find(11) != _do[2].end());
        test(_do[2].find(11)->second == false);
        test(_do[2].find(101) != _do[2].end());
        test(_do[2].find(101)->second == true);
        called();
    }

    void opShortIntDS(const Test::ShortIntDS& ro, const Test::ShortIntDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find(110) != ro[0].end());
        test(ro[0].find(110)->second == -1);
        test(ro[0].find(111) != ro[0].end());
        test(ro[0].find(111)->second == -100);
        test(ro[0].find(1101) != ro[0].end());
        test(ro[0].find(1101)->second == 0);
        test(ro[1].size() == 2);
        test(ro[1].find(110)->second == -1);
        test(ro[1].find(1100)->second == 123123);
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(100) != _do[0].end());
        test(_do[0].find(100)->second == -1001);
        test(_do[1].size() == 2);
        test(_do[1].find(110) != _do[1].end());
        test(_do[1].find(110)->second == -1);
        test(_do[1].find(1100) != _do[1].end());
        test(_do[1].find(1100)->second == 123123);
        test(_do[2].size() == 3);
        test(_do[2].find(110) != _do[2].end());
        test(_do[2].find(110)->second == -1);
        test(_do[2].find(111) != _do[2].end());
        test(_do[2].find(111)->second == -100);
        test(_do[2].find(1101) != _do[2].end());
        test(_do[2].find(1101)->second == 0);
        called();
    }

    void opLongFloatDS(const Test::LongFloatDS& ro, const Test::LongFloatDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find(999999110) != ro[0].end());
        test(ro[0].find(999999110)->second == Ice::Float(-1.1));
        test(ro[0].find(999999120) != ro[0].end());
        test(ro[0].find(999999120)->second == Ice::Float(-100.4));
        test(ro[0].find(999999130) != ro[0].end());
        test(ro[0].find(999999130)->second == Ice::Float(0.5));
        test(ro[1].size() == 2);
        test(ro[1].find(999999110) != ro[1].end());
        test(ro[1].find(999999110)->second == Ice::Float(-1.1));
        test(ro[1].find(999999111) != ro[1].end());
        test(ro[1].find(999999111)->second == Ice::Float(123123.2));
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(999999140) != _do[0].end());
        test(_do[0].find(999999140)->second == Ice::Float(3.14));
        test(_do[1].size() == 2);
        test(_do[1].find(999999110) != _do[1].end());
        test(_do[1].find(999999110)->second == Ice::Float(-1.1));
        test(_do[1].find(999999111) != _do[1].end());
        test(_do[1].find(999999111)->second == Ice::Float(123123.2));
        test(_do[2].size() == 3);
        test(_do[2].find(999999110) != _do[2].end());
        test(_do[2].find(999999110)->second == Ice::Float(-1.1));
        test(_do[2].find(999999120) != _do[2].end());
        test(_do[2].find(999999120)->second == Ice::Float(-100.4));
        test(_do[2].find(999999130) != _do[2].end());
        test(_do[2].find(999999130)->second == Ice::Float(0.5));
        called();
    }

    void opStringStringDS(const Test::StringStringDS& ro, const Test::StringStringDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find("foo") != ro[0].end());
        test(ro[0].find("foo")->second == "abc -1.1");
        test(ro[0].find("FOO") != ro[0].end());
        test(ro[0].find("FOO")->second == "abc -100.4");
        test(ro[0].find("BAR") != ro[0].end());
        test(ro[0].find("BAR")->second == "abc 0.5");
        test(ro[1].size() == 2);
        test(ro[1].find("foo") != ro[1].end());
        test(ro[1].find("foo")->second == "abc -1.1");
        test(ro[1].find("bar") != ro[1].end());
        test(ro[1].find("bar")->second == "abc 123123.2");
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find("f00") != _do[0].end());
        test(_do[0].find("f00")->second == "ABC -3.14");
        test(_do[1].size() == 2);
        test(_do[1].find("foo") != _do[1].end());
        test(_do[1].find("foo")->second == "abc -1.1");
        test(_do[1].find("bar") != _do[1].end());
        test(_do[1].find("bar")->second == "abc 123123.2");
        test(_do[2].size() == 3);
        test(_do[2].find("foo") != _do[2].end());
        test(_do[2].find("foo")->second == "abc -1.1");
        test(_do[2].find("FOO") != _do[2].end());
        test(_do[2].find("FOO")->second == "abc -100.4");
        test(_do[2].find("BAR") != _do[2].end());
        test(_do[2].find("BAR")->second == "abc 0.5");
        called();
    }

    void opStringMyEnumDS(const Test::StringMyEnumDS& ro, const Test::StringMyEnumDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find("abc") != ro[0].end());
        test(ro[0].find("abc")->second == Test::enum1);
        test(ro[0].find("qwerty") != ro[0].end());
        test(ro[0].find("qwerty")->second == Test::enum3);
        test(ro[0].find("Hello!!") != ro[0].end());
        test(ro[0].find("Hello!!")->second == Test::enum2);
        test(ro[1].size() == 2);
        test(ro[1].find("abc") != ro[1].end());
        test(ro[1].find("abc")->second == Test::enum1);
        test(ro[1].find("") != ro[1].end());
        test(ro[1].find("")->second == Test::enum2);
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find("Goodbye") != _do[0].end());
        test(_do[0].find("Goodbye")->second == Test::enum1);
        test(_do[1].size() == 2);
        test(_do[1].find("abc") != _do[1].end());
        test(_do[1].find("abc")->second == Test::enum1);
        test(_do[1].find("") != _do[1].end());
        test(_do[1].find("")->second == Test::enum2);
        test(_do[2].size() == 3);
        test(_do[2].find("abc") != _do[2].end());
        test(_do[2].find("abc")->second == Test::enum1);
        test(_do[2].find("qwerty") != _do[2].end());
        test(_do[2].find("qwerty")->second == Test::enum3);
        test(_do[2].find("Hello!!") != _do[2].end());
        test(_do[2].find("Hello!!")->second == Test::enum2);
        called();
    }

    void opMyEnumStringDS(const Test::MyEnumStringDS& ro, const Test::MyEnumStringDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 2);
        test(ro[0].find(Test::enum2) != ro[0].end());
        test(ro[0].find(Test::enum2)->second == "Hello!!");
        test(ro[0].find(Test::enum3) != ro[0].end());
        test(ro[0].find(Test::enum3)->second == "qwerty");
        test(ro[1].size() == 1);
        test(ro[1].find(Test::enum1) != ro[1].end());
        test(ro[1].find(Test::enum1)->second == "abc");
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(Test::enum1) != _do[0].end());
        test(_do[0].find(Test::enum1)->second == "Goodbye");
        test(_do[1].size() == 1);
        test(_do[1].find(Test::enum1) != _do[1].end());
        test(_do[1].find(Test::enum1)->second == "abc");
        test(_do[2].size() == 2);
        test(_do[2].find(Test::enum2) != _do[2].end());
        test(_do[2].find(Test::enum2)->second == "Hello!!");
        test(_do[2].find(Test::enum3) != _do[2].end());
        test(_do[2].find(Test::enum3)->second == "qwerty");
        called();
    }

    void opMyStructMyEnumDS(const Test::MyStructMyEnumDS& ro, const Test::MyStructMyEnumDS& _do)
    {
        Test::MyStruct s11 = { 1, 1 };
        Test::MyStruct s12 = { 1, 2 };
        Test::MyStruct s22 = { 2, 2 };
        Test::MyStruct s23 = { 2, 3 };

        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find(s11) != ro[0].end());
        test(ro[0].find(s11)->second == Test::enum1);
        test(ro[0].find(s22) != ro[0].end());
        test(ro[0].find(s22)->second == Test::enum3);
        test(ro[0].find(s23) != ro[0].end());
        test(ro[0].find(s23)->second == Test::enum2);
        test(ro[1].size() == 2);
        test(ro[1].find(s11) != ro[1].end());
        test(ro[1].find(s11)->second == Test::enum1);
        test(ro[1].find(s12) != ro[1].end());
        test(ro[1].find(s12)->second == Test::enum2);
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(s23) != _do[0].end());
        test(_do[0].find(s23)->second == Test::enum3);
        test(_do[1].size() == 2);
        test(_do[1].find(s11) != _do[1].end());
        test(_do[1].find(s11)->second == Test::enum1);
        test(_do[1].find(s12) != _do[1].end());
        test(_do[1].find(s12)->second == Test::enum2);
        test(_do[2].size() == 3);
        test(_do[2].find(s11) != _do[2].end());
        test(_do[2].find(s11)->second == Test::enum1);
        test(_do[2].find(s22) != _do[2].end());
        test(_do[2].find(s22)->second == Test::enum3);
        test(_do[2].find(s23) != _do[2].end());
        test(_do[2].find(s23)->second == Test::enum2);
        called();
    }

    void opByteByteSD(const Test::ByteByteSD& ro, const Test::ByteByteSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(Ice::Byte(0xf1)) != _do.end());
        test(_do.find(Ice::Byte(0xf1))->second.size() == 2);
        test(_do.find(Ice::Byte(0xf1))->second[0] == 0xf2);
        test(_do.find(Ice::Byte(0xf1))->second[1] == 0xf3);
        test(ro.size() == 3);
        test(ro.find(Ice::Byte(0x01)) != ro.end());
        test(ro.find(Ice::Byte(0x01))->second.size() == 2);
        test(ro.find(Ice::Byte(0x01))->second[0] == Ice::Byte(0x01));
        test(ro.find(Ice::Byte(0x01))->second[1] == Ice::Byte(0x11));
        test(ro.find(Ice::Byte(0x22)) != ro.end());
        test(ro.find(Ice::Byte(0x22))->second.size() == 1);
        test(ro.find(Ice::Byte(0x22))->second[0] == Ice::Byte(0x12));
        test(ro.find(Ice::Byte(0xf1)) != ro.end());
        test(ro.find(Ice::Byte(0xf1))->second.size() == 2);
        test(ro.find(Ice::Byte(0xf1))->second[0] == Ice::Byte(0xf2));
        test(ro.find(Ice::Byte(0xf1))->second[1] == Ice::Byte(0xf3));
        called();
    }

    void opBoolBoolSD(const Test::BoolBoolSD& ro, const Test::BoolBoolSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(false) != _do.end());
        test(_do.find(false)->second.size() == 2);
        test(_do.find(false)->second[0] == true);
        test(_do.find(false)->second[1] == false);
        test(ro.size() == 2);
        test(ro.find(false) != ro.end());
        test(ro.find(false)->second.size() == 2);
        test(ro.find(false)->second[0] == true);
        test(ro.find(false)->second[1] == false);
        test(ro.find(true) != ro.end());
        test(ro.find(true)->second.size()  == 3);
        test(ro.find(true)->second[0] == false);
        test(ro.find(true)->second[1] == true);
        test(ro.find(true)->second[2] == true);
        called();
    }

    void opShortShortSD(const Test::ShortShortSD& ro, const Test::ShortShortSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(4) != _do.end());
        test(_do.find(4)->second.size() == 2);
        test(_do.find(4)->second[0] == 6);
        test(_do.find(4)->second[1] == 7);
        test(ro.size() == 3);
        test(ro.find(1) != ro.end());
        test(ro.find(1)->second.size() == 3);
        test(ro.find(1)->second[0] == 1);
        test(ro.find(1)->second[1] == 2);
        test(ro.find(1)->second[2] == 3);
        test(ro.find(2) != ro.end());
        test(ro.find(2)->second.size() == 2);
        test(ro.find(2)->second[0] == 4);
        test(ro.find(2)->second[1] == 5);
        test(ro.find(4) != ro.end());
        test(ro.find(4)->second.size() == 2);
        test(ro.find(4)->second[0] == 6);
        test(ro.find(4)->second[1] == 7);
        called();
    }

    void opIntIntSD(const Test::IntIntSD& ro, const Test::IntIntSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(400) != _do.end());
        test(_do.find(400)->second.size() == 2);
        test(_do.find(400)->second[0] == 600);
        test(_do.find(400)->second[1] == 700);
        test(ro.size() == 3);
        test(ro.find(100) != ro.end());
        test(ro.find(100)->second.size() == 3);
        test(ro.find(100)->second[0] == 100);
        test(ro.find(100)->second[1] == 200);
        test(ro.find(100)->second[2] == 300);
        test(ro.find(200) != ro.end());
        test(ro.find(200)->second.size() == 2);
        test(ro.find(200)->second[0] == 400);
        test(ro.find(200)->second[1] == 500);
        test(ro.find(400) != ro.end());
        test(ro.find(400)->second.size() == 2);
        test(ro.find(400)->second[0] == 600);
        test(ro.find(400)->second[1] == 700);
        called();
    }

    void opLongLongSD(const Test::LongLongSD& ro, const Test::LongLongSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(999999992) != _do.end());
        test(_do.find(999999992)->second.size() == 2);
        test(_do.find(999999992)->second[0] == 999999110);
        test(_do.find(999999992)->second[1] == 999999120);
        test(ro.size() == 3);
        test(ro.find(999999990) != ro.end());
        test(ro.find(999999990)->second.size() == 3);
        test(ro.find(999999990)->second[0] == 999999110);
        test(ro.find(999999990)->second[1] == 999999111);
        test(ro.find(999999990)->second[2] == 999999110);
        test(ro.find(999999991) != ro.end());
        test(ro.find(999999991)->second.size() == 2);
        test(ro.find(999999991)->second[0] == 999999120);
        test(ro.find(999999991)->second[1] == 999999130);
        test(ro.find(999999992) != ro.end());
        test(ro.find(999999992)->second.size() == 2);
        test(ro.find(999999992)->second[0] == 999999110);
        test(ro.find(999999992)->second[1] == 999999120);
        called();
    }

    void opStringFloatSD(const Test::StringFloatSD& ro, const Test::StringFloatSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find("aBc") != _do.end());
        test(_do.find("aBc")->second.size() == 2);
        test(_do.find("aBc")->second[0] == Ice::Float(-3.14));
        test(_do.find("aBc")->second[1] == Ice::Float(3.14));
        test(ro.size() == 3);
        test(ro.find("abc") != ro.end());
        test(ro.find("abc")->second.size() == 3);
        test(ro.find("abc")->second[0] == Ice::Float(-1.1));
        test(ro.find("abc")->second[1] == Ice::Float(123123.2));
        test(ro.find("abc")->second[2] == Ice::Float(100.0));
        test(ro.find("ABC") != ro.end());
        test(ro.find("ABC")->second.size() == 2);
        test(ro.find("ABC")->second[0] == Ice::Float(42.24));
        test(ro.find("ABC")->second[1] == Ice::Float(-1.61));
        test(ro.find("aBc") != ro.end());
        test(ro.find("aBc")->second.size() == 2);
        test(ro.find("aBc")->second[0] == Ice::Float(-3.14));
        test(ro.find("aBc")->second[1] == Ice::Float(3.14));
        called();
    }

    void opStringDoubleSD(const Test::StringDoubleSD& ro, const Test::StringDoubleSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find("") != _do.end());
        test(_do.find("")->second.size() == 2);
        test(_do.find("")->second[0] == Ice::Double(1.6E10));
        test(_do.find("")->second[1] == Ice::Double(1.7E10));
        test(ro.size() == 3);
        test(ro.find("Hello!!") != ro.end());
        test(ro.find("Hello!!")->second.size() == 3);
        test(ro.find("Hello!!")->second[0] == Ice::Double(1.1E10));
        test(ro.find("Hello!!")->second[1] == Ice::Double(1.2E10));
        test(ro.find("Hello!!")->second[2] == Ice::Double(1.3E10));
        test(ro.find("Goodbye") != ro.end());
        test(ro.find("Goodbye")->second.size() == 2);
        test(ro.find("Goodbye")->second[0] == Ice::Double(1.4E10));
        test(ro.find("Goodbye")->second[1] == Ice::Double(1.5E10));
        test(ro.find("") != ro.end());
        test(ro.find("")->second.size() == 2);
        test(ro.find("")->second[0] == Ice::Double(1.6E10));
        test(ro.find("")->second[1] == Ice::Double(1.7E10));
        called();
    }

    void opStringStringSD(const Test::StringStringSD& ro, const Test::StringStringSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find("ghi") != _do.end());
        test(_do.find("ghi")->second.size() == 2);
        test(_do.find("ghi")->second[0] == "and");
        test(_do.find("ghi")->second[1] == "xor");
        test(ro.size() == 3);
        test(ro.find("abc") != ro.end());
        test(ro.find("abc")->second.size() == 3);
        test(ro.find("abc")->second[0] == "abc");
        test(ro.find("abc")->second[1] == "de");
        test(ro.find("abc")->second[2] == "fghi");
        test(ro.find("def") != ro.end());
        test(ro.find("def")->second.size() == 2);
        test(ro.find("def")->second[0] == "xyz");
        test(ro.find("def")->second[1] == "or");
        test(ro.find("ghi") != ro.end());
        test(ro.find("ghi")->second.size() == 2);
        test(ro.find("ghi")->second[0] == "and");
        test(ro.find("ghi")->second[1] == "xor");
        called();
    }

    void opMyEnumMyEnumSD(const Test::MyEnumMyEnumSD& ro, const Test::MyEnumMyEnumSD& _do)
    {
        test(_do.size() == 1);
        test(_do.find(Test::enum1) != _do.end());
        test(_do.find(Test::enum1)->second.size() == 2);
        test(_do.find(Test::enum1)->second[0] == Test::enum3);
        test(_do.find(Test::enum1)->second[1] == Test::enum3);
        test(ro.size() == 3);
        test(ro.find(Test::enum3) != ro.end());
        test(ro.find(Test::enum3)->second.size() == 3);
        test(ro.find(Test::enum3)->second[0] == Test::enum1);
        test(ro.find(Test::enum3)->second[1] == Test::enum1);
        test(ro.find(Test::enum3)->second[2] == Test::enum2);
        test(ro.find(Test::enum2) != ro.end());
        test(ro.find(Test::enum2)->second.size() == 2);
        test(ro.find(Test::enum2)->second[0] == Test::enum1);
        test(ro.find(Test::enum2)->second[1] == Test::enum2);
        test(ro.find(Test::enum1) != ro.end());
        test(ro.find(Test::enum1)->second.size() == 2);
        test(ro.find(Test::enum1)->second[0] == Test::enum3);
        test(ro.find(Test::enum1)->second[1] == Test::enum3);
        called();
    }

    void opIntS(const Test::IntS& r)
    {
        for(int j = 0; j < static_cast<int>(r.size()); ++j)
        {
            test(r[j] == -j);
        }
        called();
    }

    void opDoubleMarshaling()
    {
        called();
    }

    void opIdempotent()
    {
        called();
    }

    void opNonmutating()
    {
        called();
    }

    void opDerived()
    {
        called();
    }

    void exCB(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch (const Ice::OperationNotExistException&)
        {
            called();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }

private:

    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

}

void
twowaysAMI(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrx& p)
{
    {
        CallbackPtr cb = new Callback;
        Ice::Callback_Object_ice_pingPtr callback = Ice::newCallback_Object_ice_ping(cb,
                                                                                     &Callback::ping,
                                                                                     &Callback::exCB);
        p->begin_ice_ping(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Ice::Callback_Object_ice_isAPtr callback = Ice::newCallback_Object_ice_isA(cb,
                                                                                   &Callback::isA,
                                                                                   &Callback::exCB);
        p->begin_ice_isA(Test::MyClass::ice_staticId(), callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Ice::Callback_Object_ice_idPtr callback = Ice::newCallback_Object_ice_id(cb,
                                                                                 &Callback::id,
                                                                                 &Callback::exCB);
        p->begin_ice_id(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Ice::Callback_Object_ice_idsPtr callback = Ice::newCallback_Object_ice_ids(cb,
                                                                                   &Callback::ids,
                                                                                   &Callback::exCB);
        p->begin_ice_ids(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                     &Callback::opVoid,
                                                                                     &Callback::exCB);
        p->begin_opVoid(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opBytePtr callback = Test::newCallback_MyClass_opByte(cb,
                                                                                     &Callback::opByte,
                                                                                     &Callback::exCB);
        p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opBoolPtr callback = Test::newCallback_MyClass_opBool(cb,
                                                                                     &Callback::opBool,
                                                                                     &Callback::exCB);
        p->begin_opBool(true, false, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opShortIntLongPtr callback =
            Test::newCallback_MyClass_opShortIntLong(cb, &Callback::opShortIntLong, &Callback::exCB);
        p->begin_opShortIntLong(10, 11, 12, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opFloatDoublePtr callback =
            Test::newCallback_MyClass_opFloatDouble(cb, &Callback::opFloatDouble, &Callback::exCB);
        p->begin_opFloatDouble(Ice::Float(3.14), Ice::Double(1.1E10), callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringPtr callback = Test::newCallback_MyClass_opString(cb,
                                                                                         &Callback::opString,
                                                                                         &Callback::exCB);
        p->begin_opString("hello", "world", callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opMyEnumPtr callback = Test::newCallback_MyClass_opMyEnum(cb,
                                                                                         &Callback::opMyEnum,
                                                                                         &Callback::exCB);
        p->begin_opMyEnum(Test::enum2, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback(communicator);
        Test::Callback_MyClass_opMyClassPtr callback = Test::newCallback_MyClass_opMyClass(cb,
                                                                                           &Callback::opMyClass,
                                                                                           &Callback::exCB);
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
                                                                                         &Callback::exCB);
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
                                                                                       &Callback::exCB);
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
                                                                                       &Callback::exCB);
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
            Test::newCallback_MyClass_opShortIntLongS(cb, &Callback::opShortIntLongS, &Callback::exCB);
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
            Test::newCallback_MyClass_opFloatDoubleS(cb, &Callback::opFloatDoubleS, &Callback::exCB);
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
                                                                                           &Callback::exCB);
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
                                                                                         &Callback::exCB);
        p->begin_opByteSS(bsi1, bsi2, callback);
        cb->check();
    }

    {
        Test::BoolSS bsi1;
        bsi1.resize(3);
        Test::BoolSS bsi2;
        bsi2.resize(1);

        bsi1[0].push_back(true);
        bsi1[1].push_back(false);
        bsi1[2].push_back(true);
        bsi1[2].push_back(true);

        bsi2[0].push_back(false);
        bsi2[0].push_back(false);
        bsi2[0].push_back(true);

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opBoolSSPtr callback = Test::newCallback_MyClass_opBoolSS(cb,
                                                                                         &Callback::opBoolSS,
                                                                                         &Callback::exCB);
        p->begin_opBoolSS(bsi1, bsi2, callback);
        cb->check();
    }

    {
        Test::ShortSS ssi;
        ssi.resize(3);
        Test::IntSS isi;
        isi.resize(2);
        Test::LongSS lsi;
        lsi.resize(1);
        ssi[0].push_back(1);
        ssi[0].push_back(2);
        ssi[0].push_back(5);
        ssi[1].push_back(13);
        isi[0].push_back(24);
        isi[0].push_back(98);
        isi[1].push_back(42);
        lsi[0].push_back(496);
        lsi[0].push_back(1729);

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opShortIntLongSSPtr callback = Test::newCallback_MyClass_opShortIntLongSS(cb,
                                                                                         &Callback::opShortIntLongSS,
                                                                                         &Callback::exCB);
        p->begin_opShortIntLongSS(ssi, isi, lsi, callback);
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
            Test::newCallback_MyClass_opFloatDoubleSS(cb, &Callback::opFloatDoubleSS, &Callback::exCB);
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
            Test::newCallback_MyClass_opStringSS(cb, &Callback::opStringSS, &Callback::exCB);
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
            Test::newCallback_MyClass_opByteBoolD(cb, &Callback::opByteBoolD, &Callback::exCB);
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
            Test::newCallback_MyClass_opShortIntD(cb, &Callback::opShortIntD, &Callback::exCB);
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
            Test::newCallback_MyClass_opLongFloatD(cb, &Callback::opLongFloatD, &Callback::exCB);
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
            Test::newCallback_MyClass_opStringStringD(cb, &Callback::opStringStringD, &Callback::exCB);
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
            Test::newCallback_MyClass_opStringMyEnumD(cb, &Callback::opStringMyEnumD, &Callback::exCB);
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
            Test::newCallback_MyClass_opMyStructMyEnumD(cb, &Callback::opMyStructMyEnumD, &Callback::exCB);
        p->begin_opMyStructMyEnumD(di1, di2, callback);
        cb->check();
    }

    {
        Test::ByteBoolDS dsi1;
        dsi1.resize(2);
        Test::ByteBoolDS dsi2;
        dsi2.resize(1);

        Test::ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        Test::ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;
        Test::ByteBoolD di3;
        di3[100] = false;
        di3[101] = false;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opByteBoolDSPtr callback =
            Test::newCallback_MyClass_opByteBoolDS(cb, &Callback::opByteBoolDS, &Callback::exCB);
        p->begin_opByteBoolDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::ShortIntDS dsi1;
        dsi1.resize(2);
        Test::ShortIntDS dsi2;
        dsi2.resize(1);

        Test::ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        Test::ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;
        Test::ShortIntD di3;
        di3[100] = -1001;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opShortIntDSPtr callback =
            Test::newCallback_MyClass_opShortIntDS(cb, &Callback::opShortIntDS, &Callback::exCB);
        p->begin_opShortIntDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::LongFloatDS dsi1;
        dsi1.resize(2);
        Test::LongFloatDS dsi2;
        dsi2.resize(1);

        Test::LongFloatD di1;
        di1[999999110] = Ice::Float(-1.1);
        di1[999999111] = Ice::Float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = Ice::Float(-1.1);
        di2[999999120] = Ice::Float(-100.4);
        di2[999999130] = Ice::Float(0.5);
        Test::LongFloatD di3;
        di3[999999140] = Ice::Float(3.14);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opLongFloatDSPtr callback =
            Test::newCallback_MyClass_opLongFloatDS(cb, &Callback::opLongFloatDS, &Callback::exCB);
        p->begin_opLongFloatDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::StringStringDS dsi1;
        dsi1.resize(2);
        Test::StringStringDS dsi2;
        dsi2.resize(1);

        Test::StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        Test::StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";
        Test::StringStringD di3;
        di3["f00"] = "ABC -3.14";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringStringDSPtr callback =
            Test::newCallback_MyClass_opStringStringDS(cb, &Callback::opStringStringDS, &Callback::exCB);
        p->begin_opStringStringDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::StringMyEnumDS dsi1;
        dsi1.resize(2);
        Test::StringMyEnumDS dsi2;
        dsi2.resize(1);

        Test::StringMyEnumD di1;
        di1["abc"] = Test::enum1;
        di1[""] = Test::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = Test::enum1;
        di2["qwerty"] = Test::enum3;
        di2["Hello!!"] = Test::enum2;
        Test::StringMyEnumD di3;
        di3["Goodbye"] = Test::enum1;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringMyEnumDSPtr callback =
            Test::newCallback_MyClass_opStringMyEnumDS(cb, &Callback::opStringMyEnumDS, &Callback::exCB);
        p->begin_opStringMyEnumDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::MyEnumStringDS dsi1;
        dsi1.resize(2);
        Test::MyEnumStringDS dsi2;
        dsi2.resize(1);

        Test::MyEnumStringD di1;
        di1[Test::enum1] = "abc";
        Test::MyEnumStringD di2;
        di2[Test::enum2] = "Hello!!";
        di2[Test::enum3] = "qwerty";
        Test::MyEnumStringD di3;
        di3[Test::enum1] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opMyEnumStringDSPtr callback =
            Test::newCallback_MyClass_opMyEnumStringDS(cb, &Callback::opMyEnumStringDS, &Callback::exCB);
        p->begin_opMyEnumStringDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        Test::MyStructMyEnumDS dsi2;
        dsi2.resize(1);

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

        Test::MyStructMyEnumD di3;
        di3[s23] = Test::enum3;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opMyStructMyEnumDSPtr callback =
            Test::newCallback_MyClass_opMyStructMyEnumDS(cb, &Callback::opMyStructMyEnumDS, &Callback::exCB);
        p->begin_opMyStructMyEnumDS(dsi1, dsi2, callback);
        cb->check();
    }

    {
        Test::ByteByteSD sdi1;
        Test::ByteByteSD sdi2;

        Test::ByteS si1;
        Test::ByteS si2;
        Test::ByteS si3;

        si1.push_back(Ice::Byte(0x01));
        si1.push_back(Ice::Byte(0x11));
        si2.push_back(Ice::Byte(0x12));
        si3.push_back(Ice::Byte(0xf2));
        si3.push_back(Ice::Byte(0xf3));

        sdi1[Ice::Byte(0x01)] = si1;
        sdi1[Ice::Byte(0x22)] = si2;
        sdi2[Ice::Byte(0xf1)] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opByteByteSDPtr callback =
            Test::newCallback_MyClass_opByteByteSD(cb, &Callback::opByteByteSD, &Callback::exCB);
        p->begin_opByteByteSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::BoolBoolSD sdi1;
        Test::BoolBoolSD sdi2;

        Test::BoolS si1;
        Test::BoolS si2;

        si1.push_back(true);
        si1.push_back(false);
        si2.push_back(false);
        si2.push_back(true);
        si2.push_back(true);

        sdi1[false] = si1;
        sdi1[true] = si2;
        sdi2[false] = si1;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opBoolBoolSDPtr callback =
            Test::newCallback_MyClass_opBoolBoolSD(cb, &Callback::opBoolBoolSD, &Callback::exCB);
        p->begin_opBoolBoolSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::ShortShortSD sdi1;
        Test::ShortShortSD sdi2;

        Test::ShortS si1;
        Test::ShortS si2;
        Test::ShortS si3;

        si1.push_back(1);
        si1.push_back(2);
        si1.push_back(3);
        si2.push_back(4);
        si2.push_back(5);
        si3.push_back(6);
        si3.push_back(7);

        sdi1[1] = si1;
        sdi1[2] = si2;
        sdi2[4] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opShortShortSDPtr callback =
            Test::newCallback_MyClass_opShortShortSD(cb, &Callback::opShortShortSD, &Callback::exCB);
        p->begin_opShortShortSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::IntIntSD sdi1;
        Test::IntIntSD sdi2;

        Test::IntS si1;
        Test::IntS si2;
        Test::IntS si3;

        si1.push_back(100);
        si1.push_back(200);
        si1.push_back(300);
        si2.push_back(400);
        si2.push_back(500);
        si3.push_back(600);
        si3.push_back(700);

        sdi1[100] = si1;
        sdi1[200] = si2;
        sdi2[400] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opIntIntSDPtr callback =
            Test::newCallback_MyClass_opIntIntSD(cb, &Callback::opIntIntSD, &Callback::exCB);
        p->begin_opIntIntSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::LongLongSD sdi1;
        Test::LongLongSD sdi2;

        Test::LongS si1;
        Test::LongS si2;
        Test::LongS si3;

        si1.push_back(999999110);
        si1.push_back(999999111);
        si1.push_back(999999110);
        si2.push_back(999999120);
        si2.push_back(999999130);
        si3.push_back(999999110);
        si3.push_back(999999120);

        sdi1[999999990] = si1;
        sdi1[999999991] = si2;
        sdi2[999999992] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opLongLongSDPtr callback =
            Test::newCallback_MyClass_opLongLongSD(cb, &Callback::opLongLongSD, &Callback::exCB);
        p->begin_opLongLongSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::StringFloatSD sdi1;
        Test::StringFloatSD sdi2;

        Test::FloatS si1;
        Test::FloatS si2;
        Test::FloatS si3;

        si1.push_back(Ice::Float(-1.1));
        si1.push_back(Ice::Float(123123.2));
        si1.push_back(Ice::Float(100.0));
        si2.push_back(Ice::Float(42.24));
        si2.push_back(Ice::Float(-1.61));
        si3.push_back(Ice::Float(-3.14));
        si3.push_back(Ice::Float(3.14));

        sdi1["abc"] = si1;
        sdi1["ABC"] = si2;
        sdi2["aBc"] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringFloatSDPtr callback =
            Test::newCallback_MyClass_opStringFloatSD(cb, &Callback::opStringFloatSD, &Callback::exCB);
        p->begin_opStringFloatSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::StringDoubleSD sdi1;
        Test::StringDoubleSD sdi2;

        Test::DoubleS si1;
        Test::DoubleS si2;
        Test::DoubleS si3;

        si1.push_back(Ice::Double(1.1E10));
        si1.push_back(Ice::Double(1.2E10));
        si1.push_back(Ice::Double(1.3E10));
        si2.push_back(Ice::Double(1.4E10));
        si2.push_back(Ice::Double(1.5E10));
        si3.push_back(Ice::Double(1.6E10));
        si3.push_back(Ice::Double(1.7E10));

        sdi1["Hello!!"] = si1;
        sdi1["Goodbye"] = si2;
        sdi2[""] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringDoubleSDPtr callback =
            Test::newCallback_MyClass_opStringDoubleSD(cb, &Callback::opStringDoubleSD, &Callback::exCB);
        p->begin_opStringDoubleSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::StringStringSD sdi1;
        Test::StringStringSD sdi2;

        Test::StringS si1;
        Test::StringS si2;
        Test::StringS si3;

        si1.push_back("abc");
        si1.push_back("de");
        si1.push_back("fghi");

        si2.push_back("xyz");
        si2.push_back("or");

        si3.push_back("and");
        si3.push_back("xor");

        sdi1["abc"] = si1;
        sdi1["def"] = si2;
        sdi2["ghi"] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opStringStringSDPtr callback =
            Test::newCallback_MyClass_opStringStringSD(cb, &Callback::opStringStringSD, &Callback::exCB);
        p->begin_opStringStringSD(sdi1, sdi2, callback);
        cb->check();
    }

    {
        Test::MyEnumMyEnumSD sdi1;
        Test::MyEnumMyEnumSD sdi2;

        Test::MyEnumS si1;
        Test::MyEnumS si2;
        Test::MyEnumS si3;

        si1.push_back(Test::enum1);
        si1.push_back(Test::enum1);
        si1.push_back(Test::enum2);
        si2.push_back(Test::enum1);
        si2.push_back(Test::enum2);
        si3.push_back(Test::enum3);
        si3.push_back(Test::enum3);

        sdi1[Test::enum3] = si1;
        sdi1[Test::enum2] = si2;
        sdi2[Test::enum1] = si3;

        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opMyEnumMyEnumSDPtr callback =
            Test::newCallback_MyClass_opMyEnumMyEnumSD(cb, &Callback::opMyEnumMyEnumSD, &Callback::exCB);
        p->begin_opMyEnumMyEnumSD(sdi1, sdi2, callback);
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
            CallbackPtr cb = new Callback;
            Test::Callback_MyClass_opIntSPtr callback =
                Test::newCallback_MyClass_opIntS(cb, &Callback::opIntS, &Callback::exCB);
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

#ifndef ICE_OS_WINRT
        if(p->ice_getConnection())
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
#endif
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opDoubleMarshalingPtr callback =
            Test::newCallback_MyClass_opDoubleMarshaling(cb, &Callback::opDoubleMarshaling, &Callback::exCB);
        p->begin_opDoubleMarshaling(d, ds, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opIdempotentPtr callback =
            Test::newCallback_MyClass_opIdempotent(cb, &Callback::opIdempotent, &Callback::exCB);
        p->begin_opIdempotent(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opNonmutatingPtr callback =
            Test::newCallback_MyClass_opNonmutating(cb, &Callback::opNonmutating, &Callback::exCB);
        p->begin_opNonmutating(callback);
        cb->check();
    }

    {
        Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(p);
        test(derived);
        CallbackPtr cb = new Callback;
        Test::Callback_MyDerivedClass_opDerivedPtr callback =
            Test::newCallback_MyDerivedClass_opDerived(cb, &Callback::opDerived, &Callback::exCB);
        derived->begin_opDerived(callback);
        cb->check();
    }
#ifdef ICE_CPP11

    {
        CallbackPtr cb = new Callback;
        p->begin_ice_ping([=](){ cb->ping(); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_ice_isA(Test::MyClass::ice_staticId(), [=](bool isA){ cb->isA(isA); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_ice_id([=](const string& id){ cb->id(id); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_ice_ids([=](const Ice::StringSeq& ids){ cb->ids(ids); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opVoid([=](){ cb->opVoid(); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f),
                        [=](const Ice::Byte& p1, const Ice::Byte& p2){ cb->opByte(p1, p2); },
                        [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opBoolPtr callback = Test::newCallback_MyClass_opBool(cb,
                                                                                     &Callback::opBool,
                                                                                     &Callback::exCB);
        p->begin_opBool(true, false, [=](bool p1, bool p2){ cb->opBool(p1, p2); }, [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opShortIntLong(10, 11, 12,
                                [=](Ice::Long p1, Ice::Short p2, Ice::Int p3, Ice::Long p4){ cb->opShortIntLong(p1, p2, p3, p4); },
                                [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opFloatDouble(Ice::Float(3.14), Ice::Double(1.1E10),
                               [=](Ice::Double p1, Ice::Float p2, Ice::Double p3){ cb->opFloatDouble(p1, p2, p3); },
                               [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opString("hello", "world",
                          [=](const string& p1, const string& p2){ cb->opString(p1, p2); },
                          [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opMyEnum(Test::enum2,
                          [=](Test::MyEnum p1, Test::MyEnum p2){ cb->opMyEnum(p1, p2); },
                          [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback(communicator);
        p->begin_opMyClass(p,
                        [=](const Test::MyClassPrx& p1, const Test::MyClassPrx p2, const Test::MyClassPrx p3)
                            {
                                cb->opMyClass(p1, p2, p3);
                            },
                        [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opStruct(si1, si2,
                            [=](const Test::Structure& p1, const Test::Structure& p2)
                                {
                                    cb->opStruct(p1, p2);
                                },
                            [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
                                                                                       &Callback::exCB);
        p->begin_opByteS(bsi1, bsi2,
                        [=](const Test::ByteS& p1, const Test::ByteS& p2)
                            {
                                cb->opByteS(p1, p2);
                            },
                        [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opBoolS(bsi1, bsi2,
                            [=](const Test::BoolS& p1, const Test::BoolS& p2)
                                {
                                    cb->opBoolS(p1, p2);
                                },
                            [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opShortIntLongS(ssi, isi, lsi,
                                [=](const Test::LongS& p1, const Test::ShortS& p2, const Test::IntS& p3, const Test::LongS& p4)
                                    {
                                        cb->opShortIntLongS(p1, p2, p3, p4);
                                    },
                                [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opFloatDoubleS(fsi, dsi,
                                [=](const Test::DoubleS& p1, const Test::FloatS& p2, const Test::DoubleS& p3)
                                    {
                                        cb->opFloatDoubleS(p1, p2, p3);
                                    },
                                [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opStringS(ssi1, ssi2,
                            [=](const Test::StringS& p1, const Test::StringS& p2)
                                {
                                    cb->opStringS(p1, p2);
                                },
                            [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opByteSS(bsi1, bsi2,
                            [=](const Test::ByteSS& p1, const Test::ByteSS& p2)
                                {
                                    cb->opByteSS(p1, p2);
                                },
                            [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opFloatDoubleSS(fsi, dsi,
                                    [=](const Test::DoubleSS& p1, const Test::FloatSS& p2, const Test::DoubleSS& p3)
                                        {
                                            cb->opFloatDoubleSS(p1, p2, p3);
                                        },
                                    [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opStringSS(ssi1, ssi2,
                                [=](const Test::StringSS& p1, const Test::StringSS& p2)
                                    {
                                        cb->opStringSS(p1, p2);
                                    },
                                [=](const Ice::Exception& ex){ cb->exCB(ex); });
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
        p->begin_opByteBoolD(di1, di2,
                                [=](const Test::ByteBoolD& p1, const Test::ByteBoolD& p2)
                                    {
                                        cb->opByteBoolD(p1, p2);
                                    },
                                [=](const Ice::Exception& ex)
                                    {
                                        cb->exCB(ex);
                                    });
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
        p->begin_opShortIntD(di1, di2,
                                [=](const Test::ShortIntD& p1, const Test::ShortIntD& p2)
                                    {
                                        cb->opShortIntD(p1, p2);
                                    },
                                [=](const Ice::Exception& ex)
                                    {
                                        cb->exCB(ex);
                                    });
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
        p->begin_opLongFloatD(di1, di2,
                                [=](const Test::LongFloatD& p1, const Test::LongFloatD& p2)
                                    {
                                        cb->opLongFloatD(p1, p2);
                                    },
                                [=](const Ice::Exception& ex)
                                    {
                                        cb->exCB(ex);
                                    });
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
        p->begin_opStringStringD(di1, di2,
                                    [=](const Test::StringStringD& p1, const Test::StringStringD& p2)
                                        {
                                            cb->opStringStringD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
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
        p->begin_opStringMyEnumD(di1, di2,
                                    [=](const Test::StringMyEnumD& p1, const Test::StringMyEnumD& p2)
                                        {
                                            cb->opStringMyEnumD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
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
        p->begin_opMyStructMyEnumD(di1, di2,
                                    [=](const Test::MyStructMyEnumD& p1, const Test::MyStructMyEnumD& p2)
                                        {
                                            cb->opMyStructMyEnumD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::ByteBoolDS dsi1;
        dsi1.resize(2);
        Test::ByteBoolDS dsi2;
        dsi2.resize(1);

        Test::ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        Test::ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;
        Test::ByteBoolD di3;
        di3[100] = false;
        di3[101] = false;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opByteBoolDS(dsi1, dsi2,
                                    [=](const Test::ByteBoolDS& p1, const Test::ByteBoolDS& p2)
                                        {
                                            cb->opByteBoolDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::ShortIntDS dsi1;
        dsi1.resize(2);
        Test::ShortIntDS dsi2;
        dsi2.resize(1);

        Test::ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        Test::ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;
        Test::ShortIntD di3;
        di3[100] = -1001;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opShortIntDS(dsi1, dsi2,
                                    [=](const Test::ShortIntDS& p1, const Test::ShortIntDS& p2)
                                        {
                                            cb->opShortIntDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::LongFloatDS dsi1;
        dsi1.resize(2);
        Test::LongFloatDS dsi2;
        dsi2.resize(1);

        Test::LongFloatD di1;
        di1[999999110] = Ice::Float(-1.1);
        di1[999999111] = Ice::Float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = Ice::Float(-1.1);
        di2[999999120] = Ice::Float(-100.4);
        di2[999999130] = Ice::Float(0.5);
        Test::LongFloatD di3;
        di3[999999140] = Ice::Float(3.14);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opLongFloatDS(dsi1, dsi2,
                                    [=](const Test::LongFloatDS& p1, const Test::LongFloatDS& p2)
                                        {
                                            cb->opLongFloatDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::StringStringDS dsi1;
        dsi1.resize(2);
        Test::StringStringDS dsi2;
        dsi2.resize(1);

        Test::StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        Test::StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";
        Test::StringStringD di3;
        di3["f00"] = "ABC -3.14";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opStringStringDS(dsi1, dsi2,
                                    [=](const Test::StringStringDS& p1, const Test::StringStringDS& p2)
                                        {
                                            cb->opStringStringDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::StringMyEnumDS dsi1;
        dsi1.resize(2);
        Test::StringMyEnumDS dsi2;
        dsi2.resize(1);

        Test::StringMyEnumD di1;
        di1["abc"] = Test::enum1;
        di1[""] = Test::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = Test::enum1;
        di2["qwerty"] = Test::enum3;
        di2["Hello!!"] = Test::enum2;
        Test::StringMyEnumD di3;
        di3["Goodbye"] = Test::enum1;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opStringMyEnumDS(dsi1, dsi2,
                                    [=](const Test::StringMyEnumDS& p1, const Test::StringMyEnumDS& p2)
                                        {
                                            cb->opStringMyEnumDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::MyEnumStringDS dsi1;
        dsi1.resize(2);
        Test::MyEnumStringDS dsi2;
        dsi2.resize(1);

        Test::MyEnumStringD di1;
        di1[Test::enum1] = "abc";
        Test::MyEnumStringD di2;
        di2[Test::enum2] = "Hello!!";
        di2[Test::enum3] = "qwerty";
        Test::MyEnumStringD di3;
        di3[Test::enum1] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opMyEnumStringDS(dsi1, dsi2,
                                    [=](const Test::MyEnumStringDS& p1, const Test::MyEnumStringDS& p2)
                                        {
                                            cb->opMyEnumStringDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        Test::MyStructMyEnumDS dsi2;
        dsi2.resize(1);

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

        Test::MyStructMyEnumD di3;
        di3[s23] = Test::enum3;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = new Callback;
        p->begin_opMyStructMyEnumDS(dsi1, dsi2,
                                    [=](const Test::MyStructMyEnumDS& p1, const Test::MyStructMyEnumDS& p2)
                                        {
                                            cb->opMyStructMyEnumDS(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::ByteByteSD sdi1;
        Test::ByteByteSD sdi2;

        Test::ByteS si1;
        Test::ByteS si2;
        Test::ByteS si3;

        si1.push_back(Ice::Byte(0x01));
        si1.push_back(Ice::Byte(0x11));
        si2.push_back(Ice::Byte(0x12));
        si3.push_back(Ice::Byte(0xf2));
        si3.push_back(Ice::Byte(0xf3));

        sdi1[Ice::Byte(0x01)] = si1;
        sdi1[Ice::Byte(0x22)] = si2;
        sdi2[Ice::Byte(0xf1)] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opByteByteSD(sdi1, sdi2,
                                    [=](const Test::ByteByteSD& p1, const Test::ByteByteSD& p2)
                                        {
                                            cb->opByteByteSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::BoolBoolSD sdi1;
        Test::BoolBoolSD sdi2;

        Test::BoolS si1;
        Test::BoolS si2;

        si1.push_back(true);
        si1.push_back(false);
        si2.push_back(false);
        si2.push_back(true);
        si2.push_back(true);

        sdi1[false] = si1;
        sdi1[true] = si2;
        sdi2[false] = si1;

        CallbackPtr cb = new Callback;
        p->begin_opBoolBoolSD(sdi1, sdi2,
                                    [=](const Test::BoolBoolSD& p1, const Test::BoolBoolSD& p2)
                                        {
                                            cb->opBoolBoolSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::ShortShortSD sdi1;
        Test::ShortShortSD sdi2;

        Test::ShortS si1;
        Test::ShortS si2;
        Test::ShortS si3;

        si1.push_back(1);
        si1.push_back(2);
        si1.push_back(3);
        si2.push_back(4);
        si2.push_back(5);
        si3.push_back(6);
        si3.push_back(7);

        sdi1[1] = si1;
        sdi1[2] = si2;
        sdi2[4] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opShortShortSD(sdi1, sdi2,
                                    [=](const Test::ShortShortSD& p1, const Test::ShortShortSD& p2)
                                        {
                                            cb->opShortShortSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::IntIntSD sdi1;
        Test::IntIntSD sdi2;

        Test::IntS si1;
        Test::IntS si2;
        Test::IntS si3;

        si1.push_back(100);
        si1.push_back(200);
        si1.push_back(300);
        si2.push_back(400);
        si2.push_back(500);
        si3.push_back(600);
        si3.push_back(700);

        sdi1[100] = si1;
        sdi1[200] = si2;
        sdi2[400] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opIntIntSD(sdi1, sdi2,
                                    [=](const Test::IntIntSD& p1, const Test::IntIntSD& p2)
                                        {
                                            cb->opIntIntSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::LongLongSD sdi1;
        Test::LongLongSD sdi2;

        Test::LongS si1;
        Test::LongS si2;
        Test::LongS si3;

        si1.push_back(999999110);
        si1.push_back(999999111);
        si1.push_back(999999110);
        si2.push_back(999999120);
        si2.push_back(999999130);
        si3.push_back(999999110);
        si3.push_back(999999120);

        sdi1[999999990] = si1;
        sdi1[999999991] = si2;
        sdi2[999999992] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opLongLongSD(sdi1, sdi2,
                                    [=](const Test::LongLongSD& p1, const Test::LongLongSD& p2)
                                        {
                                            cb->opLongLongSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::StringFloatSD sdi1;
        Test::StringFloatSD sdi2;

        Test::FloatS si1;
        Test::FloatS si2;
        Test::FloatS si3;

        si1.push_back(Ice::Float(-1.1));
        si1.push_back(Ice::Float(123123.2));
        si1.push_back(Ice::Float(100.0));
        si2.push_back(Ice::Float(42.24));
        si2.push_back(Ice::Float(-1.61));
        si3.push_back(Ice::Float(-3.14));
        si3.push_back(Ice::Float(3.14));

        sdi1["abc"] = si1;
        sdi1["ABC"] = si2;
        sdi2["aBc"] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opStringFloatSD(sdi1, sdi2,
                                    [=](const Test::StringFloatSD& p1, const Test::StringFloatSD& p2)
                                        {
                                            cb->opStringFloatSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::StringDoubleSD sdi1;
        Test::StringDoubleSD sdi2;

        Test::DoubleS si1;
        Test::DoubleS si2;
        Test::DoubleS si3;

        si1.push_back(Ice::Double(1.1E10));
        si1.push_back(Ice::Double(1.2E10));
        si1.push_back(Ice::Double(1.3E10));
        si2.push_back(Ice::Double(1.4E10));
        si2.push_back(Ice::Double(1.5E10));
        si3.push_back(Ice::Double(1.6E10));
        si3.push_back(Ice::Double(1.7E10));

        sdi1["Hello!!"] = si1;
        sdi1["Goodbye"] = si2;
        sdi2[""] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opStringDoubleSD(sdi1, sdi2,
                                    [=](const Test::StringDoubleSD& p1, const Test::StringDoubleSD& p2)
                                        {
                                            cb->opStringDoubleSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();
    }

    {
        Test::StringStringSD sdi1;
        Test::StringStringSD sdi2;

        Test::StringS si1;
        Test::StringS si2;
        Test::StringS si3;

        si1.push_back("abc");
        si1.push_back("de");
        si1.push_back("fghi");

        si2.push_back("xyz");
        si2.push_back("or");

        si3.push_back("and");
        si3.push_back("xor");

        sdi1["abc"] = si1;
        sdi1["def"] = si2;
        sdi2["ghi"] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opStringStringSD(sdi1, sdi2,
                                    [=](const Test::StringStringSD& p1, const Test::StringStringSD& p2)
                                        {
                                            cb->opStringStringSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
        cb->check();

    }

    {
        Test::MyEnumMyEnumSD sdi1;
        Test::MyEnumMyEnumSD sdi2;

        Test::MyEnumS si1;
        Test::MyEnumS si2;
        Test::MyEnumS si3;

        si1.push_back(Test::enum1);
        si1.push_back(Test::enum1);
        si1.push_back(Test::enum2);
        si2.push_back(Test::enum1);
        si2.push_back(Test::enum2);
        si3.push_back(Test::enum3);
        si3.push_back(Test::enum3);

        sdi1[Test::enum3] = si1;
        sdi1[Test::enum2] = si2;
        sdi2[Test::enum1] = si3;

        CallbackPtr cb = new Callback;
        p->begin_opMyEnumMyEnumSD(sdi1, sdi2,
                                    [=](const Test::MyEnumMyEnumSD& p1, const Test::MyEnumMyEnumSD& p2)
                                        {
                                            cb->opMyEnumMyEnumSD(p1, p2);
                                        },
                                    [=](const Ice::Exception& ex)
                                        {
                                            cb->exCB(ex);
                                        });
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
            CallbackPtr cb = new Callback;
            p->begin_opIntS(s,
                            [=](const Test::IntS& p1)
                                {
                                    cb->opIntS(p1);
                                },
                            [=](const Ice::Exception& ex)
                                {
                                    cb->exCB(ex);
                                });
            cb->check();
        }
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        CallbackPtr cb = new Callback;
        p->begin_opDoubleMarshaling(d, ds,
                                    [=](){ cb->opDoubleMarshaling(); },
                                    [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Test::Callback_MyClass_opIdempotentPtr callback =
            Test::newCallback_MyClass_opIdempotent(cb, &Callback::opIdempotent, &Callback::exCB);
        p->begin_opIdempotent([=](){ cb->opIdempotent(); },
                              [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opNonmutating([=](){ cb->opNonmutating(); },
                               [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }

    {
        Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(p);
        test(derived);
        CallbackPtr cb = new Callback;
        derived->begin_opDerived([=](){ cb->opDerived(); },
                                 [=](const Ice::Exception& ex){ cb->exCB(ex); });
        cb->check();
    }
#endif
}
