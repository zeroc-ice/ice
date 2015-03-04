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
#include <limits>

//
// Visual C++ defines min and max as macros
//
#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

//
// Work-around for GCC warning bug
//
#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

using namespace std;

namespace
{

class PerThreadContextInvokeThread : public IceUtil::Thread
{
public:
    
PerThreadContextInvokeThread(const Test::MyClassPrx& proxy) : _proxy(proxy)
{
}
    
virtual void
run()
{
    Ice::Context ctx = _proxy->ice_getCommunicator()->getImplicitContext()->getContext();
    test(ctx.empty());
    ctx["one"] = "UN";
    _proxy->ice_getCommunicator()->getImplicitContext()->setContext(ctx);
    test(_proxy->opContext() == ctx);
}
    
private:
    
    Test::MyClassPrx _proxy;
};

}
    
void
twoways(const Ice::CommunicatorPtr& communicator, const Test::MyClassPrx& p)
{
    {
        p->ice_ping();
    }

    {
        test(p->ice_isA(Test::MyClass::ice_staticId()));
    }

    {
        test(p->ice_id() == Test::MyDerivedClass::ice_staticId());
    }

    {
        Ice::StringSeq ids = p->ice_ids();
        test(ids.size() == 3);
        test(ids[0] == "::Ice::Object");
        test(ids[1] == "::Test::MyClass");
        test(ids[2] == "::Test::MyDerivedClass");
    }

    {
        p->opVoid();
    }

    {
        Ice::Byte b;
        Ice::Byte r;

        r = p->opByte(Ice::Byte(0xff), Ice::Byte(0x0f), b);
        test(b == Ice::Byte(0xf0));
        test(r == Ice::Byte(0xff));
    }

    {
        bool b;
        bool r;

        r = p->opBool(true, false, b);
        test(b);
        test(!r);
    }

    {
        Ice::Short s;
        Ice::Int i;
        Ice::Long l;
        Ice::Long r;

        r = p->opShortIntLong(10, 11, 12, s, i, l);
        test(s == 10);
        test(i == 11);
        test(l == 12);
        test(r == 12);
        
        r = p->opShortIntLong(numeric_limits<Ice::Short>::min(), numeric_limits<Ice::Int>::min(),
                              numeric_limits<Ice::Long>::min(), s, i, l);
        test(s == numeric_limits<Ice::Short>::min());
        test(i == numeric_limits<Ice::Int>::min());
        test(l == numeric_limits<Ice::Long>::min());
        test(r == numeric_limits<Ice::Long>::min());
        
        r = p->opShortIntLong(numeric_limits<Ice::Short>::max(), numeric_limits<Ice::Int>::max(),
                              numeric_limits<Ice::Long>::max(), s, i, l);
        test(s == numeric_limits<Ice::Short>::max());
        test(i == numeric_limits<Ice::Int>::max());
        test(l == numeric_limits<Ice::Long>::max());
        test(r == numeric_limits<Ice::Long>::max());
    }

    {
        Ice::Float f;
        Ice::Double d;
        Ice::Double r;
        
        r = p->opFloatDouble(Ice::Float(3.14), Ice::Double(1.1E10), f, d);
        test(f == Ice::Float(3.14));
        test(d == Ice::Double(1.1E10));
        test(r == Ice::Double(1.1E10));

        r = p->opFloatDouble(numeric_limits<Ice::Float>::min(), numeric_limits<Ice::Double>::min(), f, d);
        test(f == numeric_limits<Ice::Float>::min());
        test(d == numeric_limits<Ice::Double>::min());
        test(r == numeric_limits<Ice::Double>::min());

        r = p->opFloatDouble(numeric_limits<Ice::Float>::max(), numeric_limits<Ice::Double>::max(), f, d);
        test(f == numeric_limits<Ice::Float>::max());
        test(d == numeric_limits<Ice::Double>::max());
        test(r == numeric_limits<Ice::Double>::max());
    }

    {
        string s;
        string r;

        r = p->opString("hello", "world", s);
        test(s == "world hello");
        test(r == "hello world");
    }

    {
        Test::MyEnum e;
        Test::MyEnum r;
        
        r = p->opMyEnum(Test::enum2, e);
        test(e == Test::enum2);
        test(r == Test::enum3);
    }

    {
        Test::MyClassPrx c1;
        Test::MyClassPrx c2;
        Test::MyClassPrx r;
        
        r = p->opMyClass(p, c1, c2);
        test(Ice::proxyIdentityAndFacetEqual(c1, p));
        test(!Ice::proxyIdentityAndFacetEqual(c2, p));
        test(Ice::proxyIdentityAndFacetEqual(r, p));
        test(c1->ice_getIdentity() == communicator->stringToIdentity("test"));
        test(c2->ice_getIdentity() == communicator->stringToIdentity("noSuchIdentity"));
        test(r->ice_getIdentity() == communicator->stringToIdentity("test"));
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

        r = p->opMyClass(0, c1, c2);
        test(c1 == 0);
        test(c2 != 0);
        test(Ice::proxyIdentityAndFacetEqual(r, p));
        r->opVoid();
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
        
        Test::Structure so;
        Test::Structure rso = p->opStruct(si1, si2, so);
        test(rso.p == 0);
        test(rso.e == Test::enum2);
        test(rso.s.s == "def");
        test(so.p == p);
        test(so.e == Test::enum3);
        test(so.s.s == "a new string");
        so.p->opVoid();
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

        Test::ByteS bso;
        Test::ByteS rso;

        rso = p->opByteS(bsi1, bsi2, bso);
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
    }

    {
        Test::BoolS bsi1;
        Test::BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

        Test::BoolS bso;
        Test::BoolS rso;

        rso = p->opBoolS(bsi1, bsi2, bso);
        test(bso.size() == 4);
        test(bso[0]);
        test(bso[1]);
        test(!bso[2]);
        test(!bso[3]);
        test(rso.size() == 3);
        test(!rso[0]);
        test(rso[1]);
        test(rso[2]);
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

        Test::ShortS sso;
        Test::IntS iso;
        Test::LongS lso;
        Test::LongS rso;

        rso = p->opShortIntLongS(ssi, isi, lsi, sso, iso, lso);
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
    }

    {
        Test::FloatS fsi;
        Test::DoubleS dsi;

        fsi.push_back(Ice::Float(3.14));
        fsi.push_back(Ice::Float(1.11));

        dsi.push_back(Ice::Double(1.1E10));
        dsi.push_back(Ice::Double(1.2E10));
        dsi.push_back(Ice::Double(1.3E10));

        Test::FloatS fso;
        Test::DoubleS dso;
        Test::DoubleS rso;

        rso = p->opFloatDoubleS(fsi, dsi, fso, dso);
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
    }

    {
        Test::StringS ssi1;
        Test::StringS ssi2;

        ssi1.push_back("abc");
        ssi1.push_back("de");
        ssi1.push_back("fghi");

        ssi2.push_back("xyz");

        Test::StringS sso;
        Test::StringS rso;

        rso = p->opStringS(ssi1, ssi2, sso);
        test(sso.size() == 4);
        test(sso[0] == "abc");
        test(sso[1] == "de");
        test(sso[2] == "fghi");
        test(sso[3] == "xyz");
        test(rso.size() == 3);
        test(rso[0] == "fghi");
        test(rso[1] == "de");
        test(rso[2] == "abc");
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

        Test::ByteSS bso;
        Test::ByteSS rso;

        rso = p->opByteSS(bsi1, bsi2, bso);
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

        Test::FloatSS fso;
        Test::DoubleSS dso;
        Test::DoubleSS rso;

        rso = p->opFloatDoubleSS(fsi, dsi, fso, dso);
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

        Test::StringSS sso;
        Test::StringSS rso;

        rso = p->opStringSS(ssi1, ssi2, sso);
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
    }

    {
        Test::StringSSS sssi1;
        sssi1.resize(2);
        sssi1[0].resize(2);
        sssi1[0][0].push_back("abc");
        sssi1[0][0].push_back("de");
        sssi1[0][1].push_back("xyz");
        sssi1[1].resize(1);
        sssi1[1][0].push_back("hello");

        Test::StringSSS sssi2;
        sssi2.resize(3);
        sssi2[0].resize(2);
        sssi2[0][0].push_back("");
        sssi2[0][0].push_back("");
        sssi2[0][1].push_back("abcd");
        sssi2[1].resize(1);
        sssi2[1][0].push_back("");

        Test::StringSSS ssso;
        Test::StringSSS rsso;

        rsso = p->opStringSSS(sssi1, sssi2, ssso);
        test(ssso.size() == 5);
        test(ssso[0].size() == 2);
        test(ssso[0][0].size() == 2);
        test(ssso[0][1].size() == 1);
        test(ssso[1].size() == 1);
        test(ssso[1][0].size() == 1);
        test(ssso[2].size() == 2);
        test(ssso[2][0].size() == 2);
        test(ssso[2][1].size() == 1);
        test(ssso[3].size() == 1);
        test(ssso[3][0].size() == 1);
        test(ssso[4].size() == 0);
        test(ssso[0][0][0] == "abc");
        test(ssso[0][0][1] == "de");
        test(ssso[0][1][0] == "xyz");
        test(ssso[1][0][0] == "hello");
        test(ssso[2][0][0] == "");
        test(ssso[2][0][1] == "");
        test(ssso[2][1][0] == "abcd");
        test(ssso[3][0][0] == "");

        test(rsso.size() == 3);
        test(rsso[0].size() == 0);
        test(rsso[1].size() == 1);
        test(rsso[1][0].size() == 1);
        test(rsso[2].size() == 2);
        test(rsso[2][0].size() == 2);
        test(rsso[2][1].size() == 1);
        test(rsso[1][0][0] == "");
        test(rsso[2][0][0] == "");
        test(rsso[2][0][1] == "");
        test(rsso[2][1][0] == "abcd");
    }

    {
        Test::ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        Test::ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;

        Test::ByteBoolD _do;
        Test::ByteBoolD ro = p->opByteBoolD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[10] == true);
        test(ro[11] == false);
        test(ro[100] == false);
        test(ro[101] == true);
    }

    {
        Test::ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        Test::ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;

        Test::ShortIntD _do;
        Test::ShortIntD ro = p->opShortIntD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[110] == -1);
        test(ro[111] == -100);
        test(ro[1100] == 123123);
        test(ro[1101] == 0);
    }

    {
        Test::LongFloatD di1;
        di1[999999110] = Ice::Float(-1.1);
        di1[999999111] = Ice::Float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = Ice::Float(-1.1);
        di2[999999120] = Ice::Float(-100.4);
        di2[999999130] = Ice::Float(0.5);

        Test::LongFloatD _do;
        Test::LongFloatD ro = p->opLongFloatD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[999999110] == Ice::Float(-1.1));
        test(ro[999999120] == Ice::Float(-100.4));
        test(ro[999999111] == Ice::Float(123123.2));
        test(ro[999999130] == Ice::Float(0.5));
    }

    {
        Test::StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        Test::StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";

        Test::StringStringD _do;
        Test::StringStringD ro = p->opStringStringD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro["foo"] == "abc -1.1");
        test(ro["FOO"] == "abc -100.4");
        test(ro["bar"] == "abc 123123.2");
        test(ro["BAR"] == "abc 0.5");
    }

    {
        Test::StringMyEnumD di1;
        di1["abc"] = Test::enum1;
        di1[""] = Test::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = Test::enum1;
        di2["qwerty"] = Test::enum3;
        di2["Hello!!"] = Test::enum2;

        Test::StringMyEnumD _do;
        Test::StringMyEnumD ro = p->opStringMyEnumD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro["abc"] == Test::enum1);
        test(ro["qwerty"] == Test::enum3);
        test(ro[""] == Test::enum2);
        test(ro["Hello!!"] == Test::enum2);
    }

    {
        Test::MyEnumStringD di1;
        di1[Test::enum1] = "abc";
        Test::MyEnumStringD di2;
        di2[Test::enum2] = "Hello!!";
        di2[Test::enum3] = "qwerty";

        Test::MyEnumStringD _do;
        Test::MyEnumStringD ro = p->opMyEnumStringD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 3);
        test(ro[Test::enum1] == "abc");
        test(ro[Test::enum2] == "Hello!!");
        test(ro[Test::enum3] == "qwerty");
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

        Test::MyStructMyEnumD _do;
        Test::MyStructMyEnumD ro = p->opMyStructMyEnumD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[s11] == Test::enum1);
        test(ro[s12] == Test::enum2);
        test(ro[s22] == Test::enum3);
        test(ro[s23] == Test::enum2);
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
            Test::IntS r = p->opIntS(s);
            test(r.size() == static_cast<size_t>(lengths[l]));
            for(int j = 0; j < static_cast<int>(r.size()); ++j)
            {
                test(r[j] == -j);
            }
        }
    }

    {
        Ice::Context ctx;
        ctx["one"] = "ONE";
        ctx["two"] = "TWO";
        ctx["three"] = "THREE";
        {
            Test::StringStringD r = p->opContext();
            test(p->ice_getContext().empty());
            test(r != ctx);
        }
        {
            Test::StringStringD r = p->opContext(ctx);
            test(p->ice_getContext().empty());
            test(r == ctx);
        }
        {
            Test::MyClassPrx p2 = Test::MyClassPrx::checkedCast(p->ice_context(ctx));
            test(p2->ice_getContext() == ctx);
            Test::StringStringD r = p2->opContext();
            test(r == ctx);
            r = p2->opContext(ctx);
            test(r == ctx);
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
                                        ic->stringToProxy("test:default -p 12010 -t 10000"));
                
                ic->getImplicitContext()->setContext(ctx);
                test(ic->getImplicitContext()->getContext() == ctx);
                test(p->opContext() == ctx);
                
                test(ic->getImplicitContext()->containsKey("zero") == false);
                string r = ic->getImplicitContext()->put("zero", "ZERO");
                test(r == "");
                test(ic->getImplicitContext()->containsKey("zero") == true);
                test(ic->getImplicitContext()->get("zero") == "ZERO");

                ctx = ic->getImplicitContext()->getContext();
                test(p->opContext() == ctx);
                Ice::Context prxContext;
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";
                
                Ice::Context combined = prxContext;
                combined.insert(ctx.begin(), ctx.end());
                test(combined["one"] == "UN");
                
                p = Test::MyClassPrx::uncheckedCast(p->ice_context(prxContext));

                ic->getImplicitContext()->setContext(Ice::Context());
                test(p->opContext() == prxContext);

                ic->getImplicitContext()->setContext(ctx);
                test(p->opContext() == combined);

                test(ic->getImplicitContext()->remove("one") == "ONE");

                if(impls[i] == "PerThread")
                {
                    IceUtil::ThreadPtr thread = new PerThreadContextInvokeThread(p->ice_context(Ice::Context()));
                    thread->start();
                    thread->getThreadControl().join();
                }

                ic->getImplicitContext()->setContext(Ice::Context()); // Clear the context to avoid leak report.
                ic->destroy();
            }
        }
#endif
        }

    {
        Ice::Double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        p->opDoubleMarshaling(d, ds);
    }

    p->opIdempotent();

    p->opNonmutating();
}
