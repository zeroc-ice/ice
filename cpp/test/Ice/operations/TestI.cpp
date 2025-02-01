// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <algorithm>
#include <functional>
#include <iterator>

using namespace Ice;
using namespace Test;
using namespace std;

MyDerivedClassI::MyDerivedClassI() = default;

void
MyDerivedClassI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

bool
MyDerivedClassI::supportsCompress(const Ice::Current&)
{
    return true;
}

void
MyDerivedClassI::opVoid(const Ice::Current& current)
{
    test(current.mode == OperationMode::Normal);
}

uint8_t
MyDerivedClassI::opByte(uint8_t p1, uint8_t p2, uint8_t& p3, const Ice::Current&)
{
    p3 = p1 ^ p2;
    return p1;
}

bool
MyDerivedClassI::opBool(bool p1, bool p2, bool& p3, const Ice::Current&)
{
    p3 = p1;
    return p2;
}

int64_t
MyDerivedClassI::opShortIntLong(
    int16_t p1,
    int32_t p2,
    int64_t p3,
    int16_t& p4,
    int32_t& p5,
    int64_t& p6,
    const Ice::Current&)
{
    p4 = p1;
    p5 = p2;
    p6 = p3;
    return p3;
}

double
MyDerivedClassI::opFloatDouble(float p1, double p2, float& p3, double& p4, const Ice::Current&)
{
    p3 = p1;
    p4 = p2;
    return p2;
}

std::string
MyDerivedClassI::opString(string p1, string p2, string& p3, const Ice::Current&)
{
    p3 = p2 + " " + p1;
    return p1 + " " + p2;
}

Test::MyEnum
MyDerivedClassI::opMyEnum(Test::MyEnum p1, Test::MyEnum& p2, const Ice::Current&)
{
    p2 = p1;
    return MyEnum::enum3;
}

optional<Test::MyClassPrx>
MyDerivedClassI::opMyClass(
    optional<Test::MyClassPrx> p1,
    optional<Test::MyClassPrx>& p2,
    optional<Test::MyClassPrx>& p3,
    const Ice::Current& current)
{
    p2 = p1;
    p3 = current.adapter->createProxy<Test::MyClassPrx>(stringToIdentity("noSuchIdentity"));
    return current.adapter->createProxy<Test::MyClassPrx>(current.id);
}

Test::Structure
MyDerivedClassI::opStruct(Test::Structure p1, Test::Structure p2, Test::Structure& p3, const Ice::Current&)
{
    p3 = p1;
    p3.s.s = "a new string";
    return p2;
}

Test::ByteS
MyDerivedClassI::opByteS(Test::ByteS p1, Test::ByteS p2, Test::ByteS& p3, const Ice::Current&)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolS
MyDerivedClassI::opBoolS(Test::BoolS p1, Test::BoolS p2, Test::BoolS& p3, const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongS
MyDerivedClassI::opShortIntLongS(
    Test::ShortS p1,
    Test::IntS p2,
    Test::LongS p3,
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
MyDerivedClassI::opFloatDoubleS(
    Test::FloatS p1,
    Test::DoubleS p2,
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
MyDerivedClassI::opStringS(Test::StringS p1, Test::StringS p2, Test::StringS& p3, const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ByteSS
MyDerivedClassI::opByteSS(Test::ByteSS p1, Test::ByteSS p2, Test::ByteSS& p3, const Ice::Current&)
{
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteSS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    return r;
}

Test::BoolSS
MyDerivedClassI::opBoolSS(Test::BoolSS p1, Test::BoolSS p2, Test::BoolSS& p3, const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolSS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongSS
MyDerivedClassI::opShortIntLongSS(
    Test::ShortSS p1,
    Test::IntSS p2,
    Test::LongSS p3,
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
MyDerivedClassI::opFloatDoubleSS(
    Test::FloatSS p1,
    Test::DoubleSS p2,
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
MyDerivedClassI::opStringSS(Test::StringSS p1, Test::StringSS p2, Test::StringSS& p3, const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::StringSSS
MyDerivedClassI::opStringSSS(Test::StringSSS p1, Test::StringSSS p2, Test::StringSSS& p3, const Ice::Current&)
{
    p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    return r;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD(Test::ByteBoolD p1, Test::ByteBoolD p2, Test::ByteBoolD& p3, const Ice::Current&)
{
    p3 = p1;
    Test::ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortIntD
MyDerivedClassI::opShortIntD(Test::ShortIntD p1, Test::ShortIntD p2, Test::ShortIntD& p3, const Ice::Current&)
{
    p3 = p1;
    Test::ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongFloatD
MyDerivedClassI::opLongFloatD(Test::LongFloatD p1, Test::LongFloatD p2, Test::LongFloatD& p3, const Ice::Current&)
{
    p3 = p1;
    Test::LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringStringD
MyDerivedClassI::opStringStringD(
    Test::StringStringD p1,
    Test::StringStringD p2,
    Test::StringStringD& p3,
    const Ice::Current&)
{
    p3 = p1;
    Test::StringStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringMyEnumD
MyDerivedClassI::opStringMyEnumD(
    Test::StringMyEnumD p1,
    Test::StringMyEnumD p2,
    Test::StringMyEnumD& p3,
    const Ice::Current&)
{
    p3 = p1;
    Test::StringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyEnumStringD
MyDerivedClassI::opMyEnumStringD(
    Test::MyEnumStringD p1,
    Test::MyEnumStringD p2,
    Test::MyEnumStringD& p3,
    const Ice::Current&)
{
    p3 = p1;
    Test::MyEnumStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyStructMyEnumD
MyDerivedClassI::opMyStructMyEnumD(
    Test::MyStructMyEnumD p1,
    Test::MyStructMyEnumD p2,
    Test::MyStructMyEnumD& p3,
    const Ice::Current&)
{
    p3 = p1;
    Test::MyStructMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ByteBoolDS
MyDerivedClassI::opByteBoolDS(Test::ByteBoolDS p1, Test::ByteBoolDS p2, Test::ByteBoolDS& p3, const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ByteBoolDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::ShortIntDS
MyDerivedClassI::opShortIntDS(Test::ShortIntDS p1, Test::ShortIntDS p2, Test::ShortIntDS& p3, const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ShortIntDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::LongFloatDS
MyDerivedClassI::opLongFloatDS(Test::LongFloatDS p1, Test::LongFloatDS p2, Test::LongFloatDS& p3, const Ice::Current&)
{
    p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::LongFloatDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    return r;
}

Test::StringStringDS
MyDerivedClassI::opStringStringDS(
    Test::StringStringDS p1,
    Test::StringStringDS p2,
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
MyDerivedClassI::opStringMyEnumDS(
    Test::StringMyEnumDS p1,
    Test::StringMyEnumDS p2,
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
MyDerivedClassI::opMyEnumStringDS(
    Test::MyEnumStringDS p1,
    Test::MyEnumStringDS p2,
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
MyDerivedClassI::opMyStructMyEnumDS(
    Test::MyStructMyEnumDS p1,
    Test::MyStructMyEnumDS p2,
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
MyDerivedClassI::opByteByteSD(Test::ByteByteSD p1, Test::ByteByteSD p2, Test::ByteByteSD& p3, const Ice::Current&)
{
    p3 = p2;
    Test::ByteByteSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::BoolBoolSD
MyDerivedClassI::opBoolBoolSD(Test::BoolBoolSD p1, Test::BoolBoolSD p2, Test::BoolBoolSD& p3, const Ice::Current&)
{
    p3 = p2;
    Test::BoolBoolSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::ShortShortSD
MyDerivedClassI::opShortShortSD(
    Test::ShortShortSD p1,
    Test::ShortShortSD p2,
    Test::ShortShortSD& p3,
    const Ice::Current&)
{
    p3 = p2;
    Test::ShortShortSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::IntIntSD
MyDerivedClassI::opIntIntSD(Test::IntIntSD p1, Test::IntIntSD p2, Test::IntIntSD& p3, const Ice::Current&)
{
    p3 = p2;
    Test::IntIntSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::LongLongSD
MyDerivedClassI::opLongLongSD(Test::LongLongSD p1, Test::LongLongSD p2, Test::LongLongSD& p3, const Ice::Current&)
{
    p3 = p2;
    Test::LongLongSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringFloatSD
MyDerivedClassI::opStringFloatSD(
    Test::StringFloatSD p1,
    Test::StringFloatSD p2,
    Test::StringFloatSD& p3,
    const Ice::Current&)
{
    p3 = p2;
    Test::StringFloatSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringDoubleSD
MyDerivedClassI::opStringDoubleSD(
    Test::StringDoubleSD p1,
    Test::StringDoubleSD p2,
    Test::StringDoubleSD& p3,
    const Ice::Current&)
{
    p3 = p2;
    Test::StringDoubleSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::StringStringSD
MyDerivedClassI::opStringStringSD(
    Test::StringStringSD p1,
    Test::StringStringSD p2,
    Test::StringStringSD& p3,
    const Ice::Current&)
{
    p3 = p2;
    Test::StringStringSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::MyEnumMyEnumSD
MyDerivedClassI::opMyEnumMyEnumSD(
    Test::MyEnumMyEnumSD p1,
    Test::MyEnumMyEnumSD p2,
    Test::MyEnumMyEnumSD& p3,
    const Ice::Current&)
{
    p3 = p2;
    Test::MyEnumMyEnumSD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    return r;
}

Test::IntS
MyDerivedClassI::opIntS(Test::IntS s, const Ice::Current&)
{
    Test::IntS r;
    std::transform(s.begin(), s.end(), std::back_inserter(r), std::negate<>());
    return r;
}

void
MyDerivedClassI::opByteSOneway(Test::ByteS, const Ice::Current&)
{
    lock_guard lock(_mutex);
    ++_opByteSOnewayCallCount;
}

int
MyDerivedClassI::opByteSOnewayCallCount(const Ice::Current&)
{
    lock_guard lock(_mutex);
    int count = _opByteSOnewayCallCount;
    _opByteSOnewayCallCount = 0;
    return count;
}

Ice::Context
MyDerivedClassI::opContext(const Ice::Current& c)
{
    return c.ctx;
}

void
MyDerivedClassI::opDoubleMarshaling(double p1, Test::DoubleS p2, const Ice::Current&)
{
    double d = 1278312346.0 / 13.0;
    test(p1 == d);
    for (double i : p2)
    {
        test(i == d);
    }
}

void
MyDerivedClassI::opIdempotent(const Ice::Current& current)
{
    test(current.mode == OperationMode::Idempotent);
}

void
MyDerivedClassI::opDerived(const Ice::Current&)
{
}

uint8_t
MyDerivedClassI::opByte1(uint8_t b, const Ice::Current&)
{
    return b;
}

int16_t
MyDerivedClassI::opShort1(int16_t s, const Ice::Current&)
{
    return s;
}

int32_t
MyDerivedClassI::opInt1(int32_t i, const Ice::Current&)
{
    return i;
}

int64_t
MyDerivedClassI::opLong1(int64_t l, const Ice::Current&)
{
    return l;
}

float
MyDerivedClassI::opFloat1(float f, const Ice::Current&)
{
    return f;
}

double
MyDerivedClassI::opDouble1(double d, const Ice::Current&)
{
    return d;
}

std::string
MyDerivedClassI::opString1(string s, const Ice::Current&)
{
    return s;
}

Test::StringS
MyDerivedClassI::opStringS1(Test::StringS seq, const Ice::Current&)
{
    return seq;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD1(Test::ByteBoolD dict, const Ice::Current&)
{
    return dict;
}

Test::StringS
MyDerivedClassI::opStringS2(Test::StringS seq, const Ice::Current&)
{
    return seq;
}

Test::ByteBoolD
MyDerivedClassI::opByteBoolD2(Test::ByteBoolD dict, const Ice::Current&)
{
    return dict;
}

Test::MyStruct1
MyDerivedClassI::opMyStruct1(Test::MyStruct1 s, const Ice::Current&)
{
    return s;
}

Test::MyClass1Ptr
MyDerivedClassI::opMyClass1(Test::MyClass1Ptr c, const Ice::Current&)
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

MyDerivedClassI::OpMStruct1MarshaledResult
MyDerivedClassI::opMStruct1(const Ice::Current& current)
{
    Test::Structure s;
    s.e = MyEnum::enum1; // enum must be initialized
    return {s, current};
}

MyDerivedClassI::OpMStruct2MarshaledResult
MyDerivedClassI::opMStruct2(Test::Structure p1, const Ice::Current& current)
{
    return {p1, p1, current};
}

MyDerivedClassI::OpMSeq1MarshaledResult
MyDerivedClassI::opMSeq1(const Ice::Current& current)
{
    return {Test::StringS{}, current};
}

MyDerivedClassI::OpMSeq2MarshaledResult
MyDerivedClassI::opMSeq2(Test::StringS p1, const Ice::Current& current)
{
    return {p1, p1, current};
}

MyDerivedClassI::OpMDict1MarshaledResult
MyDerivedClassI::opMDict1(const Ice::Current& current)
{
    return {Test::StringStringD{}, current};
}

MyDerivedClassI::OpMDict2MarshaledResult
MyDerivedClassI::opMDict2(Test::StringStringD p1, const Ice::Current& current)
{
    return {p1, p1, current};
}
