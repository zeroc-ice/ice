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

Test::ByteV
MyDerivedClassI::opByteV(const Test::ByteV& p1,
			 const Test::ByteV& p2,
			 Test::ByteV& p3)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteV r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolV
MyDerivedClassI::opBoolV(const Test::BoolV& p1,
			 const Test::BoolV& p2,
			 Test::BoolV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolV r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongV
MyDerivedClassI::opShortIntLongV(const Test::ShortV& p1,
				 const Test::IntV& p2,
				 const Test::LongV& p3,
				 Test::ShortV& p4,
				 Test::IntV& p5,
				 Test::LongV& p6)
{
    p4 = p1;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    return p3;
}

Test::DoubleV
MyDerivedClassI::opFloatDoubleV(const Test::FloatV& p1,
				const Test::DoubleV& p2,
				Test::FloatV& p3,
				Test::DoubleV& p4)
{
    p3 = p1;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleV r = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(r));
    return r;
}

Test::StringV
MyDerivedClassI::opStringV(const Test::StringV& p1,
			   const Test::StringV& p2,
			   Test::StringV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringV r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::WStringV
MyDerivedClassI::opWStringV(const Test::WStringV& p1,
			    const Test::WStringV& p2,
			    Test::WStringV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::WStringV r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ByteVV
MyDerivedClassI::opByteVV(const Test::ByteVV& p1,
			  const Test::ByteVV& p2,
			  Test::ByteVV& p3)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteVV r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolVV
MyDerivedClassI::opBoolVV(const Test::BoolVV& p1,
			  const Test::BoolVV& p2,
			  Test::BoolVV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolVV r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongVV
MyDerivedClassI::opShortIntLongVV(const Test::ShortVV& p1,
				  const Test::IntVV& p2,
				  const Test::LongVV& p3,
				  Test::ShortVV& p4,
				  Test::IntVV& p5,
				  Test::LongVV& p6)
{
    p4 = p1;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    return p3;
}

Test::DoubleVV
MyDerivedClassI::opFloatDoubleVV(const Test::FloatVV& p1,
				 const Test::DoubleVV& p2,
				 Test::FloatVV& p3,
				 Test::DoubleVV& p4)
{
    p3 = p1;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleVV r = p2;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::StringVV
MyDerivedClassI::opStringVV(const Test::StringVV& p1,
			    const Test::StringVV& p2,
			    Test::StringVV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringVV r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::WStringVV
MyDerivedClassI::opWStringVV(const Test::WStringVV& p1,
			     const Test::WStringVV& p2,
			     Test::WStringVV& p3)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::WStringVV r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
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
	    Test::ByteV ex;
	    ex.push_back(Ice::Byte(0x0f));
	    ex.push_back(Ice::Byte(0xf0));
	    throw ex;
	}
	
	case 11:
	{
	    Test::BoolV ex;
	    ex.push_back(true);
	    ex.push_back(false);
	    throw ex;
	}
	
	case 12:
	{
	    Test::ShortV ex;
	    ex.push_back(1);
	    ex.push_back(2);
	    throw ex;
	}
	
	case 13:
	{
	    Test::IntV ex;
	    ex.push_back(3);
	    ex.push_back(4);
	    throw ex;
	}
	
	case 14:
	{
	    Test::LongV ex;
	    ex.push_back(5);
	    ex.push_back(6);
	    throw ex;
	}
	
	case 15:
	{
	    Test::FloatV ex;
	    ex.push_back(Ice::Float(7.7));
	    ex.push_back(Ice::Float(8.8));
	    throw ex;
	}
	
	case 16:
	{
	    Test::DoubleV ex;
	    ex.push_back(Ice::Double(9.9));
	    ex.push_back(Ice::Double(10.10));
	    throw ex;
	}
	
	case 17:
	{
	    Test::StringV ex;
	    ex.push_back("abc");
	    ex.push_back("def");
	    throw ex;
	}
	
	case 18:
	{
	    Test::WStringV ex;
	    ex.push_back(L"xxx");
	    ex.push_back(L"XXX");
	    throw ex;
	}
	
	case 19:
	{
	    Test::ByteV v1;
	    Test::ByteV v2;
	    v1.push_back(Ice::Byte(0x01));
	    v2.push_back(Ice::Byte(0x10));
	    v2.push_back(Ice::Byte(0x11));
	    Test::ByteVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 20:
	{
	    Test::BoolV v1;
	    Test::BoolV v2;
	    v1.push_back(true);
	    v2.push_back(false);
	    v2.push_back(true);
	    Test::BoolVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 21:
	{
	    Test::ShortV v1;
	    Test::ShortV v2;
	    v1.push_back(1);
	    v2.push_back(2);
	    v2.push_back(3);
	    Test::ShortVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 22:
	{
	    Test::IntV v1;
	    Test::IntV v2;
	    v1.push_back(4);
	    v2.push_back(5);
	    v2.push_back(6);
	    Test::IntVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 23:
	{
	    Test::LongV v1;
	    Test::LongV v2;
	    v1.push_back(7);
	    v2.push_back(8);
	    v2.push_back(9);
	    Test::LongVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 24:
	{
	    Test::FloatV v1;
	    Test::FloatV v2;
	    v1.push_back(Ice::Float(10.10));
	    v2.push_back(Ice::Float(11.11));
	    v2.push_back(Ice::Float(12.12));
	    Test::FloatVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 25:
	{
	    Test::DoubleV v1;
	    Test::DoubleV v2;
	    v1.push_back(Ice::Double(13.13));
	    v2.push_back(Ice::Double(14.14));
	    v2.push_back(Ice::Double(15.15));
	    Test::DoubleVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 26:
	{
	    Test::StringV v1;
	    Test::StringV v2;
	    v1.push_back("aaa");
	    v2.push_back("bbb");
	    v2.push_back("ccc");
	    Test::StringVV ex;
	    ex.push_back(v1);
	    ex.push_back(v2);
	    throw ex;
	}
	
	case 27:
	{
	    Test::WStringV v1;
	    Test::WStringV v2;
	    v1.push_back(L"aaa");
	    v2.push_back(L"bbb");
	    v2.push_back(L"ccc");
	    Test::WStringVV ex;
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
