// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TestI.h>
#include <TestCommon.h>
#include <functional>
#include <iterator>

using namespace Ice;
using namespace Test;
using namespace std;

MyDerivedClassI::MyDerivedClassI() : _opByteSOnewayCallCount(0)
{
}

bool
MyDerivedClassI::ice_isA(ICE_IN(string) id, const Ice::Current& current) const
{
    test(current.mode == ICE_ENUM(OperationMode, Nonmutating));
#ifdef ICE_CPP11_MAPPING
    return Test::MyDerivedClass::ice_isA(move(id), current);
#else
    return Test::MyDerivedClass::ice_isA(id, current);
#endif
}

void
MyDerivedClassI::ice_ping(const Ice::Current& current) const
{
    test(current.mode == ICE_ENUM(OperationMode, Nonmutating));
    Test::MyDerivedClass::ice_ping(current);
}

std::vector<std::string>
MyDerivedClassI::ice_ids(const Ice::Current& current) const
{
    test(current.mode == ICE_ENUM(OperationMode, Nonmutating));
    return Test::MyDerivedClass::ice_ids(current);
}

#ifdef ICE_CPP11_MAPPING
std::string
#else
const std::string&
#endif
MyDerivedClassI::ice_id(const Ice::Current& current) const
{
    test(current.mode == ICE_ENUM(OperationMode, Nonmutating));
    return Test::MyDerivedClass::ice_id(current);
}

void
MyDerivedClassI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

bool
MyDerivedClassI::supportsCompress(const Ice::Current& current)
{
#if defined(ICE_OS_UWP)
    return false;
#else
    return true;
#endif
}

void
MyDerivedClassI::opVoid(const Ice::Current& current)
{
    test(current.mode == ICE_ENUM(OperationMode, Normal));
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
MyDerivedClassI::opString(ICE_IN(string) p1,
                          ICE_IN(string) p2,
                          string& p3,
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
    return ICE_ENUM(MyEnum, enum3);
}

Test::MyClassPrxPtr
MyDerivedClassI::opMyClass(ICE_IN(Test::MyClassPrxPtr) p1,
                           Test::MyClassPrxPtr& p2,
                           Test::MyClassPrxPtr& p3,
                           const Ice::Current& current)
{
    p2 = p1;
    p3 = ICE_UNCHECKED_CAST(Test::MyClassPrx,
                            current.adapter->createProxy(
                                stringToIdentity("noSuchIdentity")));
    return ICE_UNCHECKED_CAST(Test::MyClassPrx, current.adapter->createProxy(current.id));
}

Test::Structure
MyDerivedClassI::opStruct(ICE_IN(Test::Structure) p1,
                          ICE_IN(Test::Structure) p2,
                          ::Test::Structure& p3,
                          const Ice::Current&)
{
    p3 = p1;
    p3.s.s = "a new string";
    return p2;
}

Test::ByteS
MyDerivedClassI::opByteS(ICE_IN(Test::ByteS) p1,
                         ICE_IN(Test::ByteS) p2,
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
MyDerivedClassI::opBoolS(ICE_IN(Test::BoolS) p1,
                         ICE_IN(Test::BoolS) p2,
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
MyDerivedClassI::opShortIntLongS(ICE_IN(Test::ShortS) p1,
                                 ICE_IN(Test::IntS) p2,
                                 ICE_IN(Test::LongS) p3,
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
MyDerivedClassI::opFloatDoubleS(ICE_IN(Test::FloatS) p1,
                                ICE_IN(Test::DoubleS) p2,
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
MyDerivedClassI::opStringS(ICE_IN(Test::StringS) p1,
                           ICE_IN(Test::StringS) p2,
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
MyDerivedClassI::opByteSS(ICE_IN(Test::ByteSS) p1,
                          ICE_IN(Test::ByteSS) p2,
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
MyDerivedClassI::opBoolSS(ICE_IN(Test::BoolSS) p1,
                          ICE_IN(Test::BoolSS) p2,
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
MyDerivedClassI::opShortIntLongSS(ICE_IN(Test::ShortSS) p1,
                                  ICE_IN(Test::IntSS) p2,
                                  ICE_IN(Test::LongSS) p3,
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
MyDerivedClassI::opFloatDoubleSS(ICE_IN(Test::FloatSS) p1,
                                 ICE_IN(Test::DoubleSS) p2,
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
MyDerivedClassI::opStringSS(ICE_IN(Test::StringSS) p1,
                            ICE_IN(Test::StringSS) p2,
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
MyDerivedClassI::opStringSSS(ICE_IN(Test::StringSSS) p1,
                             ICE_IN(Test::StringSSS) p2,
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
MyDerivedClassI::opByteBoolD(ICE_IN(Test::ByteBoolD) p1,
                             ICE_IN(Test::ByteBoolD) p2,
                             Test::ByteBoolD& p3,
                             const Ice::Current&)
{
    p3 = p1;
    Test::ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortIntD
MyDerivedClassI::opShortIntD(ICE_IN(Test::ShortIntD) p1,
                             ICE_IN(Test::ShortIntD) p2,
                             Test::ShortIntD& p3,
                             const Ice::Current&)
{
    p3 = p1;
    Test::ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongFloatD
MyDerivedClassI::opLongFloatD(ICE_IN(Test::LongFloatD) p1,
                              ICE_IN(Test::LongFloatD) p2,
                              Test::LongFloatD& p3,
                              const Ice::Current&)
{
    p3 = p1;
    Test::LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringStringD
MyDerivedClassI::opStringStringD(ICE_IN(Test::StringStringD) p1,
                                 ICE_IN(Test::StringStringD) p2,
                                 Test::StringStringD& p3,
                                 const Ice::Current&)
{
    p3 = p1;
    Test::StringStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringMyEnumD
MyDerivedClassI::opStringMyEnumD(ICE_IN(Test::StringMyEnumD) p1,
                                 ICE_IN(Test::StringMyEnumD) p2,
                                 Test::StringMyEnumD& p3,
                                 const Ice::Current&)
{
    p3 = p1;
    Test::StringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyEnumStringD
MyDerivedClassI::opMyEnumStringD(ICE_IN(Test::MyEnumStringD) p1,
                                 ICE_IN(Test::MyEnumStringD) p2,
                                 Test::MyEnumStringD& p3,
                                 const Ice::Current&)
{
    p3 = p1;
    Test::MyEnumStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyStructMyEnumD
MyDerivedClassI::opMyStructMyEnumD(ICE_IN(Test::MyStructMyEnumD) p1,
                                   ICE_IN(Test::MyStructMyEnumD) p2,
                                   Test::MyStructMyEnumD& p3,
                                   const Ice::Current&)
{
    p3 = p1;
    Test::MyStructMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ByteBoolDS
MyDerivedClassI::opByteBoolDS(ICE_IN(Test::ByteBoolDS) p1,
                              ICE_IN(Test::ByteBoolDS) p2,
                              Test::ByteBoolDS& p3,
                              const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ByteBoolDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ShortIntDS
MyDerivedClassI::opShortIntDS(ICE_IN(Test::ShortIntDS) p1,
                              ICE_IN(Test::ShortIntDS) p2,
                              Test::ShortIntDS& p3,
                              const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ShortIntDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongFloatDS
MyDerivedClassI::opLongFloatDS(ICE_IN(Test::LongFloatDS) p1,
                               ICE_IN(Test::LongFloatDS) p2,
                               Test::LongFloatDS& p3,
                               const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::LongFloatDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::StringStringDS
MyDerivedClassI::opStringStringDS(ICE_IN(Test::StringStringDS) p1,
                                  ICE_IN(Test::StringStringDS) p2,
                                  Test::StringStringDS& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::StringStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::StringMyEnumDS
MyDerivedClassI::opStringMyEnumDS(ICE_IN(Test::StringMyEnumDS) p1,
                                  ICE_IN(Test::StringMyEnumDS) p2,
                                  Test::StringMyEnumDS& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::StringMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::MyEnumStringDS
MyDerivedClassI::opMyEnumStringDS(ICE_IN(Test::MyEnumStringDS) p1,
                                  ICE_IN(Test::MyEnumStringDS) p2,
                                  Test::MyEnumStringDS& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::MyEnumStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::MyStructMyEnumDS
MyDerivedClassI::opMyStructMyEnumDS(ICE_IN(Test::MyStructMyEnumDS) p1,
                                    ICE_IN(Test::MyStructMyEnumDS) p2,
                                    Test::MyStructMyEnumDS& p3,
                                    const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::MyStructMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ByteByteSD
MyDerivedClassI::opByteByteSD(ICE_IN(Test::ByteByteSD) p1,
                              ICE_IN(Test::ByteByteSD) p2,
                              Test::ByteByteSD& p3,
                              const Ice::Current&)
{
    p3 = p2;
    Test::ByteByteSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::BoolBoolSD
MyDerivedClassI::opBoolBoolSD(ICE_IN(Test::BoolBoolSD) p1,
                              ICE_IN(Test::BoolBoolSD) p2,
                              Test::BoolBoolSD& p3,
                              const Ice::Current&)
{
    p3 = p2;
    Test::BoolBoolSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortShortSD
MyDerivedClassI::opShortShortSD(ICE_IN(Test::ShortShortSD) p1,
                                ICE_IN(Test::ShortShortSD) p2,
                                Test::ShortShortSD& p3,
                                const Ice::Current&)
{
    p3 = p2;
    Test::ShortShortSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::IntIntSD
MyDerivedClassI::opIntIntSD(ICE_IN(Test::IntIntSD) p1,
                            ICE_IN(Test::IntIntSD) p2,
                            Test::IntIntSD& p3,
                            const Ice::Current&)
{
    p3 = p2;
    Test::IntIntSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongLongSD
MyDerivedClassI::opLongLongSD(ICE_IN(Test::LongLongSD) p1,
                              ICE_IN(Test::LongLongSD) p2,
                              Test::LongLongSD& p3,
                              const Ice::Current&)
{
    p3 = p2;
    Test::LongLongSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringFloatSD
MyDerivedClassI::opStringFloatSD(ICE_IN(Test::StringFloatSD) p1,
                                 ICE_IN(Test::StringFloatSD) p2,
                                 Test::StringFloatSD& p3,
                                 const Ice::Current&)
{
    p3 = p2;
    Test::StringFloatSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringDoubleSD
MyDerivedClassI::opStringDoubleSD(ICE_IN(Test::StringDoubleSD) p1,
                                  ICE_IN(Test::StringDoubleSD) p2,
                                  Test::StringDoubleSD& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    Test::StringDoubleSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringStringSD
MyDerivedClassI::opStringStringSD(ICE_IN(Test::StringStringSD) p1,
                                  ICE_IN(Test::StringStringSD) p2,
                                  Test::StringStringSD& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    Test::StringStringSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyEnumMyEnumSD
MyDerivedClassI::opMyEnumMyEnumSD(ICE_IN(Test::MyEnumMyEnumSD) p1,
                                  ICE_IN(Test::MyEnumMyEnumSD) p2,
                                  Test::MyEnumMyEnumSD& p3,
                                  const Ice::Current&)
{
    p3 = p2;
    Test::MyEnumMyEnumSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::IntS
MyDerivedClassI::opIntS(ICE_IN(Test::IntS) s, const Ice::Current&)
{
    Test::IntS r;
    std::transform(s.begin(), s.end(), std::back_inserter(r), std::negate<int>());
    return r;
}

void
MyDerivedClassI::opByteSOneway(ICE_IN(Test::ByteS), const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    ++_opByteSOnewayCallCount;
}

int
MyDerivedClassI::opByteSOnewayCallCount(const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    int count = _opByteSOnewayCallCount;
    _opByteSOnewayCallCount = 0;
    return count;
}

Test::StringStringD
MyDerivedClassI::opContext(const Ice::Current& c)
{
    return c.ctx;
}

void
MyDerivedClassI::opDoubleMarshaling(Ice::Double p1, ICE_IN(Test::DoubleS) p2, const Ice::Current&)
{
    Ice::Double d = 1278312346.0 / 13.0;
    test(p1 == d);
    for(unsigned int i = 0; i < p2.size(); ++i)
    {
        test(p2[i] == d);
    }
}

void
MyDerivedClassI::opIdempotent(const Ice::Current& current)
{
    test(current.mode == ICE_ENUM(OperationMode, Idempotent));
}

void
MyDerivedClassI::opNonmutating(const Ice::Current& current)
{
    test(current.mode == ICE_ENUM(OperationMode, Nonmutating));
}

void
MyDerivedClassI::opDerived(const Ice::Current&)
{
}

Ice::Byte
MyDerivedClassI::opByte1(Ice::Byte b, const Ice::Current&)
{
    return b;
}

Ice::Short
MyDerivedClassI::opShort1(Ice::Short s, const Ice::Current&)
{
    return s;
}

Ice::Int
MyDerivedClassI::opInt1(Ice::Int i, const Ice::Current&)
{
    return i;
}

Ice::Long
MyDerivedClassI::opLong1(Ice::Long l, const Ice::Current&)
{
    return l;
}

Ice::Float
MyDerivedClassI::opFloat1(Ice::Float f, const Ice::Current&)
{
    return f;
}

Ice::Double
MyDerivedClassI::opDouble1(Ice::Double d, const Ice::Current&)
{
    return d;
}

std::string
MyDerivedClassI::opString1(ICE_IN(string) s, const Ice::Current&)
{
    return s;
}

Test::StringS
MyDerivedClassI::opStringS1(ICE_IN(Test::StringS) seq, const Ice::Current&)
{
    return seq;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD1(ICE_IN(Test::ByteBoolD) dict, const Ice::Current&)
{
    return dict;
}

Test::StringS
MyDerivedClassI::opStringS2(ICE_IN(Test::StringS) seq, const Ice::Current&)
{
    return seq;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD2(ICE_IN(Test::ByteBoolD) dict, const Ice::Current&)
{
    return dict;
}

Test::MyStruct1
MyDerivedClassI::opMyStruct1(ICE_IN(Test::MyStruct1) s, const Ice::Current&)
{
    return s;
}

Test::MyClass1Ptr
MyDerivedClassI::opMyClass1(ICE_IN(Test::MyClass1Ptr) c, const Ice::Current&)
{
    return c;
}

Test::StringS
MyDerivedClassI::opStringLiterals(const Ice::Current&)
{
    Test::StringS data;
    data.push_back(Test::s0);
    data.push_back(Test::s1);
    data.push_back(Test::s2);
    data.push_back(Test::s3);
    data.push_back(Test::s4);
    data.push_back(Test::s5);
    data.push_back(Test::s6);
    data.push_back(Test::s7);
    data.push_back(Test::s8);
    data.push_back(Test::s9);
    data.push_back(Test::s10);

    data.push_back(Test::sw0);
    data.push_back(Test::sw1);
    data.push_back(Test::sw2);
    data.push_back(Test::sw3);
    data.push_back(Test::sw4);
    data.push_back(Test::sw5);
    data.push_back(Test::sw6);
    data.push_back(Test::sw7);
    data.push_back(Test::sw8);
    data.push_back(Test::sw9);
    data.push_back(Test::sw10);

    data.push_back(Test::ss0);
    data.push_back(Test::ss1);
    data.push_back(Test::ss2);
    data.push_back(Test::ss3);
    data.push_back(Test::ss4);
    data.push_back(Test::ss5);

    data.push_back(Test::su0);
    data.push_back(Test::su1);
    data.push_back(Test::su2);

    return data;
}

Test::WStringS
MyDerivedClassI::opWStringLiterals(const Ice::Current&)
{
    Test::WStringS data;
    data.push_back(Test::ws0);
    data.push_back(Test::ws1);
    data.push_back(Test::ws2);
    data.push_back(Test::ws3);
    data.push_back(Test::ws4);
    data.push_back(Test::ws5);
    data.push_back(Test::ws6);
    data.push_back(Test::ws7);
    data.push_back(Test::ws8);
    data.push_back(Test::ws9);
    data.push_back(Test::ws10);

    data.push_back(Test::wsw0);
    data.push_back(Test::wsw1);
    data.push_back(Test::wsw2);
    data.push_back(Test::wsw3);
    data.push_back(Test::wsw4);
    data.push_back(Test::wsw5);
    data.push_back(Test::wsw6);
    data.push_back(Test::wsw7);
    data.push_back(Test::wsw8);
    data.push_back(Test::wsw9);
    data.push_back(Test::wsw10);

    data.push_back(Test::wss0);
    data.push_back(Test::wss1);
    data.push_back(Test::wss2);
    data.push_back(Test::wss3);
    data.push_back(Test::wss4);
    data.push_back(Test::wss5);

    data.push_back(Test::wsu0);
    data.push_back(Test::wsu1);
    data.push_back(Test::wsu2);

    return data;
}

#ifdef ICE_CPP11_MAPPING
MyDerivedClassI::OpMStruct1MarshaledResult
MyDerivedClassI::opMStruct1(const Ice::Current& current)
{
    Test::Structure s;
    s.e = ICE_ENUM(MyEnum, enum1); // enum must be initialized
    return OpMStruct1MarshaledResult(s, current);
}

MyDerivedClassI::OpMStruct2MarshaledResult
MyDerivedClassI::opMStruct2(ICE_IN(Test::Structure) p1, const Ice::Current& current)
{
    return OpMStruct2MarshaledResult(p1, p1, current);
}

MyDerivedClassI::OpMSeq1MarshaledResult
MyDerivedClassI::opMSeq1(const Ice::Current& current)
{
    return OpMSeq1MarshaledResult(Test::StringS(), current);
}

MyDerivedClassI::OpMSeq2MarshaledResult
MyDerivedClassI::opMSeq2(ICE_IN(Test::StringS) p1, const Ice::Current& current)
{
    return OpMSeq2MarshaledResult(p1, p1, current);
}

MyDerivedClassI::OpMDict1MarshaledResult
MyDerivedClassI::opMDict1(const Ice::Current& current)
{
    return OpMDict1MarshaledResult(Test::StringStringD(), current);
}

MyDerivedClassI::OpMDict2MarshaledResult
MyDerivedClassI::opMDict2(ICE_IN(Test::StringStringD) p1, const Ice::Current& current)
{
    return OpMDict2MarshaledResult(p1, p1, current);
}

#else

Test::Structure
MyDerivedClassI::opMStruct1(const Ice::Current&)
{
    Test::Structure s;
    s.e = ICE_ENUM(MyEnum, enum1); // enum must be initialized
    return s;
}

Test::Structure
MyDerivedClassI::opMStruct2(ICE_IN(Test::Structure) p1, Test::Structure& p2, const Ice::Current&)
{
    p2 = p1;
    return p1;
}

Test::StringS
MyDerivedClassI::opMSeq1(const Ice::Current&)
{
    return Test::StringS();
}

Test::StringS
MyDerivedClassI::opMSeq2(ICE_IN(Test::StringS) p1, Test::StringS& p2, const Ice::Current&)
{
    p2 = p1;
    return p1;
}

Test::StringStringD
MyDerivedClassI::opMDict1(const Ice::Current&)
{
    return Test::StringStringD();
}

Test::StringStringD
MyDerivedClassI::opMDict2(ICE_IN(Test::StringStringD) p1, Test::StringStringD& p2, const Ice::Current&)
{
    p2 = p1;
    return p1;
}

#endif
