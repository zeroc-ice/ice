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
#include <TestI.h>

MyDerivedClassI::MyDerivedClassI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
MyDerivedClassI::shutdown()
{
    _adapter->getCommunicator()->shutdown();
}

void
MyDerivedClassI::opVoid()
{
}

Ice::Byte
MyDerivedClassI::opByte(Ice::Byte p1,
			Ice::Byte p2,
			Ice::Byte& p3)
{
    p3 = p1 ^ p2;
    return p1;
}

bool
MyDerivedClassI::opBool(bool p1,
			bool p2,
			bool& p3)
{
    p3 = p1;
    return p2;
}

Ice::Long
MyDerivedClassI::opShortIntLong(Ice::Short p1,
				Ice::Int p2,
				Ice::Long p3,
				Ice::Short& p4,
				Ice::Int& p5,
				Ice::Long& p6)
{
    p4 = p1;
    p5 = p2;
    p6 = p3;
    return p3;
}

Ice::Double
MyDerivedClassI::opFloatDouble(Ice::Float p1,
			       Ice::Double p2,
			       Ice::Float& p3,
			       Ice::Double& p4)
{
    p3 = p1;
    p4 = p2;
    return p2;
}

std::string
MyDerivedClassI::opString(const std::string& p1,
			  const std::string& p2,
			  std::string& p3)
{
    p3 = p2 + " " + p1;
    return p1 + " " + p2;
}

std::wstring
MyDerivedClassI::opWString(const std::wstring& p1,
			   const std::wstring& p2,
			   std::wstring& p3)
{
    p3 = p2 + L" " + p1;
    return p1 + L" " + p2;
}

Test::MyEnum
MyDerivedClassI::opMyEnum(Test::MyEnum p1,
			  Test::MyEnum& p2)
{
    p2 = p1;
    return Test::enum3;
}

Test::MyClassPrx
MyDerivedClassI::opMyClass(const Test::MyClassPrx& p1,
			   Test::MyClassPrx& p2)
{
    p2 = p1;
    return Test::MyClassPrx::uncheckedCast(_adapter->objectToProxy(this));
}

Test::ByteS
MyDerivedClassI::opByteS(const Test::ByteS& p1,
			 const Test::ByteS& p2,
			 Test::ByteS& p3)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolS
MyDerivedClassI::opBoolS(const Test::BoolS& p1,
			 const Test::BoolS& p2,
			 Test::BoolS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongS
MyDerivedClassI::opShortIntLongS(const Test::ShortS& p1,
				 const Test::IntS& p2,
				 const Test::LongS& p3,
				 Test::ShortS& p4,
				 Test::IntS& p5,
				 Test::LongS& p6)
{
    p4 = p1;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    return p3;
}

Test::DoubleS
MyDerivedClassI::opFloatDoubleS(const Test::FloatS& p1,
				const Test::DoubleS& p2,
				Test::FloatS& p3,
				Test::DoubleS& p4)
{
    p3 = p1;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleS r = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(r));
    return r;
}

Test::StringS
MyDerivedClassI::opStringS(const Test::StringS& p1,
			   const Test::StringS& p2,
			   Test::StringS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::WStringS
MyDerivedClassI::opWStringS(const Test::WStringS& p1,
			    const Test::WStringS& p2,
			    Test::WStringS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::WStringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ByteSS
MyDerivedClassI::opByteSS(const Test::ByteSS& p1,
			  const Test::ByteSS& p2,
			  Test::ByteSS& p3)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteSS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolSS
MyDerivedClassI::opBoolSS(const Test::BoolSS& p1,
			  const Test::BoolSS& p2,
			  Test::BoolSS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolSS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongSS
MyDerivedClassI::opShortIntLongSS(const Test::ShortSS& p1,
				  const Test::IntSS& p2,
				  const Test::LongSS& p3,
				  Test::ShortSS& p4,
				  Test::IntSS& p5,
				  Test::LongSS& p6)
{
    p4 = p1;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    return p3;
}

Test::DoubleSS
MyDerivedClassI::opFloatDoubleSS(const Test::FloatSS& p1,
				 const Test::DoubleSS& p2,
				 Test::FloatSS& p3,
				 Test::DoubleSS& p4)
{
    p3 = p1;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleSS r = p2;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::StringSS
MyDerivedClassI::opStringSS(const Test::StringSS& p1,
			    const Test::StringSS& p2,
			    Test::StringSS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::WStringSS
MyDerivedClassI::opWStringSS(const Test::WStringSS& p1,
			     const Test::WStringSS& p2,
			     Test::WStringSS& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::WStringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD(const Test::ByteBoolD& p1, const Test::ByteBoolD& p2,
			     Test::ByteBoolD& p3)
{
    p3 = p1;
    Test::ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortIntD
MyDerivedClassI::opShortIntD(const Test::ShortIntD& p1, const Test::ShortIntD& p2,
			     Test::ShortIntD& p3)
{
    p3 = p1;
    Test::ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongFloatD
MyDerivedClassI::opLongFloatD(const Test::LongFloatD& p1, const Test::LongFloatD& p2,
			      Test::LongFloatD& p3)
{
    p3 = p1;
    Test::LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::DoubleStringD
MyDerivedClassI::opDoubleStringD(const Test::DoubleStringD& p1, const Test::DoubleStringD& p2,
				 Test::DoubleStringD& p3)
{
    p3 = p1;
    Test::DoubleStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::WStringMyEnumD
MyDerivedClassI::opWStringMyEnumD(const Test::WStringMyEnumD& p1, const Test::WStringMyEnumD& p2,
				  Test::WStringMyEnumD& p3)
{
    p3 = p1;
    Test::WStringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

using namespace std;

Test::MyClassStringD
MyDerivedClassI::opMyClassStringD(const Test::MyClassStringD& p1, const Test::MyClassStringD& p2,
				  Test::MyClassStringD& p3)
{
    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(_adapter->objectToProxy(this));
    p3 = p1;
    Test::MyClassStringD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

void
MyDerivedClassI::opEx(Ice::Int p)
{
    switch (p)
    {
	case 0:
	{
	    break;
	}

	case 1:
	{
	    throw Ice::Byte(0x1f);
	}

	case 2:
	{
	    throw bool(true);
	}

	case 3:
	{
	    throw Ice::Short(123);
	}

	case 4:
	{
	    throw Ice::Int(456);
	}

	case 5:
	{
	    throw Ice::Long(789);
	}

	case 6:
	{
	    throw Ice::Float(10.1112);
	}

	case 7:
	{
	    throw Ice::Double(13.1415);
	}

	case 8:
	{
	    throw std::string("abc");
	}

	case 9:
	{
	    throw std::wstring(L"def");
	}

	case 10:
	{
	    Test::ByteS ex;
	    ex.push_back(Ice::Byte(0x0f));
	    ex.push_back(Ice::Byte(0xf0));
	    throw ex;
	}
	
	case 11:
	{
	    Test::BoolS ex;
	    ex.push_back(true);
	    ex.push_back(false);
	    throw ex;
	}
	
	case 12:
	{
	    Test::ShortS ex;
	    ex.push_back(1);
	    ex.push_back(2);
	    throw ex;
	}
	
	case 13:
	{
	    Test::IntS ex;
	    ex.push_back(3);
	    ex.push_back(4);
	    throw ex;
	}
	
	case 14:
	{
	    Test::LongS ex;
	    ex.push_back(5);
	    ex.push_back(6);
	    throw ex;
	}
	
	case 15:
	{
	    Test::FloatS ex;
	    ex.push_back(Ice::Float(7.7));
	    ex.push_back(Ice::Float(8.8));
	    throw ex;
	}
	
	case 16:
	{
	    Test::DoubleS ex;
	    ex.push_back(Ice::Double(9.9));
	    ex.push_back(Ice::Double(10.10));
	    throw ex;
	}
	
	case 17:
	{
	    Test::StringS ex;
	    ex.push_back("abc");
	    ex.push_back("def");
	    throw ex;
	}
	
	case 18:
	{
	    Test::WStringS ex;
	    ex.push_back(L"xxx");
	    ex.push_back(L"XXX");
	    throw ex;
	}
	
	case 19:
	{
	    Test::ByteS v1;
	    Test::ByteS v2;
	    v1.push_back(Ice::Byte(0x01));
	    v2.push_back(Ice::Byte(0x10));
	    v2.push_back(Ice::Byte(0x11));
	    Test::ByteSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 20:
	{
	    Test::BoolS v1;
	    Test::BoolS v2;
	    v1.push_back(true);
	    v2.push_back(false);
	    v2.push_back(true);
	    Test::BoolSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 21:
	{
	    Test::ShortS v1;
	    Test::ShortS v2;
	    v1.push_back(1);
	    v2.push_back(2);
	    v2.push_back(3);
	    Test::ShortSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 22:
	{
	    Test::IntS v1;
	    Test::IntS v2;
	    v1.push_back(4);
	    v2.push_back(5);
	    v2.push_back(6);
	    Test::IntSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 23:
	{
	    Test::LongS v1;
	    Test::LongS v2;
	    v1.push_back(7);
	    v2.push_back(8);
	    v2.push_back(9);
	    Test::LongSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 24:
	{
	    Test::FloatS v1;
	    Test::FloatS v2;
	    v1.push_back(Ice::Float(10.10));
	    v2.push_back(Ice::Float(11.11));
	    v2.push_back(Ice::Float(12.12));
	    Test::FloatSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 25:
	{
	    Test::DoubleS v1;
	    Test::DoubleS v2;
	    v1.push_back(Ice::Double(13.13));
	    v2.push_back(Ice::Double(14.14));
	    v2.push_back(Ice::Double(15.15));
	    Test::DoubleSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 26:
	{
	    Test::StringS v1;
	    Test::StringS v2;
	    v1.push_back("aaa");
	    v2.push_back("bbb");
	    v2.push_back("ccc");
	    Test::StringSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 27:
	{
	    Test::WStringS v1;
	    Test::WStringS v2;
	    v1.push_back(L"aaa");
	    v2.push_back(L"bbb");
	    v2.push_back(L"ccc");
	    Test::WStringSS ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 28:
	{
	    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(_adapter->objectToProxy(this));
	    p->_throw();
	}
    }
}

void
MyDerivedClassI::opDerived()
{
};
