// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

    bool check()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	while(!_called)
	{
	    if(!timedWait(IceUtil::Time::seconds(5)))
	    {
		return false;
	    }
	}
	return true;
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

class MyClass_opVoidI : public Test::MyClass_opVoid, public CallbackBase
{
public:

    virtual void ice_response()
    {
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opVoidI> MyClass_opVoidIPtr;

class MyClass_opByteI : public Test::MyClass_opByte, public CallbackBase
{
public:

    virtual void ice_response(::Ice::Byte r, ::Ice::Byte b)
    {
	test(b == Ice::Byte(0xf0));
	test(r == Ice::Byte(0xff));
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opByteI> MyClass_opByteIPtr;

class MyClass_opBoolI : public Test::MyClass_opBool, public CallbackBase
{
public:

    virtual void ice_response(bool r, bool b)
    {
	test(b);
	test(!r);
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opBoolI> MyClass_opBoolIPtr;

class MyClass_opShortIntLongI : public Test::MyClass_opShortIntLong, public CallbackBase
{
public:

    virtual void ice_response(::Ice::Long r, ::Ice::Short s, ::Ice::Int i, ::Ice::Long l)
    {
	test(s == 10);
	test(i == 11);
	test(l == 12);
	test(r == 12);
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opShortIntLongI> MyClass_opShortIntLongIPtr;

class MyClass_opFloatDoubleI : public Test::MyClass_opFloatDouble, public CallbackBase
{
public:

    virtual void ice_response(::Ice::Double r, ::Ice::Float f, ::Ice::Double d)
    {
	test(f == Ice::Float(3.14));
	test(d == Ice::Double(1.1E10));
	test(r == Ice::Double(1.1E10));
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opFloatDoubleI> MyClass_opFloatDoubleIPtr;

class MyClass_opStringI : public Test::MyClass_opString, public CallbackBase
{
public:

    virtual void ice_response(const ::std::string& r, const ::std::string& s)
    {
	test(s == "world hello");
	test(r == "hello world");
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStringI> MyClass_opStringIPtr;

class MyClass_opMyEnumI : public Test::MyClass_opMyEnum, public CallbackBase
{
public:

    virtual void ice_response(::Test::MyEnum r, ::Test::MyEnum e)
    {
	test(e == Test::enum2);
	test(r == Test::enum3);
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opMyEnumI> MyClass_opMyEnumIPtr;

class MyClass_opMyClassI : public Test::MyClass_opMyClass, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::MyClassPrx& r, const ::Test::MyClassPrx& c1, const ::Test::MyClassPrx& c2)
    {
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
	catch(const Ice::ObjectNotExistException&)
	{
	}
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opMyClassI> MyClass_opMyClassIPtr;

class MyClass_opStructI : public Test::MyClass_opStruct, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::Structure& rso, const ::Test::Structure& so)
    {
	test(rso.p == 0);
	test(rso.e == Test::enum2);
	test(rso.s.s == "def");
	test(so.e == Test::enum3);
	test(so.s.s == "a new string");
	so.p->opVoid();
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStructI> MyClass_opStructIPtr;

class MyClass_opByteSI : public Test::MyClass_opByteS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::ByteS& rso, const ::Test::ByteS& bso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opByteSI> MyClass_opByteSIPtr;

class MyClass_opBoolSI : public Test::MyClass_opBoolS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::BoolS& rso, const ::Test::BoolS& bso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opBoolSI> MyClass_opBoolSIPtr;

class MyClass_opShortIntLongSI : public Test::MyClass_opShortIntLongS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::LongS& rso, const ::Test::ShortS& sso, const ::Test::IntS& iso,
			      const ::Test::LongS& lso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opShortIntLongSI> MyClass_opShortIntLongSIPtr;

class MyClass_opFloatDoubleSI : public Test::MyClass_opFloatDoubleS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::DoubleS& rso, const ::Test::FloatS& fso, const ::Test::DoubleS& dso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opFloatDoubleSI> MyClass_opFloatDoubleSIPtr;

class MyClass_opStringSI : public Test::MyClass_opStringS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::StringS& rso, const ::Test::StringS& sso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStringSI> MyClass_opStringSIPtr;

class MyClass_opByteSSI : public Test::MyClass_opByteSS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::ByteSS& rso, const ::Test::ByteSS& bso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opByteSSI> MyClass_opByteSSIPtr;

class MyClass_opBoolSSI : public Test::MyClass_opBoolSS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::BoolSS&, const ::Test::BoolSS& bso)
    {
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opBoolSSI> MyClass_opBoolSSIPtr;

class MyClass_opShortIntLongSSI : public Test::MyClass_opShortIntLongSS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::LongSS&, const ::Test::ShortSS&, const ::Test::IntSS&,
			      const ::Test::LongSS&)
    {
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opShortIntLongSSI> MyClass_opShortIntLongSSIPtr;

class MyClass_opFloatDoubleSSI : public Test::MyClass_opFloatDoubleSS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::DoubleSS& rso, const ::Test::FloatSS& fso, const ::Test::DoubleSS& dso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opFloatDoubleSSI> MyClass_opFloatDoubleSSIPtr;

class MyClass_opStringSSI : public Test::MyClass_opStringSS, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::StringSS& rso, const ::Test::StringSS& sso)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStringSSI> MyClass_opStringSSIPtr;

class MyClass_opByteBoolDI : public Test::MyClass_opByteBoolD, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::ByteBoolD& ro, const ::Test::ByteBoolD& _do)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opByteBoolDI> MyClass_opByteBoolDIPtr;

class MyClass_opShortIntDI : public Test::MyClass_opShortIntD, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::ShortIntD& ro, const ::Test::ShortIntD& _do)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opShortIntDI> MyClass_opShortIntDIPtr;

class MyClass_opLongFloatDI : public Test::MyClass_opLongFloatD, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::LongFloatD& ro, const ::Test::LongFloatD& _do)
    {
	Test::LongFloatD di1;
	di1[999999110] = Ice::Float(-1.1);
	di1[9999991100] = Ice::Float(123123.2);
	test(_do == di1);
	test(ro.size() == 4);
	test(ro.find(999999110) != ro.end());
	test(ro.find(999999110)->second == Ice::Float(-1.1));
	test(ro.find(999999111) != ro.end());
	test(ro.find(999999111)->second == Ice::Float(-100.4));
	test(ro.find(9999991100) != ro.end());
	test(ro.find(9999991100)->second == Ice::Float(123123.2));
	test(ro.find(9999991101) != ro.end());
	test(ro.find(9999991101)->second == Ice::Float(0.5));
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opLongFloatDI> MyClass_opLongFloatDIPtr;

class MyClass_opStringStringDI : public Test::MyClass_opStringStringD, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::StringStringD& ro, const ::Test::StringStringD& _do)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStringStringDI> MyClass_opStringStringDIPtr;

class MyClass_opStringMyEnumDI : public Test::MyClass_opStringMyEnumD, public CallbackBase
{
public:

    virtual void ice_response(const ::Test::StringMyEnumD& ro, const ::Test::StringMyEnumD& _do)
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

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyClass_opStringMyEnumDI> MyClass_opStringMyEnumDIPtr;

class MyDerivedClass_opDerivedI : public Test::MyDerivedClass_opDerived, public CallbackBase
{
public:

    virtual void ice_response()
    {
	called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
	test(false);
    }
};

typedef IceUtil::Handle<MyDerivedClass_opDerivedI> MyDerivedClass_opDerivedIPtr;

void
twowaysAMI(const Test::MyClassPrx& p)
{
    {
	MyClass_opVoidIPtr cb = new MyClass_opVoidI;
	p->opVoid_async(cb);
	test(cb->check());
    }

    {
	MyClass_opByteIPtr cb = new MyClass_opByteI;
	p->opByte_async(cb, Ice::Byte(0xff), Ice::Byte(0x0f));
	test(cb->check());
    }

    {
	MyClass_opBoolIPtr cb = new MyClass_opBoolI;
	p->opBool_async(cb, true, false);
	test(cb->check());
    }

    {
	MyClass_opShortIntLongIPtr cb = new MyClass_opShortIntLongI;
	p->opShortIntLong_async(cb, 10, 11, 12);
	test(cb->check());
    }

    {
	MyClass_opFloatDoubleIPtr cb = new MyClass_opFloatDoubleI;
	p->opFloatDouble_async(cb, Ice::Float(3.14), Ice::Double(1.1E10));
	test(cb->check());
    }

    {
	MyClass_opStringIPtr cb = new MyClass_opStringI;
	p->opString_async(cb, "hello", "world");
	test(cb->check());
    }

    {
	MyClass_opMyEnumIPtr cb = new MyClass_opMyEnumI;
	p->opMyEnum_async(cb, Test::enum2);
	test(cb->check());
    }

    {
	MyClass_opMyClassIPtr cb = new MyClass_opMyClassI;
	p->opMyClass_async(cb, p);
	test(cb->check());
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
	
	MyClass_opStructIPtr cb = new MyClass_opStructI;
	p->opStruct_async(cb, si1, si2);
	test(cb->check());
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

	MyClass_opByteSIPtr cb = new MyClass_opByteSI;
	p->opByteS_async(cb, bsi1, bsi2);
	test(cb->check());
    }

    {
	Test::BoolS bsi1;
	Test::BoolS bsi2;

	bsi1.push_back(true);
	bsi1.push_back(true);
	bsi1.push_back(false);

	bsi2.push_back(false);

	MyClass_opBoolSIPtr cb = new MyClass_opBoolSI;
	p->opBoolS_async(cb, bsi1, bsi2);
	test(cb->check());
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

	MyClass_opShortIntLongSIPtr cb = new MyClass_opShortIntLongSI;
	p->opShortIntLongS_async(cb, ssi, isi, lsi);
	test(cb->check());
    }

    {
	Test::FloatS fsi;
	Test::DoubleS dsi;

	fsi.push_back(Ice::Float(3.14));
	fsi.push_back(Ice::Float(1.11));

	dsi.push_back(Ice::Double(1.1E10));
	dsi.push_back(Ice::Double(1.2E10));
	dsi.push_back(Ice::Double(1.3E10));

	MyClass_opFloatDoubleSIPtr cb = new MyClass_opFloatDoubleSI;
	p->opFloatDoubleS_async(cb, fsi, dsi);
	test(cb->check());
    }

    {
	Test::StringS ssi1;
	Test::StringS ssi2;

	ssi1.push_back("abc");
	ssi1.push_back("de");
	ssi1.push_back("fghi");

	ssi2.push_back("xyz");

	MyClass_opStringSIPtr cb = new MyClass_opStringSI;
	p->opStringS_async(cb, ssi1, ssi2);
	test(cb->check());
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

	MyClass_opByteSSIPtr cb = new MyClass_opByteSSI;
	p->opByteSS_async(cb, bsi1, bsi2);
	test(cb->check());
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

	MyClass_opFloatDoubleSSIPtr cb = new MyClass_opFloatDoubleSSI;
	p->opFloatDoubleSS_async(cb, fsi, dsi);
	test(cb->check());
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

	MyClass_opStringSSIPtr cb = new MyClass_opStringSSI;
	p->opStringSS_async(cb, ssi1, ssi2);
	test(cb->check());
    }

    {
	Test::ByteBoolD di1;
	di1[10] = true;
	di1[100] = false;
	Test::ByteBoolD di2;
	di2[10] = true;
	di2[11] = false;
	di2[101] = true;

	MyClass_opByteBoolDIPtr cb = new MyClass_opByteBoolDI;
	p->opByteBoolD_async(cb, di1, di2);
	test(cb->check());
    }

    {
	Test::ShortIntD di1;
	di1[110] = -1;
	di1[1100] = 123123;
	Test::ShortIntD di2;
	di2[110] = -1;
	di2[111] = -100;
	di2[1101] = 0;

	MyClass_opShortIntDIPtr cb = new MyClass_opShortIntDI;
	p->opShortIntD_async(cb, di1, di2);
	test(cb->check());
    }

    {
	Test::LongFloatD di1;
	di1[999999110] = Ice::Float(-1.1);
	di1[9999991100] = Ice::Float(123123.2);
	Test::LongFloatD di2;
	di2[999999110] = Ice::Float(-1.1);
	di2[999999111] = Ice::Float(-100.4);
	di2[9999991101] = Ice::Float(0.5);

	MyClass_opLongFloatDIPtr cb = new MyClass_opLongFloatDI;
	p->opLongFloatD_async(cb, di1, di2);
	test(cb->check());
    }

    {
	Test::StringStringD di1;
	di1["foo"] = "abc -1.1";
	di1["bar"] = "abc 123123.2";
	Test::StringStringD di2;
	di2["foo"] = "abc -1.1";
	di2["FOO"] = "abc -100.4";
	di2["BAR"] = "abc 0.5";

	MyClass_opStringStringDIPtr cb = new MyClass_opStringStringDI;
	p->opStringStringD_async(cb, di1, di2);
	test(cb->check());
    }

    {
	Test::StringMyEnumD di1;
	di1["abc"] = Test::enum1;
	di1[""] = Test::enum2;
	Test::StringMyEnumD di2;
	di2["abc"] = Test::enum1;
	di2["qwerty"] = Test::enum3;
	di2["Hello!!"] = Test::enum2;

	MyClass_opStringMyEnumDIPtr cb = new MyClass_opStringMyEnumDI;
	p->opStringMyEnumD_async(cb, di1, di2);
	test(cb->check());
    }

    {
	Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(p);
	test(derived);
	MyDerivedClass_opDerivedIPtr cb = new MyDerivedClass_opDerivedI;
	derived->opDerived_async(cb);
	test(cb->check());
    }

}
