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
twoways(Test::MyClassPrx p)
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
	Test::MyClassPrx c;
	Test::MyClassPrx r;
	
	r = p->opMyClass(p, c);
	test(c == p);
	test(r == p);
	c->opVoid();
	r->opVoid();

	r = p->opMyClass(0, c);
	test(c == 0);
	test(r == p);
	r->opVoid();
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

	Test::ByteS bvo;
	Test::ByteS rvo;

	rvo = p->opByteS(bsi1, bsi2, bvo);
	test(bvo.size() == 4);
	test(bvo[0] == Ice::Byte(0x22));
	test(bvo[1] == Ice::Byte(0x12));
	test(bvo[2] == Ice::Byte(0x11));
	test(bvo[3] == Ice::Byte(0x01));
	test(rvo.size() == 8);
	test(rvo[0] == Ice::Byte(0x01));
	test(rvo[1] == Ice::Byte(0x11));
	test(rvo[2] == Ice::Byte(0x12));
	test(rvo[3] == Ice::Byte(0x22));
	test(rvo[4] == Ice::Byte(0xf1));
	test(rvo[5] == Ice::Byte(0xf2));
	test(rvo[6] == Ice::Byte(0xf3));
	test(rvo[7] == Ice::Byte(0xf4));
    }

    {
	Test::BoolS bsi1;
	Test::BoolS bsi2;

	bsi1.push_back(true);
	bsi1.push_back(true);
	bsi1.push_back(false);

	bsi2.push_back(false);

	Test::BoolS bvo;
	Test::BoolS rvo;

	rvo = p->opBoolS(bsi1, bsi2, bvo);
	test(bvo.size() == 4);
	test(bvo[0]);
	test(bvo[1]);
	test(!bvo[2]);
	test(!bvo[3]);
	test(rvo.size() == 3);
	test(!rvo[0]);
	test(rvo[1]);
	test(rvo[2]);
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

	Test::ShortS svo;
	Test::IntS ivo;
	Test::LongS lvo;
	Test::LongS rvo;

	rvo = p->opShortIntLongS(ssi, isi, lsi, svo, ivo, lvo);
	test(svo.size() == 3);
	test(svo[0] == 1);
	test(svo[1] == 2);
	test(svo[2] == 3);
	test(ivo.size() == 4);
	test(ivo[0] == 8);
	test(ivo[1] == 7);
	test(ivo[2] == 6);
	test(ivo[3] == 5);
	test(lvo.size() == 6);
	test(lvo[0] == 10);
	test(lvo[1] == 30);
	test(lvo[2] == 20);
	test(lvo[3] == 10);
	test(lvo[4] == 30);
	test(lvo[5] == 20);
	test(rvo.size() == 3);
	test(rvo[0] == 10);
	test(rvo[1] == 30);
	test(rvo[2] == 20);
    }

    {
	Test::FloatS fsi;
	Test::DoubleS dsi;

	fsi.push_back(Ice::Float(3.14));
	fsi.push_back(Ice::Float(1.11));

	dsi.push_back(Ice::Double(1.1E10));
	dsi.push_back(Ice::Double(1.2E10));
	dsi.push_back(Ice::Double(1.3E10));

	Test::FloatS fvo;
	Test::DoubleS dvo;
	Test::DoubleS rvo;

	rvo = p->opFloatDoubleS(fsi, dsi, fvo, dvo);
	test(fvo.size() == 2);
	test(fvo[0] == ::Ice::Float(3.14));
	test(fvo[1] == ::Ice::Float(1.11));
	test(dvo.size() == 3);
	test(dvo[0] == ::Ice::Double(1.3E10));
	test(dvo[1] == ::Ice::Double(1.2E10));
	test(dvo[2] == ::Ice::Double(1.1E10));
	test(rvo.size() == 5);
	test(rvo[0] == ::Ice::Double(1.1E10));
	test(rvo[1] == ::Ice::Double(1.2E10));
	test(rvo[2] == ::Ice::Double(1.3E10));
	test(::Ice::Float(rvo[3]) == ::Ice::Float(3.14));
	test(::Ice::Float(rvo[4]) == ::Ice::Float(1.11));
    }

    {
	Test::StringS ssi1;
	Test::StringS ssi2;

	ssi1.push_back("abc");
	ssi1.push_back("de");
	ssi1.push_back("fghi");

	ssi2.push_back("xyz");

	Test::StringS svo;
	Test::StringS rvo;

	rvo = p->opStringS(ssi1, ssi2, svo);
	test(svo.size() == 4);
	test(svo[0] == "abc");
	test(svo[1] == "de");
	test(svo[2] == "fghi");
	test(svo[3] == "xyz");
	test(rvo.size() == 3);
	test(rvo[0] == "fghi");
	test(rvo[1] == "de");
	test(rvo[2] == "abc");
    }

    {
	Test::WStringS ssi1;
	Test::WStringS ssi2;

	ssi1.push_back(L"abc");
	ssi1.push_back(L"de");
	ssi1.push_back(L"fghi");

	ssi2.push_back(L"xyz");

	Test::WStringS svo;
	Test::WStringS rvo;

	rvo = p->opWStringS(ssi1, ssi2, svo);
	test(svo.size() == 4);
	test(svo[0] == L"abc");
	test(svo[1] == L"de");
	test(svo[2] == L"fghi");
	test(svo[3] == L"xyz");
	test(rvo.size() == 3);
	test(rvo[0] == L"fghi");
	test(rvo[1] == L"de");
	test(rvo[2] == L"abc");
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

	Test::ByteSS bvo;
	Test::ByteSS rvo;

	rvo = p->opByteSS(bsi1, bsi2, bvo);
	test(bvo.size() == 2);
	test(bvo[0].size() == 1);
	test(bvo[0][0] == Ice::Byte(0xff));
	test(bvo[1].size() == 3);
	test(bvo[1][0] == Ice::Byte(0x01));
	test(bvo[1][1] == Ice::Byte(0x11));
	test(bvo[1][2] == Ice::Byte(0x12));
	test(rvo.size() == 4);
	test(rvo[0].size() == 3);
	test(rvo[0][0] == Ice::Byte(0x01));
	test(rvo[0][1] == Ice::Byte(0x11));
	test(rvo[0][2] == Ice::Byte(0x12));
	test(rvo[1].size() == 1);
	test(rvo[1][0] == Ice::Byte(0xff));
	test(rvo[2].size() == 1);
	test(rvo[2][0] == Ice::Byte(0x0e));
	test(rvo[3].size() == 2);
	test(rvo[3][0] == Ice::Byte(0xf2));
	test(rvo[3][1] == Ice::Byte(0xf1));
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

	Test::FloatSS fvo;
	Test::DoubleSS dvo;
	Test::DoubleSS rvo;

	rvo = p->opFloatDoubleSS(fsi, dsi, fvo, dvo);
	test(fvo.size() == 3);
	test(fvo[0].size() == 1);
	test(fvo[0][0] == ::Ice::Float(3.14));
	test(fvo[1].size() == 1);
	test(fvo[1][0] == ::Ice::Float(1.11));
	test(fvo[2].size() == 0);
	test(dvo.size() == 1);
	test(dvo[0].size() == 3);
	test(dvo[0][0] == ::Ice::Double(1.1E10));
	test(dvo[0][1] == ::Ice::Double(1.2E10));
	test(dvo[0][2] == ::Ice::Double(1.3E10));
	test(rvo.size() == 2);
	test(rvo[0].size() == 3);
	test(rvo[0][0] == ::Ice::Double(1.1E10));
	test(rvo[0][1] == ::Ice::Double(1.2E10));
	test(rvo[0][2] == ::Ice::Double(1.3E10));
	test(rvo[1].size() == 3);
	test(rvo[1][0] == ::Ice::Double(1.1E10));
	test(rvo[1][1] == ::Ice::Double(1.2E10));
	test(rvo[1][2] == ::Ice::Double(1.3E10));
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

	Test::StringSS svo;
	Test::StringSS rvo;

	rvo = p->opStringSS(ssi1, ssi2, svo);
	test(svo.size() == 5);
	test(svo[0].size() == 1);
	test(svo[0][0] == "abc");
	test(svo[1].size() == 2);
	test(svo[1][0] == "de");
	test(svo[1][1] == "fghi");
	test(svo[2].size() == 0);
	test(svo[3].size() == 0);
	test(svo[4].size() == 1);
	test(svo[4][0] == "xyz");
	test(rvo.size() == 3);
	test(rvo[0].size() == 1);
	test(rvo[0][0] == "xyz");
	test(rvo[1].size() == 0);
	test(rvo[2].size() == 0);
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

	Test::WStringSS svo;
	Test::WStringSS rvo;

	rvo = p->opWStringSS(ssi1, ssi2, svo);
	test(svo.size() == 5);
	test(svo[0].size() == 1);
	test(svo[0][0] == L"abc");
	test(svo[1].size() == 2);
	test(svo[1][0] == L"de");
	test(svo[1][1] == L"fghi");
	test(svo[2].size() == 0);
	test(svo[3].size() == 0);
	test(svo[4].size() == 1);
	test(svo[4][0] == L"xyz");
	test(rvo.size() == 3);
	test(rvo[0].size() == 1);
	test(rvo[0][0] == L"xyz");
	test(rvo[1].size() == 0);
	test(rvo[2].size() == 0);
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
