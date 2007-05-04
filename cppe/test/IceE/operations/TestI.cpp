// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>
#include <TestCommon.h>
#include <functional>

MyDerivedClassI::MyDerivedClassI()
{
}

void
MyDerivedClassI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}

void
MyDerivedClassI::opVoid(const Ice::Current&)
{
}

void
MyDerivedClassI::opSleep(int duration, const Ice::Current&)
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(duration));
}

Ice::Byte
MyDerivedClassI::opByte(Ice::Byte p1,
			Ice::Byte p2,
			Ice::Byte& p3,
			const Ice::Current&)
{
    p3 = p1 ^ p2;
    return p1;
}

bool
MyDerivedClassI::opBool(bool p1,
			bool p2,
			bool& p3,
			const Ice::Current&)
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
				Ice::Long& p6,
				const Ice::Current&)
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
			       Ice::Double& p4,
			       const Ice::Current&)
{
    p3 = p1;
    p4 = p2;
    return p2;
}

std::string
MyDerivedClassI::opString(const std::string& p1,
			  const std::string& p2,
			  std::string& p3,
			  const Ice::Current&)
{
    p3 = p2 + " " + p1;
    return p1 + " " + p2;
}

Test::MyEnum
MyDerivedClassI::opMyEnum(Test::MyEnum p1,
			  Test::MyEnum& p2,
			  const Ice::Current&)
{
    p2 = p1;
    return Test::enum3;
}

Test::MyClassPrx
MyDerivedClassI::opMyClass(const Test::MyClassPrx& p1,
			   Test::MyClassPrx& p2,
			   Test::MyClassPrx& p3,
			   const Ice::Current& current)
{
    p2 = p1;
    p3 = Test::MyClassPrx::uncheckedCast(current.adapter->createProxy(
    			current.adapter->getCommunicator()->stringToIdentity("noSuchIdentity")));
    return Test::MyClassPrx::uncheckedCast(current.adapter->createProxy(current.id));
}

Test::Structure
MyDerivedClassI::opStruct(const Test::Structure& p1,
			  const ::Test::Structure& p2,
			  ::Test::Structure& p3,
			  const Ice::Current&)
{
    p3 = p1;
    p3.s.s = "a new string";
    return p2;
}

Test::ByteS
MyDerivedClassI::opByteS(const Test::ByteS& p1,
			 const Test::ByteS& p2,
			 Test::ByteS& p3,
			 const Ice::Current&)
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
			 Test::BoolS& p3,
			 const Ice::Current&)
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
				 Test::LongS& p6,
				 const Ice::Current&)
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
				Test::DoubleS& p4,
				const Ice::Current&)
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
			   Test::StringS& p3,
			   const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ByteSS
MyDerivedClassI::opByteSS(const Test::ByteSS& p1,
			  const Test::ByteSS& p2,
			  Test::ByteSS& p3,
			  const Ice::Current&)
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
			  Test::BoolSS& p3,
			  const Ice::Current&)
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
				  Test::LongSS& p6,
				  const Ice::Current&)
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
				 Test::DoubleSS& p4,
				 const Ice::Current&)
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
			    Test::StringSS& p3,
			    const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::StringSSS
MyDerivedClassI::opStringSSS(const Test::StringSSS& p1,
			     const Test::StringSSS& p2,
			     Test::StringSSS& p3,
			     const ::Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD(const Test::ByteBoolD& p1,
			     const Test::ByteBoolD& p2,
			     Test::ByteBoolD& p3,
			     const Ice::Current&)
{
    p3 = p1;
    Test::ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortIntD
MyDerivedClassI::opShortIntD(const Test::ShortIntD& p1,
			     const Test::ShortIntD& p2,
			     Test::ShortIntD& p3,
			     const Ice::Current&)
{
    p3 = p1;
    Test::ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongFloatD
MyDerivedClassI::opLongFloatD(const Test::LongFloatD& p1,
			      const Test::LongFloatD& p2,
			      Test::LongFloatD& p3,
			      const Ice::Current&)
{
    p3 = p1;
    Test::LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringStringD
MyDerivedClassI::opStringStringD(const Test::StringStringD& p1,
				 const Test::StringStringD& p2,
				 Test::StringStringD& p3,
				 const Ice::Current&)
{
    p3 = p1;
    Test::StringStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringMyEnumD
MyDerivedClassI::opStringMyEnumD(const Test::StringMyEnumD& p1,
				 const Test::StringMyEnumD& p2,
				 Test::StringMyEnumD& p3,
				 const Ice::Current&)
{
    p3 = p1;
    Test::StringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::IntS
MyDerivedClassI::opIntS(const Test::IntS& s, const Ice::Current&)
{
    Test::IntS r;
    std::transform(s.begin(), s.end(), std::back_inserter(r), std::negate<int>());
    return r;
}

void
MyDerivedClassI::opByteSOneway(const Test::ByteS&, const Ice::Current&)
{
}

Test::StringStringD
MyDerivedClassI::opContext(const Ice::Current& c)
{
    return c.ctx;
}

void 
MyDerivedClassI::opDoubleMarshaling(Ice::Double p1, const Test::DoubleS& p2, const Ice::Current&)
{
    Ice::Double d = 1278312346.0 / 13.0;
    test(p1 == d);
    for(unsigned int i = 0; i < p2.size(); ++i)
    {
        test(p2[i] == d);
    }
}

void
MyDerivedClassI::opDerived(const Ice::Current&)
{
}
