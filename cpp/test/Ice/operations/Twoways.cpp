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
	Test::ByteS bvi1;
	Test::ByteS bvi2;

	bvi1.push_back(Ice::Byte(0x01));
	bvi1.push_back(Ice::Byte(0x11));
	bvi1.push_back(Ice::Byte(0x12));
	bvi1.push_back(Ice::Byte(0x22));

	bvi2.push_back(Ice::Byte(0xf1));
	bvi2.push_back(Ice::Byte(0xf2));
	bvi2.push_back(Ice::Byte(0xf3));
	bvi2.push_back(Ice::Byte(0xf4));

	Test::ByteS bvo;
	Test::ByteS rvo;

	rvo = p->opByteS(bvi1, bvi2, bvo);
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
	Test::BoolS bvi1;
	Test::BoolS bvi2;

	bvi1.push_back(true);
	bvi1.push_back(true);
	bvi1.push_back(false);

	bvi2.push_back(false);

	Test::BoolS bvo;
	Test::BoolS rvo;

	rvo = p->opBoolS(bvi1, bvi2, bvo);
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
	Test::ShortS svi;
	Test::IntS ivi;
	Test::LongS lvi;

	svi.push_back(1);
	svi.push_back(2);
	svi.push_back(3);

	ivi.push_back(5);
	ivi.push_back(6);
	ivi.push_back(7);
	ivi.push_back(8);

	lvi.push_back(10);
	lvi.push_back(30);
	lvi.push_back(20);

	Test::ShortS svo;
	Test::IntS ivo;
	Test::LongS lvo;
	Test::LongS rvo;

	rvo = p->opShortIntLongS(svi, ivi, lvi, svo, ivo, lvo);
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
	Test::FloatS fvi;
	Test::DoubleS dvi;

	fvi.push_back(Ice::Float(3.14));
	fvi.push_back(Ice::Float(1.11));

	dvi.push_back(Ice::Double(1.1E10));
	dvi.push_back(Ice::Double(1.2E10));
	dvi.push_back(Ice::Double(1.3E10));

	Test::FloatS fvo;
	Test::DoubleS dvo;
	Test::DoubleS rvo;

	rvo = p->opFloatDoubleS(fvi, dvi, fvo, dvo);
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
	Test::StringS svi1;
	Test::StringS svi2;

	svi1.push_back("abc");
	svi1.push_back("de");
	svi1.push_back("fghi");

	svi2.push_back("xyz");

	Test::StringS svo;
	Test::StringS rvo;

	rvo = p->opStringS(svi1, svi2, svo);
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
	Test::WStringS svi1;
	Test::WStringS svi2;

	svi1.push_back(L"abc");
	svi1.push_back(L"de");
	svi1.push_back(L"fghi");

	svi2.push_back(L"xyz");

	Test::WStringS svo;
	Test::WStringS rvo;

	rvo = p->opWStringS(svi1, svi2, svo);
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
	Test::ByteSS bvi1;
	bvi1.resize(2);
	Test::ByteSS bvi2;
	bvi2.resize(2);

	bvi1[0].push_back(Ice::Byte(0x01));
	bvi1[0].push_back(Ice::Byte(0x11));
	bvi1[0].push_back(Ice::Byte(0x12));
	bvi1[1].push_back(Ice::Byte(0xff));

	bvi2[0].push_back(Ice::Byte(0x0e));
	bvi2[1].push_back(Ice::Byte(0xf2));
	bvi2[1].push_back(Ice::Byte(0xf1));

	Test::ByteSS bvo;
	Test::ByteSS rvo;

	rvo = p->opByteSS(bvi1, bvi2, bvo);
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
	Test::FloatSS fvi;
	fvi.resize(3);
	Test::DoubleSS dvi;
	dvi.resize(1);

	fvi[0].push_back(Ice::Float(3.14));
	fvi[1].push_back(Ice::Float(1.11));

	dvi[0].push_back(Ice::Double(1.1E10));
	dvi[0].push_back(Ice::Double(1.2E10));
	dvi[0].push_back(Ice::Double(1.3E10));

	Test::FloatSS fvo;
	Test::DoubleSS dvo;
	Test::DoubleSS rvo;

	rvo = p->opFloatDoubleSS(fvi, dvi, fvo, dvo);
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
	Test::StringSS svi1;
	svi1.resize(2);
	Test::StringSS svi2;
	svi2.resize(3);

	svi1[0].push_back("abc");
	svi1[1].push_back("de");
	svi1[1].push_back("fghi");

	svi2[2].push_back("xyz");

	Test::StringSS svo;
	Test::StringSS rvo;

	rvo = p->opStringSS(svi1, svi2, svo);
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
	Test::WStringSS svi1;
	svi1.resize(2);
	Test::WStringSS svi2;
	svi2.resize(3);

	svi1[0].push_back(L"abc");
	svi1[1].push_back(L"de");
	svi1[1].push_back(L"fghi");

	svi2[2].push_back(L"xyz");

	Test::WStringSS svo;
	Test::WStringSS rvo;

	rvo = p->opWStringSS(svi1, svi2, svo);
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
}
