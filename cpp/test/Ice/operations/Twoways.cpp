// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"
#include <limits>

//
// Visual C++ defines min and max as macros
//
#ifdef min
#    undef min
#endif
#ifdef max
#    undef max
#endif

//
// Work-around for GCC warning bug
//
#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

using namespace std;
using namespace Test;

namespace
{
    class PerThreadContextInvokeThread final
    {
    public:
        PerThreadContextInvokeThread(Test::MyClassPrx proxy) : _proxy(std::move(proxy)) {}

        void run()
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
    Test::StringS literals = p->opStringLiterals();

    test(Test::s0 == "\\" && Test::s0 == Test::sw0 && Test::s0 == literals[0] && Test::s0 == literals[11]);

    test(Test::s1 == "A" && Test::s1 == Test::sw1 && Test::s1 == literals[1] && Test::s1 == literals[12]);

    test(Test::s2 == "Ice" && Test::s2 == Test::sw2 && Test::s2 == literals[2] && Test::s2 == literals[13]);

    test(Test::s3 == "A21" && Test::s3 == Test::sw3 && Test::s3 == literals[3] && Test::s3 == literals[14]);

    test(
        Test::s4 == "\\u0041 \\U00000041" && Test::s4 == Test::sw4 && Test::s4 == literals[4] &&
        Test::s4 == literals[15]);

    test(Test::s5 == "\xc3\xbf" && Test::s5 == Test::sw5 && Test::s5 == literals[5] && Test::s5 == literals[16]);

    test(Test::s6 == "\xcf\xbf" && Test::s6 == Test::sw6 && Test::s6 == literals[6] && Test::s6 == literals[17]);

    test(Test::s7 == "\xd7\xb0" && Test::s7 == Test::sw7 && Test::s7 == literals[7] && Test::s7 == literals[18]);

    test(
        Test::s8 == "\xf0\x90\x80\x80" && Test::s8 == Test::sw8 && Test::s8 == literals[8] && Test::s8 == literals[19]);

    test(
        Test::s9 == "\xf0\x9f\x8d\x8c" && Test::s9 == Test::sw9 && Test::s9 == literals[9] && Test::s9 == literals[20]);

    test(
        Test::s10 == "\xe0\xb6\xa7" && Test::s10 == Test::sw10 && Test::s10 == literals[10] &&
        Test::s10 == literals[21]);

    test(Test::s11 == "\xe2\x82\xac\xe2\x82\xac\xe2\x82\xac");
    test(Test::s12 == "\\101");

    test(
        Test::ss0 == "\'\"\?\\\a\b\f\n\r\t\v\006" && Test::ss0 == Test::ss1 && Test::ss0 == Test::ss2 &&
        Test::ss0 == literals[22] && Test::ss0 == literals[23] && Test::ss0 == literals[24]);

    test(Test::ss3 == "\\\\U\\u\\" && Test::ss3 == literals[25]);

    test(Test::ss4 == "\\A\\" && Test::ss4 == literals[26]);

    test(Test::ss5 == "\\u0041\\" && Test::ss5 == literals[27]);

    test(Test::su0 == Test::su1);
    test(Test::su0 == Test::su2);
    test(Test::su0 == literals[28]);
    test(Test::su0 == literals[29]);
    test(Test::su0 == literals[30]);

    //
    // Same but using wide strings
    //
    Test::WStringS wliterals = p->opWStringLiterals();

    test(Test::ws0 == L"\\" && Test::ws0 == Test::wsw0 && Test::ws0 == wliterals[0] && Test::ws0 == wliterals[11]);

    test(Test::ws1 == L"A" && Test::ws1 == Test::wsw1 && Test::ws1 == wliterals[1] && Test::ws1 == wliterals[12]);

    test(Test::ws2 == L"Ice" && Test::ws2 == Test::wsw2 && Test::ws2 == wliterals[2] && Test::ws2 == wliterals[13]);

    test(Test::ws3 == L"A21" && Test::ws3 == Test::wsw3 && Test::ws3 == wliterals[3] && Test::ws3 == wliterals[14]);

    test(
        Test::ws4 == L"\\u0041 \\U00000041" && Test::ws4 == Test::wsw4 && Test::ws4 == wliterals[4] &&
        Test::ws4 == wliterals[15]);

    test(Test::ws5 == L"\u00FF" && Test::ws5 == Test::wsw5 && Test::ws5 == wliterals[5] && Test::ws5 == wliterals[16]);

    test(Test::ws6 == L"\u03FF" && Test::ws6 == Test::wsw6 && Test::ws6 == wliterals[6] && Test::ws6 == wliterals[17]);

    test(Test::ws7 == L"\u05F0" && Test::ws7 == Test::wsw7 && Test::ws7 == wliterals[7] && Test::ws7 == wliterals[18]);

    test(
        Test::ws8 == L"\U00010000" && Test::ws8 == Test::wsw8 && Test::ws8 == wliterals[8] &&
        Test::ws8 == wliterals[19]);

    test(
        Test::ws9 == L"\U0001F34C" && Test::ws9 == Test::wsw9 && Test::ws9 == wliterals[9] &&
        Test::ws9 == wliterals[20]);

    test(
        Test::ws10 == L"\u0DA7" && Test::ws10 == Test::wsw10 && Test::ws10 == wliterals[10] &&
        Test::ws10 == wliterals[21]);

    test(
        Test::wss0 == L"\'\"\?\\\a\b\f\n\r\t\v\006" && Test::wss0 == Test::wss1 && Test::wss0 == Test::wss2 &&
        Test::wss0 == wliterals[22] && Test::wss0 == wliterals[23] && Test::wss0 == wliterals[24]);

    test(Test::wss3 == L"\\\\U\\u\\" && Test::wss3 == wliterals[25]);

    test(Test::wss4 == L"\\A\\" && Test::wss4 == wliterals[26]);

    test(Test::wss5 == L"\\u0041\\" && Test::wss5 == wliterals[27]);

    test(
        Test::wsu0 == Test::wsu1 && Test::wsu0 == Test::wsu2 && Test::wsu0 == wliterals[28] &&
        Test::wsu0 == wliterals[29] && Test::wsu0 == wliterals[30]);

    {
        p->ice_ping();
    }

    {
        test(string{Test::MyClassPrx::ice_staticId()} == Test::MyClass::ice_staticId());
        test(string{Ice::ObjectPrx::ice_staticId()} == Ice::Object::ice_staticId());
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
        uint8_t b;
        uint8_t r;

        r = p->opByte(uint8_t(0xff), uint8_t(0x0f), b);
        test(b == uint8_t(0xf0));
        test(r == uint8_t(0xff));
    }

    {
        bool b;
        bool r;

        r = p->opBool(true, false, b);
        test(b);
        test(!r);
    }

    {
        int16_t s;
        int32_t i;
        int64_t l;
        int64_t r;

        r = p->opShortIntLong(10, 11, 12, s, i, l);
        test(s == 10);
        test(i == 11);
        test(l == 12);
        test(r == 12);

        r = p->opShortIntLong(
            numeric_limits<int16_t>::min(),
            numeric_limits<int32_t>::min(),
            numeric_limits<int64_t>::min(),
            s,
            i,
            l);
        test(s == numeric_limits<int16_t>::min());
        test(i == numeric_limits<int32_t>::min());
        test(l == numeric_limits<int64_t>::min());
        test(r == numeric_limits<int64_t>::min());

        r = p->opShortIntLong(
            numeric_limits<int16_t>::max(),
            numeric_limits<int32_t>::max(),
            numeric_limits<int64_t>::max(),
            s,
            i,
            l);
        test(s == numeric_limits<int16_t>::max());
        test(i == numeric_limits<int32_t>::max());
        test(l == numeric_limits<int64_t>::max());
        test(r == numeric_limits<int64_t>::max());
    }

    {
        float f;
        double d;
        double r;

        r = p->opFloatDouble(float(3.14), double(1.1E10), f, d);
        test(f == float(3.14));
        test(d == double(1.1E10));
        test(r == double(1.1E10));

        r = p->opFloatDouble(numeric_limits<float>::min(), numeric_limits<double>::min(), f, d);
        test(f == numeric_limits<float>::min());
        test(d == numeric_limits<double>::min());
        test(r == numeric_limits<double>::min());

        r = p->opFloatDouble(numeric_limits<float>::max(), numeric_limits<double>::max(), f, d);
        test(f == numeric_limits<float>::max());
        test(d == numeric_limits<double>::max());
        test(r == numeric_limits<double>::max());
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

        r = p->opMyEnum(MyEnum::enum2, e);
        test(e == MyEnum::enum2);
        test(r == MyEnum::enum3);
    }

    {
        optional<MyClassPrx> c1;
        optional<MyClassPrx> c2;
        optional<MyClassPrx> r;

        r = p->opMyClass(p, c1, c2);
        test(Ice::proxyIdentityAndFacetEqual(c1, p));
        test(!Ice::proxyIdentityAndFacetEqual(c2, p));
        test(Ice::proxyIdentityAndFacetEqual(r, p));
        test(c1->ice_getIdentity() == Ice::stringToIdentity("test"));
        test(c2->ice_getIdentity() == Ice::stringToIdentity("noSuchIdentity"));
        test(r->ice_getIdentity() == Ice::stringToIdentity("test"));
        r->opVoid();
        c1->opVoid();
        try
        {
            c2->opVoid();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        r = p->opMyClass(nullopt, c1, c2);
        test(c1 == nullopt);
        test(c2 != nullopt);
        test(Ice::proxyIdentityAndFacetEqual(r, p));
        r->opVoid();
    }

    {
        Test::Structure si1;
        si1.p = p;
        si1.e = MyEnum::enum3;
        si1.s.s = "abc";
        Test::Structure si2;
        si2.p = nullopt;
        si2.e = MyEnum::enum2;
        si2.s.s = "def";

        Test::Structure so;
        Test::Structure rso = p->opStruct(si1, si2, so);
        test(rso.p == nullopt);
        test(rso.e == MyEnum::enum2);
        test(rso.s.s == "def");
        test(so.p == p);
        test(so.e == MyEnum::enum3);
        test(so.s.s == "a new string");
        so.p->opVoid();
    }

    {
        Test::ByteS bsi1;
        Test::ByteS bsi2;

        bsi1.push_back(byte{0x01});
        bsi1.push_back(byte{0x11});
        bsi1.push_back(byte{0x12});
        bsi1.push_back(byte{0x22});

        bsi2.push_back(byte{0xf1});
        bsi2.push_back(byte{0xf2});
        bsi2.push_back(byte{0xf3});
        bsi2.push_back(byte{0xf4});

        Test::ByteS bso;
        Test::ByteS rso;

        rso = p->opByteS(bsi1, bsi2, bso);
        test(bso.size() == 4);
        test(bso[0] == byte{0x22});
        test(bso[1] == byte{0x12});
        test(bso[2] == byte{0x11});
        test(bso[3] == byte{0x01});
        test(rso.size() == 8);
        test(rso[0] == byte{0x01});
        test(rso[1] == byte{0x11});
        test(rso[2] == byte{0x12});
        test(rso[3] == byte{0x22});
        test(rso[4] == byte{0xf1});
        test(rso[5] == byte{0xf2});
        test(rso[6] == byte{0xf3});
        test(rso[7] == byte{0xf4});
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

        fsi.push_back(float(3.14));
        fsi.push_back(float(1.11));

        dsi.push_back(double(1.1E10));
        dsi.push_back(double(1.2E10));
        dsi.push_back(double(1.3E10));

        Test::FloatS fso;
        Test::DoubleS dso;
        Test::DoubleS rso;

        rso = p->opFloatDoubleS(fsi, dsi, fso, dso);
        test(fso.size() == 2);
        test(fso[0] == float(3.14));
        test(fso[1] == float(1.11));
        test(dso.size() == 3);
        test(dso[0] == double(1.3E10));
        test(dso[1] == double(1.2E10));
        test(dso[2] == double(1.1E10));
        test(rso.size() == 5);
        test(rso[0] == double(1.1E10));
        test(rso[1] == double(1.2E10));
        test(rso[2] == double(1.3E10));
        test(float(rso[3]) == float(3.14));
        test(float(rso[4]) == float(1.11));
    }

    {
        Test::StringS ssi1;
        Test::StringS ssi2;

        ssi1.emplace_back("abc");
        ssi1.emplace_back("de");
        ssi1.emplace_back("fghi");

        ssi2.emplace_back("xyz");

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

        bsi1[0].push_back(byte{0x01});
        bsi1[0].push_back(byte{0x11});
        bsi1[0].push_back(byte{0x12});
        bsi1[1].push_back(byte{0xff});

        bsi2[0].push_back(byte{0x0e});
        bsi2[1].push_back(byte{0xf2});
        bsi2[1].push_back(byte{0xf1});

        Test::ByteSS bso;
        Test::ByteSS rso;

        rso = p->opByteSS(bsi1, bsi2, bso);
        test(bso.size() == 2);
        test(bso[0].size() == 1);
        test(bso[0][0] == byte{0xff});
        test(bso[1].size() == 3);
        test(bso[1][0] == byte{0x01});
        test(bso[1][1] == byte{0x11});
        test(bso[1][2] == byte{0x12});
        test(rso.size() == 4);
        test(rso[0].size() == 3);
        test(rso[0][0] == byte{0x01});
        test(rso[0][1] == byte{0x11});
        test(rso[0][2] == byte{0x12});
        test(rso[1].size() == 1);
        test(rso[1][0] == byte{0xff});
        test(rso[2].size() == 1);
        test(rso[2][0] == byte{0x0e});
        test(rso[3].size() == 2);
        test(rso[3][0] == byte{0xf2});
        test(rso[3][1] == byte{0xf1});
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

        Test::BoolSS bso;
        Test::BoolSS rso;

        rso = p->opBoolSS(bsi1, bsi2, bso);
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

        Test::LongSS rso;
        Test::ShortSS sso;
        Test::IntSS iso;
        Test::LongSS lso;

        rso = p->opShortIntLongSS(ssi, isi, lsi, sso, iso, lso);
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
    }

    {
        Test::FloatSS fsi;
        fsi.resize(3);
        Test::DoubleSS dsi;
        dsi.resize(1);

        fsi[0].push_back(float(3.14));
        fsi[1].push_back(float(1.11));

        dsi[0].push_back(double(1.1E10));
        dsi[0].push_back(double(1.2E10));
        dsi[0].push_back(double(1.3E10));

        Test::FloatSS fso;
        Test::DoubleSS dso;
        Test::DoubleSS rso;

        rso = p->opFloatDoubleSS(fsi, dsi, fso, dso);
        test(fso.size() == 3);
        test(fso[0].size() == 1);
        test(fso[0][0] == float(3.14));
        test(fso[1].size() == 1);
        test(fso[1][0] == float(1.11));
        test(fso[2].size() == 0);
        test(dso.size() == 1);
        test(dso[0].size() == 3);
        test(dso[0][0] == double(1.1E10));
        test(dso[0][1] == double(1.2E10));
        test(dso[0][2] == double(1.3E10));
        test(rso.size() == 2);
        test(rso[0].size() == 3);
        test(rso[0][0] == double(1.1E10));
        test(rso[0][1] == double(1.2E10));
        test(rso[0][2] == double(1.3E10));
        test(rso[1].size() == 3);
        test(rso[1][0] == double(1.1E10));
        test(rso[1][1] == double(1.2E10));
        test(rso[1][2] == double(1.3E10));
    }

    {
        Test::StringSS ssi1;
        ssi1.resize(2);
        Test::StringSS ssi2;
        ssi2.resize(3);

        ssi1[0].emplace_back("abc");
        ssi1[1].emplace_back("de");
        ssi1[1].emplace_back("fghi");

        ssi2[2].emplace_back("xyz");

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
        sssi1[0][0].emplace_back("abc");
        sssi1[0][0].emplace_back("de");
        sssi1[0][1].emplace_back("xyz");
        sssi1[1].resize(1);
        sssi1[1][0].emplace_back("hello");

        Test::StringSSS sssi2;
        sssi2.resize(3);
        sssi2[0].resize(2);
        sssi2[0][0].emplace_back("");
        sssi2[0][0].emplace_back("");
        sssi2[0][1].emplace_back("abcd");
        sssi2[1].resize(1);
        sssi2[1][0].emplace_back("");

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
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);

        Test::LongFloatD _do;
        Test::LongFloatD ro = p->opLongFloatD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[999999110] == float(-1.1));
        test(ro[999999120] == float(-100.4));
        test(ro[999999111] == float(123123.2));
        test(ro[999999130] == float(0.5));
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
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;

        Test::StringMyEnumD _do;
        Test::StringMyEnumD ro = p->opStringMyEnumD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro["abc"] == MyEnum::enum1);
        test(ro["qwerty"] == MyEnum::enum3);
        test(ro[""] == MyEnum::enum2);
        test(ro["Hello!!"] == MyEnum::enum2);
    }

    {
        Test::MyEnumStringD di1;
        di1[MyEnum::enum1] = "abc";
        Test::MyEnumStringD di2;
        di2[MyEnum::enum2] = "Hello!!";
        di2[MyEnum::enum3] = "qwerty";

        Test::MyEnumStringD _do;
        Test::MyEnumStringD ro = p->opMyEnumStringD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 3);
        test(ro[MyEnum::enum1] == "abc");
        test(ro[MyEnum::enum2] == "Hello!!");
        test(ro[MyEnum::enum3] == "qwerty");
    }

    {
        Test::MyStruct ms11 = {1, 1};
        Test::MyStruct ms12 = {1, 2};
        Test::MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        Test::MyStruct ms22 = {2, 2};
        Test::MyStruct ms23 = {2, 3};
        Test::MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        Test::MyStructMyEnumD _do;
        Test::MyStructMyEnumD ro = p->opMyStructMyEnumD(di1, di2, _do);

        test(_do == di1);
        test(ro.size() == 4);
        test(ro[ms11] == MyEnum::enum1);
        test(ro[ms12] == MyEnum::enum2);
        test(ro[ms22] == MyEnum::enum3);
        test(ro[ms23] == MyEnum::enum2);
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

        try
        {
            Test::ByteBoolDS _do;
            Test::ByteBoolDS ro = p->opByteBoolDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0][10] == true);
            test(ro[0][11] == false);
            test(ro[0][101] == true);
            test(ro[1].size() == 2);
            test(ro[1][10] == true);
            test(ro[1][100] == false);

            test(_do.size() == 3);
            test(_do[0].size() == 2);
            test(_do[0][100] == false);
            test(_do[0][101] == false);
            test(_do[1].size() == 2);
            test(_do[1][10] == true);
            test(_do[1][100] == false);
            test(_do[2].size() == 3);
            test(_do[2][10] == true);
            test(_do[2][11] == false);
            test(_do[2][101] == true);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::ShortIntDS _do;
            Test::ShortIntDS ro = p->opShortIntDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0][110] == -1);
            test(ro[0][111] == -100);
            test(ro[0][1101] == 0);
            test(ro[1].size() == 2);
            test(ro[1][110] == -1);
            test(ro[1][1100] == 123123);

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0][100] == -1001);
            test(_do[1].size() == 2);
            test(_do[1][110] == -1);
            test(_do[1][1100] == 123123);
            test(_do[2].size() == 3);
            test(_do[2][110] == -1);
            test(_do[2][111] == -100);
            test(_do[2][1101] == 0);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::LongFloatDS dsi1;
        dsi1.resize(2);
        Test::LongFloatDS dsi2;
        dsi2.resize(1);

        Test::LongFloatD di1;
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        Test::LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);
        Test::LongFloatD di3;
        di3[999999140] = float(3.14);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        try
        {
            Test::LongFloatDS _do;
            Test::LongFloatDS ro = p->opLongFloatDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0][999999110] == float(-1.1));
            test(ro[0][999999120] == float(-100.4));
            test(ro[0][999999130] == float(0.5));
            test(ro[1].size() == 2);
            test(ro[1][999999110] == float(-1.1));
            test(ro[1][999999111] == float(123123.2));

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0][999999140] == float(3.14));
            test(_do[1].size() == 2);
            test(_do[1][999999110] == float(-1.1));
            test(_do[1][999999111] == float(123123.2));
            test(_do[2].size() == 3);
            test(_do[2][999999110] == float(-1.1));
            test(_do[2][999999120] == float(-100.4));
            test(_do[2][999999130] == float(0.5));
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::StringStringDS _do;
            Test::StringStringDS ro = p->opStringStringDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0]["foo"] == "abc -1.1");
            test(ro[0]["FOO"] == "abc -100.4");
            test(ro[0]["BAR"] == "abc 0.5");
            test(ro[1].size() == 2);
            test(ro[1]["foo"] == "abc -1.1");
            test(ro[1]["bar"] == "abc 123123.2");

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0]["f00"] == "ABC -3.14");
            test(_do[1].size() == 2);
            test(_do[1]["foo"] == "abc -1.1");
            test(_do[1]["bar"] == "abc 123123.2");
            test(_do[2].size() == 3);
            test(_do[2]["foo"] == "abc -1.1");
            test(_do[2]["FOO"] == "abc -100.4");
            test(_do[2]["BAR"] == "abc 0.5");
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::StringMyEnumDS dsi1;
        dsi1.resize(2);
        Test::StringMyEnumDS dsi2;
        dsi2.resize(1);

        Test::StringMyEnumD di1;
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        Test::StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;
        Test::StringMyEnumD di3;
        di3["Goodbye"] = MyEnum::enum1;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        try
        {
            Test::StringMyEnumDS _do;
            Test::StringMyEnumDS ro = p->opStringMyEnumDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0]["abc"] == MyEnum::enum1);
            test(ro[0]["qwerty"] == MyEnum::enum3);
            test(ro[0]["Hello!!"] == MyEnum::enum2);
            test(ro[1].size() == 2);
            test(ro[1]["abc"] == MyEnum::enum1);
            test(ro[1][""] == MyEnum::enum2);

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0]["Goodbye"] == MyEnum::enum1);
            test(_do[1].size() == 2);
            test(_do[1]["abc"] == MyEnum::enum1);
            test(_do[1][""] == MyEnum::enum2);
            test(_do[2].size() == 3);
            test(_do[2]["abc"] == MyEnum::enum1);
            test(_do[2]["qwerty"] == MyEnum::enum3);
            test(_do[2]["Hello!!"] == MyEnum::enum2);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::MyEnumStringDS dsi1;
        dsi1.resize(2);
        Test::MyEnumStringDS dsi2;
        dsi2.resize(1);

        Test::MyEnumStringD di1;
        di1[MyEnum::enum1] = "abc";
        Test::MyEnumStringD di2;
        di2[MyEnum::enum2] = "Hello!!";
        di2[MyEnum::enum3] = "qwerty";
        Test::MyEnumStringD di3;
        di3[MyEnum::enum1] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        try
        {
            Test::MyEnumStringDS _do;
            Test::MyEnumStringDS ro = p->opMyEnumStringDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 2);
            test(ro[0][MyEnum::enum2] == "Hello!!");
            test(ro[0][MyEnum::enum3] == "qwerty");
            test(ro[1].size() == 1);
            test(ro[1][MyEnum::enum1] == "abc");

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0][MyEnum::enum1] == "Goodbye");
            test(_do[1].size() == 1);
            test(_do[1][MyEnum::enum1] == "abc");
            test(_do[2].size() == 2);
            test(_do[2][MyEnum::enum2] == "Hello!!");
            test(_do[2][MyEnum::enum3] == "qwerty");
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        Test::MyStructMyEnumDS dsi2;
        dsi2.resize(1);

        Test::MyStruct ms11 = {1, 1};
        Test::MyStruct ms12 = {1, 2};
        Test::MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        Test::MyStruct ms22 = {2, 2};
        Test::MyStruct ms23 = {2, 3};
        Test::MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        Test::MyStructMyEnumD di3;
        di3[ms23] = MyEnum::enum3;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        try
        {
            Test::MyStructMyEnumDS _do;
            Test::MyStructMyEnumDS ro = p->opMyStructMyEnumDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0][ms11] == MyEnum::enum1);
            test(ro[0][ms22] == MyEnum::enum3);
            test(ro[0][ms23] == MyEnum::enum2);
            test(ro[1].size() == 2);
            test(ro[1][ms11] == MyEnum::enum1);
            test(ro[1][ms12] == MyEnum::enum2);

            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0][ms23] == MyEnum::enum3);
            test(_do[1].size() == 2);
            test(_do[1][ms11] == MyEnum::enum1);
            test(_do[1][ms12] == MyEnum::enum2);
            test(_do[2].size() == 3);
            test(_do[2][ms11] == MyEnum::enum1);
            test(_do[2][ms22] == MyEnum::enum3);
            test(_do[2][ms23] == MyEnum::enum2);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::ByteByteSD sdi1;
        Test::ByteByteSD sdi2;

        Test::ByteS si1;
        Test::ByteS si2;
        Test::ByteS si3;

        si1.push_back(byte{0x01});
        si1.push_back(byte{0x11});
        si2.push_back(byte{0x12});
        si3.push_back(byte{0xf2});
        si3.push_back(byte{0xf3});

        sdi1[uint8_t{0x01}] = si1;
        sdi1[uint8_t{0x22}] = si2;
        sdi2[uint8_t{0xf1}] = si3;

        try
        {
            Test::ByteByteSD _do;
            Test::ByteByteSD ro = p->opByteByteSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro[uint8_t{0x01}].size() == 2);
            test(ro[uint8_t{0x01}][0] == byte{0x01});
            test(ro[uint8_t{0x01}][1] == byte{0x11});
            test(ro[uint8_t{0x22}].size() == 1);
            test(ro[uint8_t{0x22}][0] == byte{0x12});
            test(ro[uint8_t{0xf1}].size() == 2);
            test(ro[uint8_t{0xf1}][0] == byte{0xf2});
            test(ro[uint8_t{0xf1}][1] == byte{0xf3});
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::BoolBoolSD _do;
            Test::BoolBoolSD ro = p->opBoolBoolSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 2);
            test(ro[false].size() == 2);
            test(ro[false][0] == true);
            test(ro[false][1] == false);
            test(ro[true].size() == 3);
            test(ro[true][0] == false);
            test(ro[true][1] == true);
            test(ro[true][2] == true);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::ShortShortSD _do;
            Test::ShortShortSD ro = p->opShortShortSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro[1].size() == 3);
            test(ro[1][0] == 1);
            test(ro[1][1] == 2);
            test(ro[1][2] == 3);
            test(ro[2].size() == 2);
            test(ro[2][0] == 4);
            test(ro[2][1] == 5);
            test(ro[4].size() == 2);
            test(ro[4][0] == 6);
            test(ro[4][1] == 7);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::IntIntSD _do;
            Test::IntIntSD ro = p->opIntIntSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro[100].size() == 3);
            test(ro[100][0] == 100);
            test(ro[100][1] == 200);
            test(ro[100][2] == 300);
            test(ro[200].size() == 2);
            test(ro[200][0] == 400);
            test(ro[200][1] == 500);
            test(ro[400].size() == 2);
            test(ro[400][0] == 600);
            test(ro[400][1] == 700);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
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

        try
        {
            Test::LongLongSD _do;
            Test::LongLongSD ro = p->opLongLongSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro[999999990].size() == 3);
            test(ro[999999990][0] == 999999110);
            test(ro[999999990][1] == 999999111);
            test(ro[999999990][2] == 999999110);
            test(ro[999999991].size() == 2);
            test(ro[999999991][0] == 999999120);
            test(ro[999999991][1] == 999999130);
            test(ro[999999992].size() == 2);
            test(ro[999999992][0] == 999999110);
            test(ro[999999992][1] == 999999120);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::StringFloatSD sdi1;
        Test::StringFloatSD sdi2;

        Test::FloatS si1;
        Test::FloatS si2;
        Test::FloatS si3;

        si1.push_back(float(-1.1));
        si1.push_back(float(123123.2));
        si1.push_back(float(100.0));
        si2.push_back(float(42.24));
        si2.push_back(float(-1.61));
        si3.push_back(float(-3.14));
        si3.push_back(float(3.14));

        sdi1["abc"] = si1;
        sdi1["ABC"] = si2;
        sdi2["aBc"] = si3;

        try
        {
            Test::StringFloatSD _do;
            Test::StringFloatSD ro = p->opStringFloatSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro["abc"].size() == 3);
            test(ro["abc"][0] == float(-1.1));
            test(ro["abc"][1] == float(123123.2));
            test(ro["abc"][2] == float(100.0));
            test(ro["ABC"].size() == 2);
            test(ro["ABC"][0] == float(42.24));
            test(ro["ABC"][1] == float(-1.61));
            test(ro["aBc"].size() == 2);
            test(ro["aBc"][0] == float(-3.14));
            test(ro["aBc"][1] == float(3.14));
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::StringDoubleSD sdi1;
        Test::StringDoubleSD sdi2;

        Test::DoubleS si1;
        Test::DoubleS si2;
        Test::DoubleS si3;

        si1.push_back(double(1.1E10));
        si1.push_back(double(1.2E10));
        si1.push_back(double(1.3E10));
        si2.push_back(double(1.4E10));
        si2.push_back(double(1.5E10));
        si3.push_back(double(1.6E10));
        si3.push_back(double(1.7E10));

        sdi1["Hello!!"] = si1;
        sdi1["Goodbye"] = si2;
        sdi2[""] = si3;

        try
        {
            Test::StringDoubleSD _do;
            Test::StringDoubleSD ro = p->opStringDoubleSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro["Hello!!"].size() == 3);
            test(ro["Hello!!"][0] == double(1.1E10));
            test(ro["Hello!!"][1] == double(1.2E10));
            test(ro["Hello!!"][2] == double(1.3E10));
            test(ro["Goodbye"].size() == 2);
            test(ro["Goodbye"][0] == double(1.4E10));
            test(ro["Goodbye"][1] == double(1.5E10));
            test(ro[""].size() == 2);
            test(ro[""][0] == double(1.6E10));
            test(ro[""][1] == double(1.7E10));
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::StringStringSD sdi1;
        Test::StringStringSD sdi2;

        Test::StringS si1;
        Test::StringS si2;
        Test::StringS si3;

        si1.emplace_back("abc");
        si1.emplace_back("de");
        si1.emplace_back("fghi");

        si2.emplace_back("xyz");
        si2.emplace_back("or");

        si3.emplace_back("and");
        si3.emplace_back("xor");

        sdi1["abc"] = si1;
        sdi1["def"] = si2;
        sdi2["ghi"] = si3;

        try
        {
            Test::StringStringSD _do;
            Test::StringStringSD ro = p->opStringStringSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro["abc"].size() == 3);
            test(ro["abc"][0] == "abc");
            test(ro["abc"][1] == "de");
            test(ro["abc"][2] == "fghi");
            test(ro["def"].size() == 2);
            test(ro["def"][0] == "xyz");
            test(ro["def"][1] == "or");
            test(ro["ghi"].size() == 2);
            test(ro["ghi"][0] == "and");
            test(ro["ghi"][1] == "xor");
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        Test::MyEnumMyEnumSD sdi1;
        Test::MyEnumMyEnumSD sdi2;

        Test::MyEnumS si1;
        Test::MyEnumS si2;
        Test::MyEnumS si3;

        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum2);
        si2.push_back(MyEnum::enum1);
        si2.push_back(MyEnum::enum2);
        si3.push_back(MyEnum::enum3);
        si3.push_back(MyEnum::enum3);

        sdi1[MyEnum::enum3] = si1;
        sdi1[MyEnum::enum2] = si2;
        sdi2[MyEnum::enum1] = si3;

        try
        {
            Test::MyEnumMyEnumSD _do;
            Test::MyEnumMyEnumSD ro = p->opMyEnumMyEnumSD(sdi1, sdi2, _do);

            test(_do == sdi2);
            test(ro.size() == 3);
            test(ro[MyEnum::enum3].size() == 3);
            test(ro[MyEnum::enum3][0] == MyEnum::enum1);
            test(ro[MyEnum::enum3][1] == MyEnum::enum1);
            test(ro[MyEnum::enum3][2] == MyEnum::enum2);
            test(ro[MyEnum::enum2].size() == 2);
            test(ro[MyEnum::enum2][0] == MyEnum::enum1);
            test(ro[MyEnum::enum2][1] == MyEnum::enum2);
            test(ro[MyEnum::enum1].size() == 2);
            test(ro[MyEnum::enum1][0] == MyEnum::enum3);
            test(ro[MyEnum::enum1][1] == MyEnum::enum3);
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
    }

    {
        const int lengths[] = {0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};

        for (int length : lengths)
        {
            Test::IntS s;
            for (int i = 0; i < length; ++i)
            {
                s.push_back(i);
            }
            Test::IntS r = p->opIntS(s);
            test(r.size() == static_cast<size_t>(length));
            for (int j = 0; j < static_cast<int>(r.size()); ++j)
            {
                test(r[static_cast<size_t>(j)] == -j);
            }
        }
    }

    {
        {
            Ice::Context ctx;
            ctx["one"] = "ONE";
            ctx["two"] = "TWO";
            ctx["three"] = "THREE";
            {
                Ice::Context r = p->opContext();
                test(p->ice_getContext().empty());
                test(r != ctx);
            }
            {
                Ice::Context r = p->opContext(ctx);
                test(p->ice_getContext().empty());
                test(r == ctx);
            }
            {
                MyClassPrx p2 = p->ice_context(ctx);
                test(p2->ice_getContext() == ctx);
                Ice::Context r = p2->opContext();
                test(r == ctx);
                r = p2->opContext(ctx);
                test(r == ctx);
            }
        }

        if (p->ice_getConnection() && communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "bt")
        {
            //
            // Test implicit context propagation
            //

            string impls[] = {"Shared", "PerThread"};
            for (const auto& impl : impls)
            {
                Ice::InitializationData initData;
                initData.properties = communicator->getProperties()->clone();
                initData.properties->setProperty("Ice.ImplicitContext", impl);
                installTransport(initData);

                Ice::CommunicatorPtr ic = initialize(initData);

                Ice::Context ctx;
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";

                Ice::PropertiesPtr properties = ic->getProperties();
                Test::MyClassPrx q(ic, "test:" + TestHelper::getTestEndpoint(properties, 0));

                ic->getImplicitContext()->setContext(ctx);
                test(ic->getImplicitContext()->getContext() == ctx);
                test(q->opContext() == ctx);

                test(ic->getImplicitContext()->containsKey("zero") == false);
                string r = ic->getImplicitContext()->put("zero", "ZERO");
                test(r == "");
                test(ic->getImplicitContext()->containsKey("zero") == true);
                test(ic->getImplicitContext()->get("zero") == "ZERO");

                ctx = ic->getImplicitContext()->getContext();
                test(q->opContext() == ctx);
                Ice::Context prxContext;
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                Ice::Context combined = prxContext;
                combined.insert(ctx.begin(), ctx.end());
                test(combined["one"] == "UN");

                q = q->ice_context(prxContext);

                ic->getImplicitContext()->setContext(Ice::Context());
                test(q->opContext() == prxContext);

                ic->getImplicitContext()->setContext(ctx);
                test(q->opContext() == combined);

                test(ic->getImplicitContext()->remove("one") == "ONE");

                if (impl == "PerThread")
                {
                    auto invoker = make_shared<PerThreadContextInvokeThread>(q->ice_context(Ice::Context()));
                    auto invokerThread = std::thread([invoker] { invoker->run(); });
                    invokerThread.join();
                }

                ic->getImplicitContext()->setContext(Ice::Context()); // Clear the context to avoid leak report.
                ic->destroy();
            }
        }
    }

    {
        double d = 1278312346.0 / 13.0;
        Test::DoubleS ds(5, d);
        p->opDoubleMarshaling(d, ds);
    }

    p->opIdempotent();

    test(p->opByte1(0xFF) == 0xFF);
    test(p->opShort1(0x7FFF) == 0x7FFF);
    test(p->opInt1(0x7FFFFFFF) == 0x7FFFFFFF);
    test(p->opLong1(0x7FFFFFFFFFFFFFFFLL) == 0x7FFFFFFFFFFFFFFFLL);
    test(p->opFloat1(1.0) == 1.0);
    test(p->opDouble1(1.0) == 1.0);
    test(p->opString1("opString1") == "opString1");

    auto d = Ice::uncheckedCast<MyDerivedClassPrx>(p);

    Test::MyStruct1 s;
    s.tesT = "Test::MyStruct1::s";
    s.myClass = nullopt;
    s.myStruct1 = "Test::MyStruct1::myStruct1";
    s = d->opMyStruct1(s);
    test(s.tesT == "Test::MyStruct1::s");
    test(s.myClass == nullopt);
    test(s.myStruct1 == "Test::MyStruct1::myStruct1");

    Test::MyClass1Ptr c = make_shared<Test::MyClass1>();
    c->tesT = "Test::MyClass1::testT";
    c->myClass = nullopt;
    c->myClass1 = "Test::MyClass1::myClass1";
    c = d->opMyClass1(c);
    test(c->tesT == "Test::MyClass1::testT");
    test(c->myClass == nullopt);
    test(c->myClass1 == "Test::MyClass1::myClass1");

    Test::StringS seq;
    p->opStringS1(seq);

    Test::ByteBoolD dict;
    p->opByteBoolD1(dict);

    {
        Test::Structure p1 = p->opMStruct1();
        p1.e = MyEnum::enum3;
        Test::Structure p2, p3;
        p3 = p->opMStruct2(p1, p2);
        test(p2.e == p1.e && p3.e == p1.e);
    }

    {
        p->opMSeq1();

        StringS p1;
        p1.emplace_back("test");
        StringS p2, p3;
        p3 = p->opMSeq2(p1, p2);
        test(p2 == p1 && p3 == p1);
    }

    {
        p->opMDict1();

        map<string, string> p1;
        p1["test"] = "test";
        map<string, string> p2, p3;
        p3 = p->opMDict2(p1, p2);
        test(p2 == p1 && p3 == p1);
    }
}
