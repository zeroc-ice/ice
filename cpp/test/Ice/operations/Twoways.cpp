// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <limits>

//
// Stupid Visual C++ defines min and max as macros :-(
//
#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

using namespace std;

void
twoways(const Test::MyClassPrx& p)
{
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
			      numeric_limits<Ice::Long>::max(),	s, i, l);
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
	wstring s;
	wstring r;

	r = p->opWString(L"hello", L"world", s);
	test(s == L"world hello");
	test(r == L"hello world");
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
	test(c1 == p);
	test(c2 != p);
	test(r == p);
	test(c1->_ice_getIdentity() == "test");
	test(c2->_ice_getIdentity() == "noSuchIdentity");
	test(r->_ice_getIdentity() == "test");
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
	test(r == p);
	r->opVoid();
// TODO: For some reasons this test sometimes doesn't work on linux
#ifdef WIN32
	try
	{
	    c1->opVoid();
	    test(false);
	}
	catch(const IceUtil::NullHandleException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	try
	{
	    c1->opVoid();
	    test(false);
	}
	catch(const Ice::Exception&)
	{
	}
	catch(...)
	{
	    test(false);
	}
#endif
    }


    {
	Test::Struct si1;
	si1.p = p;
	si1.e = Test::enum3;
	si1.s.s = "abc";
	Test::Struct si2;
	si2.p = 0;
	si2.e = Test::enum2;
	si2.s.s = "def";
	
	Test::Struct so;
	Test::Struct rso = p->opStruct(si1, si2, so);
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
	Test::WStringS ssi1;
	Test::WStringS ssi2;

	ssi1.push_back(L"abc");
	ssi1.push_back(L"de");
	ssi1.push_back(L"fghi");

	ssi2.push_back(L"xyz");

	Test::WStringS sso;
	Test::WStringS rso;

	rso = p->opWStringS(ssi1, ssi2, sso);
	test(sso.size() == 4);
	test(sso[0] == L"abc");
	test(sso[1] == L"de");
	test(sso[2] == L"fghi");
	test(sso[3] == L"xyz");
	test(rso.size() == 3);
	test(rso[0] == L"fghi");
	test(rso[1] == L"de");
	test(rso[2] == L"abc");
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
	Test::WStringSS ssi1;
	ssi1.resize(2);
	Test::WStringSS ssi2;
	ssi2.resize(3);

	ssi1[0].push_back(L"abc");
	ssi1[1].push_back(L"de");
	ssi1[1].push_back(L"fghi");

	ssi2[2].push_back(L"xyz");

	Test::WStringSS sso;
	Test::WStringSS rso;

	rso = p->opWStringSS(ssi1, ssi2, sso);
	test(sso.size() == 5);
	test(sso[0].size() == 1);
	test(sso[0][0] == L"abc");
	test(sso[1].size() == 2);
	test(sso[1][0] == L"de");
	test(sso[1][1] == L"fghi");
	test(sso[2].size() == 0);
	test(sso[3].size() == 0);
	test(sso[4].size() == 1);
	test(sso[4][0] == L"xyz");
	test(rso.size() == 3);
	test(rso[0].size() == 1);
	test(rso[0][0] == L"xyz");
	test(rso[1].size() == 0);
	test(rso[2].size() == 0);
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
	di1[9999991100] = Ice::Float(123123.2);
	Test::LongFloatD di2;
	di2[999999110] = Ice::Float(-1.1);
	di2[999999111] = Ice::Float(-100.4);
	di2[9999991101] = Ice::Float(0.5);

	Test::LongFloatD _do;
	Test::LongFloatD ro = p->opLongFloatD(di1, di2, _do);

	test(_do == di1);
	test(ro.size() == 4);
	test(ro[999999110] == Ice::Float(-1.1));
	test(ro[999999111] == Ice::Float(-100.4));
	test(ro[9999991100] == Ice::Float(123123.2));
	test(ro[9999991101] == Ice::Float(0.5));
    }

    {
	Test::DoubleStringD di1;
	di1[999999110.10E10] = "abc -1.1";
	di1[9999991100.10E10] = "abc 123123.2";
	Test::DoubleStringD di2;
	di2[999999110.10E10] = "abc -1.1";
	di2[999999111.10E10] = "abc -100.4";
	di2[9999991101.10E10] = "abc 0.5";

	Test::DoubleStringD _do;
	Test::DoubleStringD ro = p->opDoubleStringD(di1, di2, _do);

	test(_do == di1);
	test(ro.size() == 4);
	test(ro[999999110.10E10] == "abc -1.1");
	test(ro[999999111.10E10] == "abc -100.4");
	test(ro[9999991100.10E10] == "abc 123123.2");
	test(ro[9999991101.10E10] == "abc 0.5");
    }

    {
	Test::WStringMyEnumD di1;
	di1[L"abc"] = Test::enum1;
	di1[L""] = Test::enum2;
	Test::WStringMyEnumD di2;
	di2[L"abc"] = Test::enum1;
	di2[L"qwerty"] = Test::enum3;
	di2[L"Hello!!"] = Test::enum2;

	Test::WStringMyEnumD _do;
	Test::WStringMyEnumD ro = p->opWStringMyEnumD(di1, di2, _do);

	test(_do == di1);
	test(ro.size() == 4);
	test(ro[L"abc"] == Test::enum1);
	test(ro[L"qwerty"] == Test::enum3);
	test(ro[L""] == Test::enum2);
	test(ro[L"Hello!!"] == Test::enum2);
    }

    {
	Test::MyClassStringD di1;
	di1[p] = "abc";
	di1[0] = "def";
	Test::MyClassStringD di2;
	di2[p] = "abc";

	Test::MyClassStringD _do;
	Test::MyClassStringD ro = p->opMyClassStringD(di1, di2, _do);

	test(_do == di1);
	test(ro.size() == 2);
	test(ro[p] == "abc");
	test(ro[0] == "def");

	int i = 0;
	for (Test::MyClassStringD::iterator q = ro.begin(); q != ro.end(); ++q, ++i)
	{
	    test(i < 2);

	    if (i == 0)
	    {
		test(!q->first);
	    }
	    else
	    {
		test(q->first);
		q->first->opVoid();
	    }
	}
    }
}
