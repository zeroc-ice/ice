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

using namespace std;

void
exceptions(Test::MyClassPrx p)
{
    p->opEx(0);

    try
    {
	p->opEx(1);
	test(false);
    }
    catch(Ice::Byte ex)
    {
	test(ex == 0x1f);
    }

    try
    {
	p->opEx(2);
	test(false);
    }
    catch(bool ex)
    {
	test(ex == true);
    }

    try
    {
	p->opEx(3);
	test(false);
    }
    catch(Ice::Short ex)
    {
	test(ex == 123);
    }

    try
    {
	p->opEx(4);
	test(false);
    }
    catch(Ice::Int ex)
    {
	test(ex == 456);
    }

    try
    {
	p->opEx(5);
	test(false);
    }
    catch(Ice::Long ex)
    {
	test(ex == 789);
    }

    try
    {
	p->opEx(6);
	test(false);
    }
    catch(Ice::Float ex)
    {
	test(ex == Ice::Float(10.1112));
    }

    try
    {
	p->opEx(7);
	test(false);
    }
    catch(Ice::Double ex)
    {
	test(ex == Ice::Double(13.1415));
    }

    try
    {
	p->opEx(8);
	test(false);
    }
    catch(const string& ex)
    {
	test(ex == "abc");
    }

    try
    {
	p->opEx(9);
	test(false);
    }
    catch(const wstring& ex)
    {
	test(ex == L"def");
    }

    try
    {
	p->opEx(10);
	test(false);
    }
    catch(const Test::ByteS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Byte(0x0f));
	test(ex[1] == Ice::Byte(0xf0));
    }

    try
    {
	p->opEx(11);
	test(false);
    }
    catch(const Test::BoolS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == true);
	test(ex[1] == false);
    }

    try
    {
	p->opEx(12);
	test(false);
    }
    catch(const Test::ShortS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 1);
	test(ex[1] == 2);
    }

    try
    {
	p->opEx(13);
	test(false);
    }
    catch(const Test::IntS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 3);
	test(ex[1] == 4);
    }

    try
    {
	p->opEx(14);
	test(false);
    }
    catch(const Test::LongS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 5);
	test(ex[1] == 6);
    }

    try
    {
	p->opEx(15);
	test(false);
    }
    catch(const Test::FloatS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Float(7.7));
	test(ex[1] == Ice::Float(8.8));
    }

    try
    {
	p->opEx(16);
	test(false);
    }
    catch(const Test::DoubleS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Double(9.9));
	test(ex[1] == Ice::Double(10.10));
    }

    try
    {
	p->opEx(17);
	test(false);
    }
    catch(const Test::StringS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == "abc");
	test(ex[1] == "def");
    }

    try
    {
	p->opEx(18);
	test(false);
    }
    catch(const Test::WStringS& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == L"xxx");
	test(ex[1] == L"XXX");
    }

    try
    {
	p->opEx(19);
	test(false);
    }
    catch(const Test::ByteSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == Ice::Byte(0x01));
	test(ex[1][0] == Ice::Byte(0x10));
	test(ex[1][1] == Ice::Byte(0x11));
    }

    try
    {
	p->opEx(20);
	test(false);
    }
    catch(const Test::BoolSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == true);
	test(ex[1][0] == false);
	test(ex[1][1] == true);
    }

    try
    {
	p->opEx(21);
	test(false);
    }
    catch(const Test::ShortSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == 1);
	test(ex[1][0] == 2);
	test(ex[1][1] == 3);
    }

    try
    {
	p->opEx(22);
	test(false);
    }
    catch(const Test::IntSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == 4);
	test(ex[1][0] == 5);
	test(ex[1][1] == 6);
    }

    try
    {
	p->opEx(23);
	test(false);
    }
    catch(const Test::LongSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == 7);
	test(ex[1][0] == 8);
	test(ex[1][1] == 9);
    }

    try
    {
	p->opEx(24);
	test(false);
    }
    catch(const Test::FloatSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == Ice::Float(10.10));
	test(ex[1][0] == Ice::Float(11.11));
	test(ex[1][1] == Ice::Float(12.12));
    }

    try
    {
	p->opEx(25);
	test(false);
    }
    catch(const Test::DoubleSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == Ice::Double(13.13));
	test(ex[1][0] == Ice::Double(14.14));
	test(ex[1][1] == Ice::Double(15.15));
    }

    try
    {
	p->opEx(26);
	test(false);
    }
    catch(const Test::StringSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == "aaa");
	test(ex[1][0] == "bbb");
	test(ex[1][1] == "ccc");
    }

    try
    {
	p->opEx(27);
	test(false);
    }
    catch(const Test::WStringSS& ex)
    {
	test(ex.size() == 2);
	test(ex[0].size() == 1);
	test(ex[1].size() == 2);
	test(ex[0][0] == L"aaa");
	test(ex[1][0] == L"bbb");
	test(ex[1][1] == L"ccc");
    }

    try
    {
	p->opEx(28);
	test(false);
    }
    catch(const Test::ByteBoolD& ex)
    {
	test(ex.size() == 2);
	test(ex.count(0) == 1);
	test(ex.find(0)->second == true);
	test(ex.count(255) == 1);
	test(ex.find(255)->second == false);
    }

    try
    {
	p->opEx(29);
	test(false);
    }
    catch(const Test::ShortIntD& ex)
    {
	test(ex.size() == 3);
	test(ex.count(-10) == 1);
	test(ex.find(-10)->second == 10);
	test(ex.count(-20) == 1);
	test(ex.find(-20)->second == 20);
	test(ex.count(-30) == 1);
	test(ex.find(-30)->second == 30);
    }

    try
    {
	p->opEx(30);
	test(false);
    }
    catch(const Test::LongFloatD& ex)
    {
	test(ex.size() == 1);
	test(ex.count(0x7fffffffffffffff) == 1);
	test(ex.find(0x7fffffffffffffff)->second == Ice::Float(3.14));
    }

    try
    {
	p->opEx(31);
	test(false);
    }
    catch(const Test::DoubleStringD& ex)
    {
	test(ex.size() == 3);
	test(ex.count(-10.1E1) == 1);
	test(ex.find(-10.1E1)->second == "abc");
	test(ex.count(-20.2E10) == 1);
	test(ex.find(-20.2E10)->second == "def");
	test(ex.count(-30.3E100) == 1);
	test(ex.find(-30.3E100)->second == "ghi");
    }

    try
    {
	p->opEx(32);
	test(false);
    }
    catch(const Test::WStringMyEnumD& ex)
    {
	test(ex.size() == 1);
	test(ex.count(L"Hello") == 1);
	test(ex.find(L"Hello")->second == Test::enum2);
    }

    try
    {
	p->opEx(33);
	test(false);
    }
    catch(const Test::MyClassStringD& ex)
    {
	test(ex.size() == 2);
	test(ex.count(0) == 1);
	test(ex.find(0)->second == "null");
	test(ex.count(p) == 1);
	test(ex.find(p)->second == "MyClass");

	int i = 0;
	for (Test::MyClassStringD::const_iterator q = ex.begin(); q != ex.end(); ++q, ++i)
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

    try
    {
	p->opEx(34);
	test(false);
    }
    catch(const Test::MyClassPrxE& ex)
    {
	test(ex == p);
	ex->opVoid();
    }

}
