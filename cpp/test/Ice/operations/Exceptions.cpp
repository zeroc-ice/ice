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
    }
    catch(bool ex)
    {
	test(ex == true);
    }

    try
    {
	p->opEx(3);
    }
    catch(Ice::Short ex)
    {
	test(ex == 123);
    }

    try
    {
	p->opEx(4);
    }
    catch(Ice::Int ex)
    {
	test(ex == 456);
    }

    try
    {
	p->opEx(5);
    }
    catch(Ice::Long ex)
    {
	test(ex == 789);
    }

    try
    {
	p->opEx(6);
    }
    catch(Ice::Float ex)
    {
	test(ex == Ice::Float(10.1112));
    }

    try
    {
	p->opEx(7);
    }
    catch(Ice::Double ex)
    {
	test(ex == Ice::Double(13.1415));
    }

    try
    {
	p->opEx(8);
    }
    catch(const string& ex)
    {
	test(ex == "abc");
    }

    try
    {
	p->opEx(9);
    }
    catch(const wstring& ex)
    {
	test(ex == L"def");
    }

    try
    {
	p->opEx(10);
    }
    catch(const Test::ByteV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Byte(0x0f));
	test(ex[1] == Ice::Byte(0xf0));
    }

    try
    {
	p->opEx(11);
    }
    catch(const Test::BoolV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == true);
	test(ex[1] == false);
    }

    try
    {
	p->opEx(12);
    }
    catch(const Test::ShortV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 1);
	test(ex[1] == 2);
    }

    try
    {
	p->opEx(13);
    }
    catch(const Test::IntV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 3);
	test(ex[1] == 4);
    }

    try
    {
	p->opEx(14);
    }
    catch(const Test::LongV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == 5);
	test(ex[1] == 6);
    }

    try
    {
	p->opEx(15);
    }
    catch(const Test::FloatV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Float(7.7));
	test(ex[1] == Ice::Float(8.8));
    }

    try
    {
	p->opEx(16);
    }
    catch(const Test::DoubleV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == Ice::Double(9.9));
	test(ex[1] == Ice::Double(10.10));
    }

    try
    {
	p->opEx(17);
    }
    catch(const Test::StringV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == "abc");
	test(ex[1] == "def");
    }

    try
    {
	p->opEx(18);
    }
    catch(const Test::WStringV& ex)
    {
	test(ex.size() == 2);
	test(ex[0] == L"xxx");
	test(ex[1] == L"XXX");
    }

    try
    {
	p->opEx(19);
    }
    catch(const Test::ByteVV& ex)
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
    }
    catch(const Test::BoolVV& ex)
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
    }
    catch(const Test::ShortVV& ex)
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
    }
    catch(const Test::IntVV& ex)
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
    }
    catch(const Test::LongVV& ex)
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
    }
    catch(const Test::FloatVV& ex)
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
    }
    catch(const Test::DoubleVV& ex)
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
    }
    catch(const Test::StringVV& ex)
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
    }
    catch(const Test::WStringVV& ex)
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
    }
    catch(const Test::MyClassPrxE& ex)
    {
	test(ex == p);
	ex->opVoid();
    }

}
