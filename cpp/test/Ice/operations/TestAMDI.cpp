// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <thread>

using namespace Ice;
using namespace Test;

using namespace std;

MyDerivedClassI::MyDerivedClassI() = default;

void
MyDerivedClassI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Current& current)
{
    {
        lock_guard lock(_opVoidMutex);
        if (_opVoidThread.joinable())
        {
            _opVoidThread.join();
            _opVoidThread = thread();
        }
    }

    current.adapter->getCommunicator()->shutdown();
    response();
}

void
MyDerivedClassI::supportsCompressAsync(
    std::function<void(bool)> response,
    std::function<void(std::exception_ptr)>,
    const Current&)
{
    response(true);
}

void
MyDerivedClassI::opVoidAsync(function<void()> response, function<void(exception_ptr)>, const Current& current)
{
    test(current.mode == OperationMode::Normal);

    lock_guard lock(_opVoidMutex);
    if (_opVoidThread.joinable())
    {
        _opVoidThread.join();
        _opVoidThread = thread();
    }

    _opVoidThread = std::thread(response);
}

void
MyDerivedClassI::opByteAsync(
    uint8_t p1,
    uint8_t p2,
    function<void(uint8_t, uint8_t)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(p1, p1 ^ p2);
}

void
MyDerivedClassI::opBoolAsync(
    bool p1,
    bool p2,
    function<void(bool, bool)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(p2, p1);
}

void
MyDerivedClassI::opShortIntLongAsync(
    short p1,
    int p2,
    int64_t p3,
    function<void(int64_t, short, int, int64_t)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(p3, p1, p2, p3);
}

void
MyDerivedClassI::opFloatDoubleAsync(
    float p1,
    double p2,
    function<void(double, float, double)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(p2, p1, p2);
}

void
MyDerivedClassI::opStringAsync(
    string p1,
    string p2,
    function<void(string_view, string_view)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(p1 + " " + p2, p2 + " " + p1);
}

void
MyDerivedClassI::opMyEnumAsync(
    MyEnum p1,
    function<void(MyEnum, MyEnum)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(MyEnum::enum3, p1);
}

void
MyDerivedClassI::opMyClassAsync(
    optional<MyClassPrx> p1,
    function<void(const optional<MyClassPrx>&, const optional<MyClassPrx>&, const optional<MyClassPrx>&)> response,
    function<void(exception_ptr)>,
    const Current& current)
{
    const auto& p2 = p1;
    auto p3 = current.adapter->createProxy<MyClassPrx>(stringToIdentity("noSuchIdentity"));
    response(current.adapter->createProxy<MyClassPrx>(current.id), p2, p3);
}

void
MyDerivedClassI::opStructAsync(
    Structure p1,
    Structure p2,
    function<void(const Structure&, const Structure&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    Structure p3 = p1;
    p3.s.s = "a new string";
    response(p2, p3);
}

void
MyDerivedClassI::opByteSAsync(
    ByteS p1,
    ByteS p2,
    function<void(const ByteS&, const ByteS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ByteS p3;
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    ByteS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    response(r, p3);
}

void
MyDerivedClassI::opBoolSAsync(
    BoolS p1,
    BoolS p2,
    function<void(const BoolS&, const BoolS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    BoolS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    BoolS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opShortIntLongSAsync(
    ShortS p1,
    IntS p2,
    LongS p3,
    function<void(const LongS&, const ShortS&, const IntS&, const LongS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    const ShortS& p4 = p1;
    IntS p5;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    LongS p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    response(p3, p4, p5, p6);
}

void
MyDerivedClassI::opFloatDoubleSAsync(
    FloatS p1,
    DoubleS p2,
    function<void(const DoubleS&, const FloatS&, const DoubleS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    FloatS p3 = p1;
    DoubleS p4;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    DoubleS r = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(r));
    response(r, p3, p4);
}

void
MyDerivedClassI::opStringSAsync(
    StringS p1,
    StringS p2,
    function<void(const StringS&, const StringS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    StringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opByteSSAsync(
    ByteSS p1,
    ByteSS p2,
    function<void(const ByteSS&, const ByteSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ByteSS p3;
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    ByteSS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    response(r, p3);
}

void
MyDerivedClassI::opBoolSSAsync(
    BoolSS p1,
    BoolSS p2,
    function<void(const BoolSS&, const BoolSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    auto p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    BoolSS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opShortIntLongSSAsync(
    ShortSS p1,
    IntSS p2,
    LongSS p3,
    function<void(const LongSS&, const ShortSS&, const IntSS&, const LongSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    const auto& p4 = p1;
    IntSS p5;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    auto p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    response(p3, p4, p5, p6);
}

void
MyDerivedClassI::opFloatDoubleSSAsync(
    FloatSS p1,
    DoubleSS p2,
    function<void(const DoubleSS&, const FloatSS&, const DoubleSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    const FloatSS& p3 = p1;
    DoubleSS p4;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    DoubleSS r = p2;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    response(r, p3, p4);
}

void
MyDerivedClassI::opStringSSAsync(
    StringSS p1,
    StringSS p2,
    function<void(const StringSS&, const StringSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringSS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    StringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opStringSSSAsync(
    StringSSS p1,
    StringSSS p2,
    function<void(const StringSSS&, const StringSSS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringSSS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    StringSSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opByteBoolDAsync(
    ByteBoolD p1,
    ByteBoolD p2,
    function<void(const ByteBoolD&, const ByteBoolD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ByteBoolD p3 = p1;
    ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opShortIntDAsync(
    ShortIntD p1,
    ShortIntD p2,
    function<void(const ShortIntD&, const ShortIntD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ShortIntD p3 = p1;
    ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opLongFloatDAsync(
    LongFloatD p1,
    LongFloatD p2,
    function<void(const LongFloatD&, const LongFloatD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    LongFloatD p3 = p1;
    LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opStringStringDAsync(
    StringStringD p1,
    StringStringD p2,
    function<void(const StringStringD&, const StringStringD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringStringD p3 = p1;
    StringStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opStringMyEnumDAsync(
    StringMyEnumD p1,
    StringMyEnumD p2,
    function<void(const StringMyEnumD&, const StringMyEnumD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringMyEnumD p3 = p1;
    StringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opMyEnumStringDAsync(
    MyEnumStringD p1,
    MyEnumStringD p2,
    function<void(const MyEnumStringD&, const MyEnumStringD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    MyEnumStringD p3 = p1;
    MyEnumStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opMyStructMyEnumDAsync(
    MyStructMyEnumD p1,
    MyStructMyEnumD p2,
    function<void(const MyStructMyEnumD&, const MyStructMyEnumD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    MyStructMyEnumD p3 = p1;
    MyStructMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opByteBoolDSAsync(
    ByteBoolDS p1,
    ByteBoolDS p2,
    function<void(const ByteBoolDS&, const ByteBoolDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ByteBoolDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    ByteBoolDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opShortIntDSAsync(
    ShortIntDS p1,
    ShortIntDS p2,
    function<void(const ShortIntDS&, const ShortIntDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ShortIntDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    ShortIntDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opLongFloatDSAsync(
    LongFloatDS p1,
    LongFloatDS p2,
    function<void(const LongFloatDS&, const LongFloatDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    LongFloatDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    LongFloatDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opStringStringDSAsync(
    StringStringDS p1,
    StringStringDS p2,
    function<void(const StringStringDS&, const StringStringDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringStringDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    StringStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opStringMyEnumDSAsync(
    StringMyEnumDS p1,
    StringMyEnumDS p2,
    function<void(const StringMyEnumDS&, const StringMyEnumDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringMyEnumDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    StringMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opMyEnumStringDSAsync(
    MyEnumStringDS p1,
    MyEnumStringDS p2,
    function<void(const MyEnumStringDS&, const MyEnumStringDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    MyEnumStringDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    MyEnumStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opMyStructMyEnumDSAsync(
    MyStructMyEnumDS p1,
    MyStructMyEnumDS p2,
    function<void(const MyStructMyEnumDS&, const MyStructMyEnumDS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    MyStructMyEnumDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    MyStructMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    response(r, p3);
}

void
MyDerivedClassI::opByteByteSDAsync(
    ByteByteSD p1,
    ByteByteSD p2,
    function<void(const ByteByteSD&, const ByteByteSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ByteByteSD p3 = p2;
    ByteByteSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opBoolBoolSDAsync(
    BoolBoolSD p1,
    BoolBoolSD p2,
    function<void(const BoolBoolSD&, const BoolBoolSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    BoolBoolSD p3 = p2;
    BoolBoolSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opShortShortSDAsync(
    ShortShortSD p1,
    ShortShortSD p2,
    function<void(const ShortShortSD&, const ShortShortSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    ShortShortSD p3 = p2;
    ShortShortSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opIntIntSDAsync(
    IntIntSD p1,
    IntIntSD p2,
    function<void(const IntIntSD&, const IntIntSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    IntIntSD p3 = p2;
    IntIntSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opLongLongSDAsync(
    LongLongSD p1,
    LongLongSD p2,
    function<void(const LongLongSD&, const LongLongSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    LongLongSD p3 = p2;
    LongLongSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opStringFloatSDAsync(
    StringFloatSD p1,
    StringFloatSD p2,
    function<void(const StringFloatSD&, const StringFloatSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringFloatSD p3 = p2;
    StringFloatSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opStringDoubleSDAsync(
    StringDoubleSD p1,
    StringDoubleSD p2,
    function<void(const StringDoubleSD&, const StringDoubleSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringDoubleSD p3 = p2;
    StringDoubleSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opStringStringSDAsync(
    StringStringSD p1,
    StringStringSD p2,
    function<void(const StringStringSD&, const StringStringSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringStringSD p3 = p2;
    StringStringSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opMyEnumMyEnumSDAsync(
    MyEnumMyEnumSD p1,
    MyEnumMyEnumSD p2,
    function<void(const MyEnumMyEnumSD&, const MyEnumMyEnumSD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    MyEnumMyEnumSD p3 = p2;
    MyEnumMyEnumSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    response(r, p3);
}

void
MyDerivedClassI::opIntSAsync(
    IntS s,
    function<void(const IntS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    IntS r;
    std::transform(s.begin(), s.end(), std::back_inserter(r), std::negate<>());
    response(r);
}

void
MyDerivedClassI::opByteSOnewayAsync(ByteS, function<void()> response, function<void(exception_ptr)>, const Current&)
{
    lock_guard lock(_mutex);
    ++_opByteSOnewayCallCount;
    response();
}

void
MyDerivedClassI::opByteSOnewayCallCountAsync(
    function<void(int)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    lock_guard lock(_mutex);
    response(_opByteSOnewayCallCount);
    _opByteSOnewayCallCount = 0;
}

void
MyDerivedClassI::opContextAsync(
    function<void(const Context&)> response,
    function<void(exception_ptr)>,
    const Current& current)
{
    response(current.ctx);
}

void
MyDerivedClassI::opDoubleMarshalingAsync(
    double p1,
    DoubleS p2,
    function<void()> response,
    function<void(exception_ptr)>,
    const Current&)
{
    double d = 1278312346.0 / 13.0;
    test(p1 == d);
    for (double i : p2)
    {
        test(i == d);
    }
    response();
}

void
MyDerivedClassI::opIdempotentAsync(function<void()> response, function<void(exception_ptr)>, const Current& current)
{
    test(current.mode == OperationMode::Idempotent);
    response();
}

void
MyDerivedClassI::opDerivedAsync(function<void()> response, function<void(exception_ptr)>, const Current&)
{
    response();
}

void
MyDerivedClassI::opByte1Async(
    uint8_t b,
    function<void(uint8_t)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(b);
}

void
MyDerivedClassI::opShort1Async(
    int16_t s,
    function<void(int16_t)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(s);
}

void
MyDerivedClassI::opInt1Async(int32_t i, function<void(int32_t)> response, function<void(exception_ptr)>, const Current&)
{
    response(i);
}

void
MyDerivedClassI::opLong1Async(
    int64_t l,
    function<void(int64_t)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(l);
}

void
MyDerivedClassI::opFloat1Async(float f, function<void(float)> response, function<void(exception_ptr)>, const Current&)
{
    response(f);
}

void
MyDerivedClassI::opDouble1Async(
    double d,
    function<void(double)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(d);
}

void
MyDerivedClassI::opString1Async(
    string s,
    function<void(string_view)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(s);
}

void
MyDerivedClassI::opStringS1Async(
    StringS seq,
    function<void(const StringS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(seq);
}

void
MyDerivedClassI::opByteBoolD1Async(
    ByteBoolD dict,
    function<void(const ByteBoolD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(dict);
}

void
MyDerivedClassI::opStringS2Async(
    StringS seq,
    function<void(const StringS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(seq);
}

void
MyDerivedClassI::opByteBoolD2Async(
    ByteBoolD dict,
    function<void(const ByteBoolD&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(dict);
}

void
MyDerivedClassI::opMyStruct1Async(
    MyStruct1 s,
    function<void(const MyStruct1&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(s);
}

void
MyDerivedClassI::opMyClass1Async(
    shared_ptr<MyClass1> c,
    function<void(const shared_ptr<MyClass1>&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response(c);
}

void
MyDerivedClassI::opStringLiteralsAsync(
    function<void(const StringS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    StringS data;
    data.push_back(s0);
    data.push_back(s1);
    data.push_back(s2);
    data.push_back(s3);
    data.push_back(s4);
    data.push_back(s5);
    data.push_back(s6);
    data.push_back(s7);
    data.push_back(s8);
    data.push_back(s9);
    data.push_back(s10);

    data.push_back(sw0);
    data.push_back(sw1);
    data.push_back(sw2);
    data.push_back(sw3);
    data.push_back(sw4);
    data.push_back(sw5);
    data.push_back(sw6);
    data.push_back(sw7);
    data.push_back(sw8);
    data.push_back(sw9);
    data.push_back(sw10);

    data.push_back(ss0);
    data.push_back(ss1);
    data.push_back(ss2);
    data.push_back(ss3);
    data.push_back(ss4);
    data.push_back(ss5);

    data.push_back(su0);
    data.push_back(su1);
    data.push_back(su2);

    response(data);
}

void
MyDerivedClassI::opWStringLiteralsAsync(
    function<void(const WStringS&)> response,
    function<void(exception_ptr)>,
    const Current&)
{
    WStringS data;
    data.push_back(ws0);
    data.push_back(ws1);
    data.push_back(ws2);
    data.push_back(ws3);
    data.push_back(ws4);
    data.push_back(ws5);
    data.push_back(ws6);
    data.push_back(ws7);
    data.push_back(ws8);
    data.push_back(ws9);
    data.push_back(ws10);

    data.push_back(wsw0);
    data.push_back(wsw1);
    data.push_back(wsw2);
    data.push_back(wsw3);
    data.push_back(wsw4);
    data.push_back(wsw5);
    data.push_back(wsw6);
    data.push_back(wsw7);
    data.push_back(wsw8);
    data.push_back(wsw9);
    data.push_back(wsw10);

    data.push_back(wss0);
    data.push_back(wss1);
    data.push_back(wss2);
    data.push_back(wss3);
    data.push_back(wss4);
    data.push_back(wss5);

    data.push_back(wsu0);
    data.push_back(wsu1);
    data.push_back(wsu2);

    response(data);
}

void
MyDerivedClassI::opMStruct1Async(
    function<void(OpMStruct1MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    Structure s;
    s.e = MyEnum::enum1; // enum must be initialized
    response(OpMStruct1MarshaledResult(s, current));
}

void
MyDerivedClassI::opMStruct2Async(
    Structure p1,
    function<void(OpMStruct2MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    response(OpMStruct2MarshaledResult(p1, p1, current));
}

void
MyDerivedClassI::opMSeq1Async(
    function<void(OpMSeq1MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    response(OpMSeq1MarshaledResult(StringS(), current));
}

void
MyDerivedClassI::opMSeq2Async(
    StringS p1,
    function<void(OpMSeq2MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    response(OpMSeq2MarshaledResult(p1, p1, current));
}

void
MyDerivedClassI::opMDict1Async(
    function<void(OpMDict1MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    response(OpMDict1MarshaledResult(StringStringD(), current));
}

void
MyDerivedClassI::opMDict2Async(
    StringStringD p1,
    function<void(OpMDict2MarshaledResult)> response,
    function<void(std::exception_ptr)>,
    const Current& current)
{
    response(OpMDict2MarshaledResult(p1, p1, current));
}
