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

MyDerivedClassI::MyDerivedClassI(const Ice::ObjectAdapterPtr& adapter, const std::string& identity) :
    _adapter(adapter),
    _identity(identity)
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
			   Test::MyClassPrx& p2, Test::MyClassPrx& p3)
{
    p2 = p1;
    p3 = Test::MyClassPrx::uncheckedCast(_adapter->createProxy("noSuchIdentity"));
    return Test::MyClassPrx::uncheckedCast(_adapter->createProxy(_identity));
}

Test::Struct
MyDerivedClassI::opStruct(const Test::Struct& p1, const ::Test::Struct& p2, ::Test::Struct& p3)
{
    p3 = p1;
    p3.s.s = "a new string";
    return p2;
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
    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(_adapter->createProxy(_identity));
    p3 = p1;
    Test::MyClassStringD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

void
MyDerivedClassI::opDerived()
{
};
