// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
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
using namespace Test;
namespace
{

class CallbackBase
#ifndef ICE_CPP11_MAPPING
: public Ice::LocalObject
#endif
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
        test(e == ICE_ENUM(Test::MyEnum, enum2));
        test(r == ICE_ENUM(Test::MyEnum, enum3));
        called();
    }

    void opMyClass(const Test::MyClassPrxPtr& r, const Test::MyClassPrxPtr& c1, const Test::MyClassPrxPtr& c2)
    {
        test(c1->ice_getIdentity() == Ice::stringToIdentity("test"));
        test(c2->ice_getIdentity() == Ice::stringToIdentity("noSuchIdentity"));
        test(r->ice_getIdentity() == Ice::stringToIdentity("test"));

        //
        // We can't do the callbacks below in connection serialization mode.
        //
        if(_communicator->getProperties()->getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
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
        test(rso.e == ICE_ENUM(Test::MyEnum, enum2));
        test(rso.s.s == "def");
        test(so.e == ICE_ENUM(Test::MyEnum, enum3));
        test(so.s.s == "a new string");

        //
        // We can't do the callbacks below in connection serialization mode.
        //
        if(_communicator->getProperties()->getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
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
        di1["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di1[""] = ICE_ENUM(Test::MyEnum, enum2);
        test(_do == di1);
        test(ro.size() == 4);
        test(ro.find("abc") != ro.end());
        test(ro.find("abc")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro.find("qwerty") != ro.end());
        test(ro.find("qwerty")->second == ICE_ENUM(Test::MyEnum, enum3));
        test(ro.find("") != ro.end());
        test(ro.find("")->second == ICE_ENUM(Test::MyEnum, enum2));
        test(ro.find("Hello!!") != ro.end());
        test(ro.find("Hello!!")->second == ICE_ENUM(Test::MyEnum, enum2));
        called();
    }

    void opMyStructMyEnumD(const Test::MyStructMyEnumD& ro, const Test::MyStructMyEnumD& _do)
    {
        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di1[ms12] = ICE_ENUM(Test::MyEnum, enum2);
        test(_do == di1);
        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };
        test(ro.size() == 4);
        test(ro.find(ms11) != ro.end());
        test(ro.find(ms11)->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro.find(ms12) != ro.end());
        test(ro.find(ms12)->second == ICE_ENUM(Test::MyEnum, enum2));
        test(ro.find(ms22) != ro.end());
        test(ro.find(ms22)->second == ICE_ENUM(Test::MyEnum, enum3));
        test(ro.find(ms23) != ro.end());
        test(ro.find(ms23)->second == ICE_ENUM(Test::MyEnum, enum2));
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
        test(ro[0].find("abc")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro[0].find("qwerty") != ro[0].end());
        test(ro[0].find("qwerty")->second == ICE_ENUM(Test::MyEnum, enum3));
        test(ro[0].find("Hello!!") != ro[0].end());
        test(ro[0].find("Hello!!")->second == ICE_ENUM(Test::MyEnum, enum2));
        test(ro[1].size() == 2);
        test(ro[1].find("abc") != ro[1].end());
        test(ro[1].find("abc")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro[1].find("") != ro[1].end());
        test(ro[1].find("")->second == ICE_ENUM(Test::MyEnum, enum2));
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find("Goodbye") != _do[0].end());
        test(_do[0].find("Goodbye")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(_do[1].size() == 2);
        test(_do[1].find("abc") != _do[1].end());
        test(_do[1].find("abc")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(_do[1].find("") != _do[1].end());
        test(_do[1].find("")->second == ICE_ENUM(Test::MyEnum, enum2));
        test(_do[2].size() == 3);
        test(_do[2].find("abc") != _do[2].end());
        test(_do[2].find("abc")->second == ICE_ENUM(Test::MyEnum, enum1));
        test(_do[2].find("qwerty") != _do[2].end());
        test(_do[2].find("qwerty")->second == ICE_ENUM(Test::MyEnum, enum3));
        test(_do[2].find("Hello!!") != _do[2].end());
        test(_do[2].find("Hello!!")->second == ICE_ENUM(Test::MyEnum, enum2));
        called();
    }

    void opMyEnumStringDS(const Test::MyEnumStringDS& ro, const Test::MyEnumStringDS& _do)
    {
        test(ro.size() == 2);
        test(ro[0].size() == 2);
        test(ro[0].find(ICE_ENUM(Test::MyEnum, enum2)) != ro[0].end());
        test(ro[0].find(ICE_ENUM(Test::MyEnum, enum2))->second == "Hello!!");
        test(ro[0].find(ICE_ENUM(Test::MyEnum, enum3)) != ro[0].end());
        test(ro[0].find(ICE_ENUM(Test::MyEnum, enum3))->second == "qwerty");
        test(ro[1].size() == 1);
        test(ro[1].find(ICE_ENUM(Test::MyEnum, enum1)) != ro[1].end());
        test(ro[1].find(ICE_ENUM(Test::MyEnum, enum1))->second == "abc");
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(ICE_ENUM(Test::MyEnum, enum1)) != _do[0].end());
        test(_do[0].find(ICE_ENUM(Test::MyEnum, enum1))->second == "Goodbye");
        test(_do[1].size() == 1);
        test(_do[1].find(ICE_ENUM(Test::MyEnum, enum1)) != _do[1].end());
        test(_do[1].find(ICE_ENUM(Test::MyEnum, enum1))->second == "abc");
        test(_do[2].size() == 2);
        test(_do[2].find(ICE_ENUM(Test::MyEnum, enum2)) != _do[2].end());
        test(_do[2].find(ICE_ENUM(Test::MyEnum, enum2))->second == "Hello!!");
        test(_do[2].find(ICE_ENUM(Test::MyEnum, enum3)) != _do[2].end());
        test(_do[2].find(ICE_ENUM(Test::MyEnum, enum3))->second == "qwerty");
        called();
    }

    void opMyStructMyEnumDS(const Test::MyStructMyEnumDS& ro, const Test::MyStructMyEnumDS& _do)
    {
        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };

        test(ro.size() == 2);
        test(ro[0].size() == 3);
        test(ro[0].find(ms11) != ro[0].end());
        test(ro[0].find(ms11)->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro[0].find(ms22) != ro[0].end());
        test(ro[0].find(ms22)->second == ICE_ENUM(Test::MyEnum, enum3));
        test(ro[0].find(ms23) != ro[0].end());
        test(ro[0].find(ms23)->second == ICE_ENUM(Test::MyEnum, enum2));
        test(ro[1].size() == 2);
        test(ro[1].find(ms11) != ro[1].end());
        test(ro[1].find(ms11)->second == ICE_ENUM(Test::MyEnum, enum1));
        test(ro[1].find(ms12) != ro[1].end());
        test(ro[1].find(ms12)->second == ICE_ENUM(Test::MyEnum, enum2));
        test(_do.size() == 3);
        test(_do[0].size() == 1);
        test(_do[0].find(ms23) != _do[0].end());
        test(_do[0].find(ms23)->second == ICE_ENUM(Test::MyEnum, enum3));
        test(_do[1].size() == 2);
        test(_do[1].find(ms11) != _do[1].end());
        test(_do[1].find(ms11)->second == ICE_ENUM(Test::MyEnum, enum1));
        test(_do[1].find(ms12) != _do[1].end());
        test(_do[1].find(ms12)->second == ICE_ENUM(Test::MyEnum, enum2));
        test(_do[2].size() == 3);
        test(_do[2].find(ms11) != _do[2].end());
        test(_do[2].find(ms11)->second == ICE_ENUM(Test::MyEnum, enum1));
        test(_do[2].find(ms22) != _do[2].end());
        test(_do[2].find(ms22)->second == ICE_ENUM(Test::MyEnum, enum3));
        test(_do[2].find(ms23) != _do[2].end());
        test(_do[2].find(ms23)->second == ICE_ENUM(Test::MyEnum, enum2));
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
        test(_do.find(ICE_ENUM(Test::MyEnum, enum1)) != _do.end());
        test(_do.find(ICE_ENUM(Test::MyEnum, enum1))->second.size() == 2);
        test(_do.find(ICE_ENUM(Test::MyEnum, enum1))->second[0] == ICE_ENUM(Test::MyEnum, enum3));
        test(_do.find(ICE_ENUM(Test::MyEnum, enum1))->second[1] == ICE_ENUM(Test::MyEnum, enum3));
        test(ro.size() == 3);
        test(ro.find(ICE_ENUM(Test::MyEnum, enum3)) != ro.end());
        test(ro.find(ICE_ENUM(Test::MyEnum, enum3))->second.size() == 3);
        test(ro.find(ICE_ENUM(Test::MyEnum, enum3))->second[0] == ICE_ENUM(Test::MyEnum, enum1));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum3))->second[1] == ICE_ENUM(Test::MyEnum, enum1));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum3))->second[2] == ICE_ENUM(Test::MyEnum, enum2));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum2)) != ro.end());
        test(ro.find(ICE_ENUM(Test::MyEnum, enum2))->second.size() == 2);
        test(ro.find(ICE_ENUM(Test::MyEnum, enum2))->second[0] == ICE_ENUM(Test::MyEnum, enum1));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum2))->second[1] == ICE_ENUM(Test::MyEnum, enum2));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum1)) != ro.end());
        test(ro.find(ICE_ENUM(Test::MyEnum, enum1))->second.size() == 2);
        test(ro.find(ICE_ENUM(Test::MyEnum, enum1))->second[0] == ICE_ENUM(Test::MyEnum, enum3));
        test(ro.find(ICE_ENUM(Test::MyEnum, enum1))->second[1] == ICE_ENUM(Test::MyEnum, enum3));
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
ICE_DEFINE_PTR(CallbackPtr, Callback);

}

#ifdef ICE_CPP11_MAPPING
function<void(exception_ptr)>
makeExceptionClosure(CallbackPtr& cb)
{
    return [&](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::Exception& ex)
            {
                cb->exCB(ex);
            }
        };
}
#endif

void
twowaysAMI(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrxPtr& p)
{
    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->ice_pingAsync(
            [&]()
            {
                cb->ping();
            },
            makeExceptionClosure(cb));
#else
        Ice::Callback_Object_ice_pingPtr callback = Ice::newCallback_Object_ice_ping(cb,
                                                                                     &Callback::ping,
                                                                                     &Callback::exCB);
        p->begin_ice_ping(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->ice_isAAsync(
            Test::MyClass::ice_staticId(),
            [&](bool v)
            {
                cb->isA(v);
            },
            makeExceptionClosure(cb));
#else
        Ice::Callback_Object_ice_isAPtr callback = Ice::newCallback_Object_ice_isA(cb,
                                                                                   &Callback::isA,
                                                                                   &Callback::exCB);
        p->begin_ice_isA(Test::MyClass::ice_staticId(), callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->ice_idAsync(
            [&](string id)
            {
                cb->id(move(id));
            },
            makeExceptionClosure(cb));
#else
        Ice::Callback_Object_ice_idPtr callback = Ice::newCallback_Object_ice_id(cb,
                                                                                 &Callback::id,
                                                                                 &Callback::exCB);
        p->begin_ice_id(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->ice_idsAsync(
            [&](vector<string> ids)
            {
                cb->ids(move(ids));
            },
            makeExceptionClosure(cb));

#else
        Ice::Callback_Object_ice_idsPtr callback = Ice::newCallback_Object_ice_ids(cb,
                                                                                   &Callback::ids,
                                                                                   &Callback::exCB);
        p->begin_ice_ids(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opVoidAsync(
            [&]()
            {
                cb->opVoid();
            },
            makeExceptionClosure(cb));

#else
        Test::Callback_MyClass_opVoidPtr callback = Test::newCallback_MyClass_opVoid(cb,
                                                                                     &Callback::opVoid,
                                                                                     &Callback::exCB);
        p->begin_opVoid(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteAsync(Ice::Byte(0xff), Ice::Byte(0x0f),
            [&](Ice::Byte b1, Ice::Byte b2)
            {
                cb->opByte(b1, b2);
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opBytePtr callback = Test::newCallback_MyClass_opByte(cb,
                                                                                     &Callback::opByte,
                                                                                     &Callback::exCB);
        p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opBoolAsync(true, false,
            [&](bool b1, bool b2)
            {
                cb->opBool(b1, b2);
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opBoolPtr callback = Test::newCallback_MyClass_opBool(cb,
                                                                                     &Callback::opBool,
                                                                                     &Callback::exCB);
        p->begin_opBool(true, false, callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortIntLongAsync(10, 11, 12,
            [&](long long int l1, short s1P, int i1, long long int l2)
            {
                cb->opShortIntLong(l1, s1P, i1, l2);
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortIntLongPtr callback =
            Test::newCallback_MyClass_opShortIntLong(cb, &Callback::opShortIntLong, &Callback::exCB);
        p->begin_opShortIntLong(10, 11, 12, callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opFloatDoubleAsync(3.14f, 1.1E10,
            [&](double d1, float f1, double d2)
            {
                cb->opFloatDouble(d1, f1, d2);
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opFloatDoublePtr callback =
            Test::newCallback_MyClass_opFloatDouble(cb, &Callback::opFloatDouble, &Callback::exCB);
        p->begin_opFloatDouble(Ice::Float(3.14), Ice::Double(1.1E10), callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringAsync("hello", "world",
            [&](string s1P, string s2P)
            {
                cb->opString(move(s1P), move(s2P));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringPtr callback = Test::newCallback_MyClass_opString(cb,
                                                                                         &Callback::opString,
                                                                                         &Callback::exCB);
        p->begin_opString("hello", "world", callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opMyEnumAsync(MyEnum::enum2,
            [&](MyEnum e1, MyEnum e2)
            {
                cb->opMyEnum(e1, e2);
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyEnumPtr callback = Test::newCallback_MyClass_opMyEnum(cb,
                                                                                         &Callback::opMyEnum,
                                                                                         &Callback::exCB);
        p->begin_opMyEnum(Test::enum2, callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback, communicator);
#ifdef ICE_CPP11_MAPPING
        p->opMyClassAsync(p,
            [&](shared_ptr<MyClassPrx> c1, shared_ptr<MyClassPrx> c2, shared_ptr<MyClassPrx> c3)
            {
                cb->opMyClass(move(c1), move(c2), move(c3));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyClassPtr callback = Test::newCallback_MyClass_opMyClass(cb,
                                                                                           &Callback::opMyClass,
                                                                                           &Callback::exCB);
        p->begin_opMyClass(p, callback);
#endif
        cb->check();
    }

    {
        Test::Structure si1;
        si1.p = p;
        si1.e = ICE_ENUM(Test::MyEnum, enum3);
        si1.s.s = "abc";
        Test::Structure si2;
        si2.p = 0;
        si2.e = ICE_ENUM(Test::MyEnum, enum2);
        si2.s.s = "def";

        CallbackPtr cb = ICE_MAKE_SHARED(Callback, communicator);
#ifdef ICE_CPP11_MAPPING
        p->opStructAsync(si1, si2,
            [&](Test::Structure si3, Test::Structure si4)
            {
                cb->opStruct(move(si3), move(si4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStructPtr callback = Test::newCallback_MyClass_opStruct(cb,
                                                                                         &Callback::opStruct,
                                                                                         &Callback::exCB);
        p->begin_opStruct(si1, si2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteSAsync(bsi1, bsi2,
            [&](Test::ByteS bsi3, Test::ByteS bsi4)
            {
                cb->opByteS(move(bsi3), move(bsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opByteSPtr callback = Test::newCallback_MyClass_opByteS(cb,
                                                                                       &Callback::opByteS,
                                                                                       &Callback::exCB);
        p->begin_opByteS(bsi1, bsi2, callback);
#endif
        cb->check();
    }

    {
        Test::BoolS bsi1;
        Test::BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opBoolSAsync(bsi1, bsi2,
            [&](Test::BoolS bsi3, Test::BoolS bsi4)
            {
                cb->opBoolS(move(bsi3), move(bsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opBoolSPtr callback = Test::newCallback_MyClass_opBoolS(cb,
                                                                                       &Callback::opBoolS,
                                                                                       &Callback::exCB);
        p->begin_opBoolS(bsi1, bsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortIntLongSAsync(ssi, isi, lsi,
            [&](Test::LongS lsi1, Test::ShortS ssi1, Test::IntS isi1, Test::LongS lsi2)
            {
                cb->opShortIntLongS(move(lsi1), move(ssi1), move(isi1), move(lsi2));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortIntLongSPtr callback =
            Test::newCallback_MyClass_opShortIntLongS(cb, &Callback::opShortIntLongS, &Callback::exCB);
        p->begin_opShortIntLongS(ssi, isi, lsi, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opFloatDoubleSAsync(fsi, dsi,
            [&](Test::DoubleS dsi1, Test::FloatS fsi1, Test::DoubleS dsi2)
            {
                cb->opFloatDoubleS(move(dsi1), move(fsi1), move(dsi2));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opFloatDoubleSPtr callback =
            Test::newCallback_MyClass_opFloatDoubleS(cb, &Callback::opFloatDoubleS, &Callback::exCB);
        p->begin_opFloatDoubleS(fsi, dsi, callback);
#endif
        cb->check();
    }

    {
        Test::StringS ssi1;
        Test::StringS ssi2;

        ssi1.push_back("abc");
        ssi1.push_back("de");
        ssi1.push_back("fghi");

        ssi2.push_back("xyz");

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringSAsync(ssi1, ssi2,
            [&](Test::StringS ssi3, Test::StringS ssi4)
            {
                cb->opStringS(move(ssi3), move(ssi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringSPtr callback = Test::newCallback_MyClass_opStringS(cb,
                                                                                           &Callback::opStringS,
                                                                                           &Callback::exCB);
        p->begin_opStringS(ssi1, ssi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteSSAsync(bsi1, bsi2,
            [&](Test::ByteSS bsi3, Test::ByteSS bsi4)
            {
                cb->opByteSS(move(bsi3), move(bsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opByteSSPtr callback = Test::newCallback_MyClass_opByteSS(cb,
                                                                                         &Callback::opByteSS,
                                                                                         &Callback::exCB);
        p->begin_opByteSS(bsi1, bsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opBoolSSAsync(bsi1, bsi2,
            [&](Test::BoolSS bsi3, Test::BoolSS bsi4)
            {
                cb->opBoolSS(move(bsi3), move(bsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opBoolSSPtr callback = Test::newCallback_MyClass_opBoolSS(cb,
                                                                                         &Callback::opBoolSS,
                                                                                         &Callback::exCB);
        p->begin_opBoolSS(bsi1, bsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortIntLongSSAsync(ssi, isi, lsi,
            [&](Test::LongSS lsi1, Test::ShortSS ssi1, Test::IntSS isi1, Test::LongSS lsi2)
            {
                cb->opShortIntLongSS(move(lsi1), move(ssi1), move(isi1), move(lsi2));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortIntLongSSPtr callback = Test::newCallback_MyClass_opShortIntLongSS(cb,
                                                                                         &Callback::opShortIntLongSS,
                                                                                         &Callback::exCB);
        p->begin_opShortIntLongSS(ssi, isi, lsi, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opFloatDoubleSSAsync(fsi, dsi,
            [&](Test::DoubleSS dsi1, Test::FloatSS fsi1, Test::DoubleSS dsi2)
            {
                cb->opFloatDoubleSS(move(dsi1), move(fsi1), move(dsi2));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opFloatDoubleSSPtr callback =
            Test::newCallback_MyClass_opFloatDoubleSS(cb, &Callback::opFloatDoubleSS, &Callback::exCB);
        p->begin_opFloatDoubleSS(fsi, dsi, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringSSAsync(ssi1, ssi2,
            [&](Test::StringSS ssi3, Test::StringSS ssi4)
            {
                cb->opStringSS(move(ssi3), move(ssi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringSSPtr callback =
            Test::newCallback_MyClass_opStringSS(cb, &Callback::opStringSS, &Callback::exCB);
        p->begin_opStringSS(ssi1, ssi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteBoolDAsync(di1, di2,
            [&](Test::ByteBoolD di3, Test::ByteBoolD di4)
            {
                cb->opByteBoolD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opByteBoolDPtr callback =
            Test::newCallback_MyClass_opByteBoolD(cb, &Callback::opByteBoolD, &Callback::exCB);
        p->begin_opByteBoolD(di1, di2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortIntDAsync(di1, di2,
            [&](Test::ShortIntD di3, Test::ShortIntD di4)
            {
                cb->opShortIntD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortIntDPtr callback =
            Test::newCallback_MyClass_opShortIntD(cb, &Callback::opShortIntD, &Callback::exCB);
        p->begin_opShortIntD(di1, di2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opLongFloatDAsync(di1, di2,
            [&](Test::LongFloatD di3, Test::LongFloatD di4)
            {
                cb->opLongFloatD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opLongFloatDPtr callback =
            Test::newCallback_MyClass_opLongFloatD(cb, &Callback::opLongFloatD, &Callback::exCB);
        p->begin_opLongFloatD(di1, di2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringStringDAsync(di1, di2,
            [&](Test::StringStringD di3, Test::StringStringD di4)
            {
                cb->opStringStringD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringStringDPtr callback =
            Test::newCallback_MyClass_opStringStringD(cb, &Callback::opStringStringD, &Callback::exCB);
        p->begin_opStringStringD(di1, di2, callback);
#endif
        cb->check();
    }

    {
        Test::StringMyEnumD di1;
        di1["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di1[""] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di2;
        di2["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di2["qwerty"] = ICE_ENUM(Test::MyEnum, enum3);
        di2["Hello!!"] = ICE_ENUM(Test::MyEnum, enum2);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringMyEnumDAsync(di1, di2,
            [&](Test::StringMyEnumD di3, Test::StringMyEnumD di4)
            {
                cb->opStringMyEnumD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringMyEnumDPtr callback =
            Test::newCallback_MyClass_opStringMyEnumD(cb, &Callback::opStringMyEnumD, &Callback::exCB);
        p->begin_opStringMyEnumD(di1, di2, callback);
#endif
        cb->check();
    }

    {
        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di1[ms12] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };
        Test::MyStructMyEnumD di2;
        di2[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di2[ms22] = ICE_ENUM(Test::MyEnum, enum3);
        di2[ms23] = ICE_ENUM(Test::MyEnum, enum2);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opMyStructMyEnumDAsync(di1, di2,
            [&](Test::MyStructMyEnumD di3, Test::MyStructMyEnumD di4)
            {
                cb->opMyStructMyEnumD(move(di3), move(di4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyStructMyEnumDPtr callback =
            Test::newCallback_MyClass_opMyStructMyEnumD(cb, &Callback::opMyStructMyEnumD, &Callback::exCB);
        p->begin_opMyStructMyEnumD(di1, di2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteBoolDSAsync(dsi1, dsi2,
            [&](Test::ByteBoolDS dsi3, Test::ByteBoolDS dsi4)
            {
                cb->opByteBoolDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opByteBoolDSPtr callback =
            Test::newCallback_MyClass_opByteBoolDS(cb, &Callback::opByteBoolDS, &Callback::exCB);
        p->begin_opByteBoolDS(dsi1, dsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortIntDSAsync(dsi1, dsi2,
            [&](Test::ShortIntDS dsi3, Test::ShortIntDS dsi4)
            {
                cb->opShortIntDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortIntDSPtr callback =
            Test::newCallback_MyClass_opShortIntDS(cb, &Callback::opShortIntDS, &Callback::exCB);
        p->begin_opShortIntDS(dsi1, dsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opLongFloatDSAsync(dsi1, dsi2,
            [&](Test::LongFloatDS dsi3, Test::LongFloatDS dsi4)
            {
                cb->opLongFloatDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opLongFloatDSPtr callback =
            Test::newCallback_MyClass_opLongFloatDS(cb, &Callback::opLongFloatDS, &Callback::exCB);
        p->begin_opLongFloatDS(dsi1, dsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringStringDSAsync(dsi1, dsi2,
            [&](Test::StringStringDS dsi3, Test::StringStringDS dsi4)
            {
                cb->opStringStringDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));

#else
        Test::Callback_MyClass_opStringStringDSPtr callback =
            Test::newCallback_MyClass_opStringStringDS(cb, &Callback::opStringStringDS, &Callback::exCB);
        p->begin_opStringStringDS(dsi1, dsi2, callback);
#endif
        cb->check();
    }

    {
        Test::StringMyEnumDS dsi1;
        dsi1.resize(2);
        Test::StringMyEnumDS dsi2;
        dsi2.resize(1);

        Test::StringMyEnumD di1;
        di1["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di1[""] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di2;
        di2["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di2["qwerty"] = ICE_ENUM(Test::MyEnum, enum3);
        di2["Hello!!"] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di3;
        di3["Goodbye"] = ICE_ENUM(Test::MyEnum, enum1);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringMyEnumDSAsync(dsi1, dsi2,
            [&](Test::StringMyEnumDS dsi3, Test::StringMyEnumDS dsi4)
            {
                cb->opStringMyEnumDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringMyEnumDSPtr callback =
            Test::newCallback_MyClass_opStringMyEnumDS(cb, &Callback::opStringMyEnumDS, &Callback::exCB);
        p->begin_opStringMyEnumDS(dsi1, dsi2, callback);
#endif
        cb->check();
    }

    {
        Test::MyEnumStringDS dsi1;
        dsi1.resize(2);
        Test::MyEnumStringDS dsi2;
        dsi2.resize(1);

        Test::MyEnumStringD di1;
        di1[ICE_ENUM(Test::MyEnum, enum1)] = "abc";
        Test::MyEnumStringD di2;
        di2[ICE_ENUM(Test::MyEnum, enum2)] = "Hello!!";
        di2[ICE_ENUM(Test::MyEnum, enum3)] = "qwerty";
        Test::MyEnumStringD di3;
        di3[ICE_ENUM(Test::MyEnum, enum1)] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opMyEnumStringDSAsync(dsi1, dsi2,
            [&](Test::MyEnumStringDS dsi3, Test::MyEnumStringDS dsi4)
            {
                cb->opMyEnumStringDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyEnumStringDSPtr callback =
            Test::newCallback_MyClass_opMyEnumStringDS(cb, &Callback::opMyEnumStringDS, &Callback::exCB);
        p->begin_opMyEnumStringDS(dsi1, dsi2, callback);
#endif
        cb->check();
    }

    {
        Test::MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        Test::MyStructMyEnumDS dsi2;
        dsi2.resize(1);

        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di1[ms12] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };
        Test::MyStructMyEnumD di2;
        di2[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di2[ms22] = ICE_ENUM(Test::MyEnum, enum3);
        di2[ms23] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStructMyEnumD di3;
        di3[ms23] = ICE_ENUM(Test::MyEnum, enum3);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opMyStructMyEnumDSAsync(dsi1, dsi2,
            [&](Test::MyStructMyEnumDS dsi3, MyStructMyEnumDS dsi4)
            {
                cb->opMyStructMyEnumDS(move(dsi3), move(dsi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyStructMyEnumDSPtr callback =
            Test::newCallback_MyClass_opMyStructMyEnumDS(cb, &Callback::opMyStructMyEnumDS, &Callback::exCB);
        p->begin_opMyStructMyEnumDS(dsi1, dsi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opByteByteSDAsync(sdi1, sdi2,
            [&](Test::ByteByteSD sdi3, Test::ByteByteSD sdi4)
            {
                cb->opByteByteSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opByteByteSDPtr callback =
            Test::newCallback_MyClass_opByteByteSD(cb, &Callback::opByteByteSD, &Callback::exCB);
        p->begin_opByteByteSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opBoolBoolSDAsync(sdi1, sdi2,
            [&](Test::BoolBoolSD sdi3, Test::BoolBoolSD sdi4)
            {
                cb->opBoolBoolSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opBoolBoolSDPtr callback =
            Test::newCallback_MyClass_opBoolBoolSD(cb, &Callback::opBoolBoolSD, &Callback::exCB);
        p->begin_opBoolBoolSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opShortShortSDAsync(sdi1, sdi2,
            [&](Test::ShortShortSD sdi3, Test::ShortShortSD sdi4)
            {
                cb->opShortShortSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opShortShortSDPtr callback =
            Test::newCallback_MyClass_opShortShortSD(cb, &Callback::opShortShortSD, &Callback::exCB);
        p->begin_opShortShortSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opIntIntSDAsync(sdi1, sdi2,
            [&](Test::IntIntSD sdi3, Test::IntIntSD sdi4)
            {
                cb->opIntIntSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opIntIntSDPtr callback =
            Test::newCallback_MyClass_opIntIntSD(cb, &Callback::opIntIntSD, &Callback::exCB);
        p->begin_opIntIntSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opLongLongSDAsync(sdi1, sdi2,
            [&](Test::LongLongSD sdi3, Test::LongLongSD sdi4)
            {
                cb->opLongLongSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opLongLongSDPtr callback =
            Test::newCallback_MyClass_opLongLongSD(cb, &Callback::opLongLongSD, &Callback::exCB);
        p->begin_opLongLongSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringFloatSDAsync(sdi1, sdi2,
            [&](Test::StringFloatSD sdi3, Test::StringFloatSD sdi4)
            {
                cb->opStringFloatSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringFloatSDPtr callback =
            Test::newCallback_MyClass_opStringFloatSD(cb, &Callback::opStringFloatSD, &Callback::exCB);
        p->begin_opStringFloatSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringDoubleSDAsync(sdi1, sdi2,
            [&](Test::StringDoubleSD sdi3, Test::StringDoubleSD sdi4)
            {
                cb->opStringDoubleSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringDoubleSDPtr callback =
            Test::newCallback_MyClass_opStringDoubleSD(cb, &Callback::opStringDoubleSD, &Callback::exCB);
        p->begin_opStringDoubleSD(sdi1, sdi2, callback);
#endif
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opStringStringSDAsync(sdi1, sdi2,
            [&](Test::StringStringSD sdi3, Test::StringStringSD sdi4)
            {
                cb->opStringStringSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opStringStringSDPtr callback =
            Test::newCallback_MyClass_opStringStringSD(cb, &Callback::opStringStringSD, &Callback::exCB);
        p->begin_opStringStringSD(sdi1, sdi2, callback);
#endif
        cb->check();
    }

    {
        Test::MyEnumMyEnumSD sdi1;
        Test::MyEnumMyEnumSD sdi2;

        Test::MyEnumS si1;
        Test::MyEnumS si2;
        Test::MyEnumS si3;

        si1.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si1.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si1.push_back(ICE_ENUM(Test::MyEnum, enum2));
        si2.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si2.push_back(ICE_ENUM(Test::MyEnum, enum2));
        si3.push_back(ICE_ENUM(Test::MyEnum, enum3));
        si3.push_back(ICE_ENUM(Test::MyEnum, enum3));

        sdi1[ICE_ENUM(Test::MyEnum, enum3)] = si1;
        sdi1[ICE_ENUM(Test::MyEnum, enum2)] = si2;
        sdi2[ICE_ENUM(Test::MyEnum, enum1)] = si3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opMyEnumMyEnumSDAsync(sdi1, sdi2,
            [&](Test::MyEnumMyEnumSD sdi3, Test::MyEnumMyEnumSD sdi4)
            {
                cb->opMyEnumMyEnumSD(move(sdi3), move(sdi4));
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opMyEnumMyEnumSDPtr callback =
            Test::newCallback_MyClass_opMyEnumMyEnumSD(cb, &Callback::opMyEnumMyEnumSD, &Callback::exCB);
        p->begin_opMyEnumMyEnumSD(sdi1, sdi2, callback);
#endif
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
            CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
            p->opIntSAsync(s,
                [&](Test::IntS s1P)
                {
                    cb->opIntS(s1P);
                },
                makeExceptionClosure(cb));
#else
            Test::Callback_MyClass_opIntSPtr callback =
                Test::newCallback_MyClass_opIntS(cb, &Callback::opIntS, &Callback::exCB);
            p->begin_opIntS(s, callback);
#endif
            cb->check();
        }
    }

    {
        {
            Ice::Context ctx;
            ctx["one"] = "ONE";
            ctx["two"] = "TWO";
            ctx["three"] = "THREE";
            {
                test(p->ice_getContext().empty());
#ifdef ICE_CPP11_MAPPING
                promise<void> prom;
                p->opContextAsync(
                    [&](Ice::Context c)
                    {
                        test(c != ctx);
                        prom.set_value();
                    },
                    [](exception_ptr)
                    {
                        test(false);
                    });
                prom.get_future().get();
#else
                Ice::AsyncResultPtr r = p->begin_opContext();
                Ice::Context c = p->end_opContext(r);
                test(c != ctx);
#endif
            }
            {
                test(p->ice_getContext().empty());
#ifdef ICE_CPP11_MAPPING
                promise<void> prom;
                p->opContextAsync(
                    [&](Ice::Context c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr)
                    {
                        test(false);
                    }, nullptr, ctx);
                prom.get_future().get();
#else
                Ice::AsyncResultPtr r = p->begin_opContext(ctx);
                Ice::Context c = p->end_opContext(r);
                test(c == ctx);
#endif
            }
            {
                Test::MyClassPrxPtr p2 = ICE_CHECKED_CAST(Test::MyClassPrx, p->ice_context(ctx));
                test(p2->ice_getContext() == ctx);
#ifdef ICE_CPP11_MAPPING
                promise<void> prom;
                p2->opContextAsync(
                    [&](Ice::Context c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr)
                    {
                        test(false);
                    });
                prom.get_future().get();
#else
                Ice::AsyncResultPtr r = p2->begin_opContext();
                Ice::Context c = p2->end_opContext(r);
                test(c == ctx);
#endif
            }
            {
                Test::MyClassPrxPtr p2 = ICE_CHECKED_CAST(Test::MyClassPrx, p->ice_context(ctx));
#ifdef ICE_CPP11_MAPPING
                promise<void> prom;
                p2->opContextAsync(
                    [&](Ice::Context c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr)
                    {
                        test(false);
                    }, nullptr, ctx);
                prom.get_future().get();
#else
                Ice::AsyncResultPtr r = p2->begin_opContext(ctx);
                Ice::Context c = p2->end_opContext(r);
                test(c == ctx);
#endif
            }
        }

        if(p->ice_getConnection() && communicator->getProperties()->getProperty("Ice.Default.Protocol") != "bt")
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

                Ice::PropertiesPtr properties = ic->getProperties();
                Test::MyClassPrxPtr q =
                    ICE_UNCHECKED_CAST(Test::MyClassPrx,
                                       ic->stringToProxy("test:" + TestHelper::getTestEndpoint(properties)));
                ic->getImplicitContext()->setContext(ctx);
                test(ic->getImplicitContext()->getContext() == ctx);
                {
#ifdef ICE_CPP11_MAPPING
                    promise<void> prom;
                    q->opContextAsync(
                        [&](Ice::Context c)
                        {
                            test(c == ctx);
                            prom.set_value();
                        },
                        [](exception_ptr)
                        {
                            test(false);
                        });
                    prom.get_future().get();
#else
                    Ice::AsyncResultPtr r = q->begin_opContext();
                    Ice::Context c = q->end_opContext(r);
                    test(c == ctx);
#endif
                }

                ic->getImplicitContext()->put("zero", "ZERO");

                ctx = ic->getImplicitContext()->getContext();
                {
#ifdef ICE_CPP11_MAPPING
                    promise<void> prom;
                    q->opContextAsync(
                        [&](Ice::Context c)
                        {
                            test(c == ctx);
                            prom.set_value();
                        },
                        [](exception_ptr ex)
                        {
                            try
                            {
                                rethrow_exception(ex);
                            }
                            catch(const Ice::Exception& e)
                            {
                                cerr << e << endl;
                            }
                            test(false);
                        });
                    prom.get_future().get();
#else
                    Ice::AsyncResultPtr r = q->begin_opContext();
                    Ice::Context c = q->end_opContext(r);
                    test(c == ctx);
#endif
                }

                Ice::Context prxContext;
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                Ice::Context combined = prxContext;
                combined.insert(ctx.begin(), ctx.end());
                test(combined["one"] == "UN");

                q = ICE_UNCHECKED_CAST(Test::MyClassPrx, q->ice_context(prxContext));

                ic->getImplicitContext()->setContext(Ice::Context());
                {
#ifdef ICE_CPP11_MAPPING
                    promise<void> prom;
                    q->opContextAsync(
                        [&](Ice::Context c)
                        {
                            test(c == prxContext);
                            prom.set_value();
                        },
                        [](exception_ptr)
                        {
                            test(false);
                        });
                    prom.get_future().get();
#else
                    Ice::AsyncResultPtr r = q->begin_opContext();
                    Ice::Context c = q->end_opContext(r);
                    test(c == prxContext);
#endif
                }

                ic->getImplicitContext()->setContext(ctx);
                {
#ifdef ICE_CPP11_MAPPING
                    promise<void> prom;
                    q->opContextAsync(
                        [&](Ice::Context c)
                        {
                            test(c == combined);
                            prom.set_value();
                        },
                        [](exception_ptr)
                        {
                            test(false);
                        });
                    prom.get_future().get();
#else
                    Ice::AsyncResultPtr r = q->begin_opContext();
                    Ice::Context c = q->end_opContext(r);
                    test(c == combined);
#endif
                }

                ic->getImplicitContext()->setContext(Ice::Context());
                ic->destroy();
            }
        }
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opDoubleMarshalingAsync(d, ds,
            [&]()
            {
                cb->opDoubleMarshaling();
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opDoubleMarshalingPtr callback =
            Test::newCallback_MyClass_opDoubleMarshaling(cb, &Callback::opDoubleMarshaling, &Callback::exCB);
        p->begin_opDoubleMarshaling(d, ds, callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opIdempotentAsync(
            [&]()
            {
                cb->opIdempotent();
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opIdempotentPtr callback =
            Test::newCallback_MyClass_opIdempotent(cb, &Callback::opIdempotent, &Callback::exCB);
        p->begin_opIdempotent(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opNonmutatingAsync(
            [&]()
            {
                cb->opNonmutating();
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyClass_opNonmutatingPtr callback =
            Test::newCallback_MyClass_opNonmutating(cb, &Callback::opNonmutating, &Callback::exCB);
        p->begin_opNonmutating(callback);
#endif
        cb->check();
    }

    {
        Test::MyDerivedClassPrxPtr derived = ICE_CHECKED_CAST(Test::MyDerivedClassPrx, p);
        test(derived);
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        derived->opDerivedAsync(
            [&]()
            {
                cb->opDerived();
            },
            makeExceptionClosure(cb));
#else
        Test::Callback_MyDerivedClass_opDerivedPtr callback =
            Test::newCallback_MyDerivedClass_opDerived(cb, &Callback::opDerived, &Callback::exCB);
        derived->begin_opDerived(callback);
#endif
        cb->check();
    }

#ifdef ICE_CPP11_MAPPING

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->ice_pingAsync();
        try
        {
            f.get();
            cb->ping();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(const std::exception& ex)
        {
            cerr << ex.what() << endl;
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->ice_isAAsync(Test::MyClass::ice_staticId());
        try
        {
            cb->isA(f.get());
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->ice_idAsync();
        try
        {
            cb->id(f.get());
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->ice_idsAsync();
        try
        {
            cb->ids(f.get());
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opVoidAsync();
        try
        {
            f.get();
            cb->opVoid();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteAsync(Ice::Byte(0xff), Ice::Byte(0x0f));
        try
        {
            auto r = f.get();
            cb->opByte(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opBoolAsync(true, false);
        try
        {
            auto r = f.get();
            cb->opBool(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opShortIntLongAsync(10, 11, 12);
        try
        {
            auto r = f.get();
            cb->opShortIntLong(r.returnValue, r.p4, r.p5, r.p6);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opFloatDoubleAsync(3.14f, 1.1E10);
        try
        {
            auto r = f.get();
            cb->opFloatDouble(r.returnValue, r.p3, r.p4);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringAsync("hello", "world");
        try
        {
            auto r = f.get();
            cb->opString(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opMyEnumAsync(Test::MyEnum::enum2);
        try
        {
            auto r = f.get();
            cb->opMyEnum(r.returnValue, r.p2);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback, communicator);
        auto f = p->opMyClassAsync(p);
        try
        {
            auto r = f.get();
            cb->opMyClass(r.returnValue, r.p2, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::Structure si1;
        si1.p = p;
        si1.e = ICE_ENUM(Test::MyEnum, enum3);
        si1.s.s = "abc";
        Test::Structure si2;
        si2.p = 0;
        si2.e = ICE_ENUM(Test::MyEnum, enum2);
        si2.s.s = "def";

        CallbackPtr cb = ICE_MAKE_SHARED(Callback, communicator);
        auto f = p->opStructAsync(si1, si2);
        try
        {
            auto r = f.get();
            cb->opStruct(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opByteS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::BoolS bsi1;
        Test::BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opBoolSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opBoolS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opShortIntLongSAsync(ssi, isi, lsi);
        try
        {
            auto r = f.get();
            cb->opShortIntLongS(r.returnValue, r.p4, r.p5, r.p6);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opFloatDoubleSAsync(fsi, dsi);
        try
        {
            auto r = f.get();
            cb->opFloatDoubleS(r.returnValue, r.p3, r.p4);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::StringS ssi1;
        Test::StringS ssi2;

        ssi1.push_back("abc");
        ssi1.push_back("de");
        ssi1.push_back("fghi");

        ssi2.push_back("xyz");

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringSAsync(ssi1, ssi2);
        try
        {
            auto r = f.get();
            cb->opStringS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteSSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opByteSS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opFloatDoubleSSAsync(fsi, dsi);
        try
        {
            auto r = f.get();
            cb->opFloatDoubleSS(r.returnValue, r.p3, r.p4);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringSSAsync(ssi1, ssi2);
        try
        {
            auto r = f.get();
            cb->opStringSS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteBoolDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opByteBoolD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opShortIntDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opShortIntD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opLongFloatDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opLongFloatD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringStringDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opStringStringD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::StringMyEnumD di1;
        di1["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di1[""] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di2;
        di2["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di2["qwerty"] = ICE_ENUM(Test::MyEnum, enum3);
        di2["Hello!!"] = ICE_ENUM(Test::MyEnum, enum2);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringMyEnumDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opStringMyEnumD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di1[ms12] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };
        Test::MyStructMyEnumD di2;
        di2[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di2[ms22] = ICE_ENUM(Test::MyEnum, enum3);
        di2[ms23] = ICE_ENUM(Test::MyEnum, enum2);

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opMyStructMyEnumDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opMyStructMyEnumD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteBoolDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opByteBoolDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opShortIntDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opShortIntDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opLongFloatDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opLongFloatDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringStringDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opStringStringDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::StringMyEnumDS dsi1;
        dsi1.resize(2);
        Test::StringMyEnumDS dsi2;
        dsi2.resize(1);

        Test::StringMyEnumD di1;
        di1["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di1[""] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di2;
        di2["abc"] = ICE_ENUM(Test::MyEnum, enum1);
        di2["qwerty"] = ICE_ENUM(Test::MyEnum, enum3);
        di2["Hello!!"] = ICE_ENUM(Test::MyEnum, enum2);
        Test::StringMyEnumD di3;
        di3["Goodbye"] = ICE_ENUM(Test::MyEnum, enum1);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringMyEnumDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opStringMyEnumDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::MyEnumStringDS dsi1;
        dsi1.resize(2);
        Test::MyEnumStringDS dsi2;
        dsi2.resize(1);

        Test::MyEnumStringD di1;
        di1[ICE_ENUM(Test::MyEnum, enum1)] = "abc";
        Test::MyEnumStringD di2;
        di2[ICE_ENUM(Test::MyEnum, enum2)] = "Hello!!";
        di2[ICE_ENUM(Test::MyEnum, enum3)] = "qwerty";
        Test::MyEnumStringD di3;
        di3[ICE_ENUM(Test::MyEnum, enum1)] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opMyEnumStringDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opMyEnumStringDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        Test::MyStructMyEnumDS dsi2;
        dsi2.resize(1);

        Test::MyStruct ms11 = { 1, 1 };
        Test::MyStruct ms12 = { 1, 2 };
        Test::MyStructMyEnumD di1;
        di1[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di1[ms12] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStruct ms22 = { 2, 2 };
        Test::MyStruct ms23 = { 2, 3 };
        Test::MyStructMyEnumD di2;
        di2[ms11] = ICE_ENUM(Test::MyEnum, enum1);
        di2[ms22] = ICE_ENUM(Test::MyEnum, enum3);
        di2[ms23] = ICE_ENUM(Test::MyEnum, enum2);

        Test::MyStructMyEnumD di3;
        di3[ms23] = ICE_ENUM(Test::MyEnum, enum3);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opMyStructMyEnumDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opMyStructMyEnumDS(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opByteByteSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opByteByteSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opBoolBoolSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opBoolBoolSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opShortShortSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opShortShortSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opIntIntSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opIntIntSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opLongLongSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opLongLongSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringFloatSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringFloatSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringDoubleSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringDoubleSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opStringStringSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringStringSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();

    }

    {
        Test::MyEnumMyEnumSD sdi1;
        Test::MyEnumMyEnumSD sdi2;

        Test::MyEnumS si1;
        Test::MyEnumS si2;
        Test::MyEnumS si3;

        si1.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si1.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si1.push_back(ICE_ENUM(Test::MyEnum, enum2));
        si2.push_back(ICE_ENUM(Test::MyEnum, enum1));
        si2.push_back(ICE_ENUM(Test::MyEnum, enum2));
        si3.push_back(ICE_ENUM(Test::MyEnum, enum3));
        si3.push_back(ICE_ENUM(Test::MyEnum, enum3));

        sdi1[ICE_ENUM(Test::MyEnum, enum3)] = si1;
        sdi1[ICE_ENUM(Test::MyEnum, enum2)] = si2;
        sdi2[ICE_ENUM(Test::MyEnum, enum1)] = si3;

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opMyEnumMyEnumSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opMyEnumMyEnumSD(r.returnValue, r.p3);
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
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
            CallbackPtr cb = ICE_MAKE_SHARED(Callback);
            auto f = p->opIntSAsync(s);
            try
            {
                cb->opIntS(f.get());
            }
            catch(const Ice::Exception& ex)
            {
                cb->exCB(ex);
            }
            catch(...)
            {
                test(false);
            }
            cb->check();
        }
    }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opDoubleMarshalingAsync(d, ds);
        try
        {
            f.get();
            cb->opDoubleMarshaling();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opIdempotentAsync();
        try
        {
            f.get();
            cb->opIdempotent();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = p->opNonmutatingAsync();
        try
        {
            f.get();
            cb->opNonmutating();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }

    {
        Test::MyDerivedClassPrxPtr derived = ICE_CHECKED_CAST(Test::MyDerivedClassPrx, p);
        test(derived);
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        auto f = derived->opDerivedAsync();
        try
        {
            f.get();
            cb->opDerived();
        }
        catch(const Ice::Exception& ex)
        {
            cb->exCB(ex);
        }
        catch(...)
        {
            test(false);
        }
        cb->check();
    }
#endif
}
