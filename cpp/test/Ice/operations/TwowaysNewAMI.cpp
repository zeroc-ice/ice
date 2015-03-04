// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    void exCB(const Ice::Exception&)
    {
        test(false);
    }

private:

    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

}

void
twowaysNewAMI(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrx& p)
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
