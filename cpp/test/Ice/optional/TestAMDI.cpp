// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace Ice;
using namespace std;

InitialI::InitialI() = default;

void
InitialI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
InitialI::pingPongAsync(
    shared_ptr<Ice::Value> obj,
    function<void(const shared_ptr<Ice::Value>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(obj);
}

void
InitialI::opOptionalExceptionAsync(
    optional<int> a,
    optional<string> b,
    function<void()>,
    function<void(exception_ptr)> ex,
    const Ice::Current&)
{
    ex(make_exception_ptr(OptionalException(false, a, b)));
}

void
InitialI::opDerivedExceptionAsync(
    optional<int> a,
    optional<string> b,
    function<void()>,
    function<void(exception_ptr)> ex,
    const Ice::Current&)
{
    ex(make_exception_ptr(DerivedException(false, a, b, "d1", b, "d2")));
}

void
InitialI::opRequiredExceptionAsync(
    optional<int> a,
    optional<string> b,
    function<void()>,
    function<void(exception_ptr)> ex,
    const Ice::Current&)
{
    RequiredException e;
    e.a = a;
    e.b = b;
    if (b)
    {
        e.ss = b.value();
    }

    ex(make_exception_ptr(e));
}

void
InitialI::opByteAsync(
    optional<uint8_t> p1,
    function<void(optional<uint8_t>, optional<uint8_t>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolAsync(
    optional<bool> p1,
    function<void(optional<bool>, optional<bool>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortAsync(
    optional<short> p1,
    function<void(optional<short>, optional<short>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntAsync(
    optional<int> p1,
    function<void(optional<int>, optional<int>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongAsync(
    optional<int64_t> p1,
    function<void(optional<int64_t>, optional<int64_t>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatAsync(
    optional<float> p1,
    function<void(optional<float>, optional<float>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleAsync(
    optional<double> p1,
    function<void(optional<double>, optional<double>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringAsync(
    optional<string> p1,
    function<void(optional<string_view>, optional<string_view>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMyEnumAsync(
    optional<Test::MyEnum> p1,
    function<void(optional<Test::MyEnum>, optional<Test::MyEnum>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructAsync(
    optional<Test::SmallStruct> p1,
    function<void(const optional<Test::SmallStruct>&, const optional<Test::SmallStruct>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructAsync(
    optional<Test::FixedStruct> p1,
    function<void(const optional<Test::FixedStruct>&, const optional<Test::FixedStruct>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructAsync(
    optional<Test::VarStruct> p1,
    function<void(const optional<Test::VarStruct>&, const optional<Test::VarStruct>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMyInterfaceProxyAsync(
    optional<::MyInterfacePrx> p1,
    function<void(const optional<::MyInterfacePrx>&, const optional<::MyInterfacePrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opOneOptionalAsync(
    shared_ptr<Test::OneOptional> p1,
    function<void(const shared_ptr<Test::OneOptional>&, const shared_ptr<Test::OneOptional>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opByteSeqAsync(
    optional<pair<const byte*, const byte*>> p1,
    function<void(optional<pair<const byte*, const byte*>>, optional<pair<const byte*, const byte*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolSeqAsync(
    optional<pair<const bool*, const bool*>> p1,
    function<void(optional<pair<const bool*, const bool*>>, optional<pair<const bool*, const bool*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortSeqAsync(
    optional<pair<const short*, const short*>> p1,
    function<void(optional<pair<const short*, const short*>>, optional<pair<const short*, const short*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntSeqAsync(
    optional<pair<const int*, const int*>> p1,
    function<void(optional<pair<const int*, const int*>>, optional<pair<const int*, const int*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongSeqAsync(
    optional<pair<const int64_t*, const int64_t*>> p1,
    function<void(optional<pair<const int64_t*, const int64_t*>>, optional<pair<const int64_t*, const int64_t*>>)>
        response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatSeqAsync(
    optional<pair<const float*, const float*>> p1,
    function<void(optional<pair<const float*, const float*>>, optional<pair<const float*, const float*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleSeqAsync(
    optional<pair<const double*, const double*>> p1,
    function<void(optional<pair<const double*, const double*>>, optional<pair<const double*, const double*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringSeqAsync(
    optional<Test::StringSeq> p1,
    function<void(const optional<Test::StringSeq>&, const optional<Test::StringSeq>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructSeqAsync(
    optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>> p1,
    function<void(
        optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
        optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructListAsync(
    optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>> p1,
    function<void(
        optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
        optional<pair<const Test::SmallStruct*, const Test::SmallStruct*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructSeqAsync(
    optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>> p1,
    function<void(
        optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
        optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructListAsync(
    optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>> p1,
    function<void(
        optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
        optional<pair<const Test::FixedStruct*, const Test::FixedStruct*>>)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructSeqAsync(
    optional<Test::VarStructSeq> p1,
    function<void(const optional<Test::VarStructSeq>&, const optional<Test::VarStructSeq>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSerializableAsync(
    optional<Test::Serializable> p1,
    function<void(const optional<Test::Serializable>&, const optional<Test::Serializable>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntIntDictAsync(
    optional<Test::IntIntDict> p1,
    function<void(const optional<Test::IntIntDict>&, const optional<Test::IntIntDict>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringIntDictAsync(
    optional<Test::StringIntDict> p1,
    function<void(const optional<Test::StringIntDict>&, const optional<Test::StringIntDict>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opClassAndUnknownOptionalAsync(
    shared_ptr<Test::A>,
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response();
}

void
InitialI::opGAsync(
    shared_ptr<Test::G> g,
    function<void(const shared_ptr<Test::G>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(g);
}

void
InitialI::opVoidAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    response();
}

void
InitialI::opMStruct1Async(
    function<void(OpMStruct1MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMStruct1MarshaledResult(Test::SmallStruct(), current));
}

void
InitialI::opMStruct2Async(
    optional<SmallStruct> p1,
    function<void(OpMStruct2MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMStruct2MarshaledResult(p1, p1, current));
}

void
InitialI::opMSeq1Async(
    function<void(OpMSeq1MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMSeq1MarshaledResult(Test::StringSeq(), current));
}

void
InitialI::opMSeq2Async(
    optional<Test::StringSeq> p1,
    function<void(OpMSeq2MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMSeq2MarshaledResult(p1, p1, current));
}

void
InitialI::opMDict1Async(
    function<void(OpMDict1MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMDict1MarshaledResult(StringIntDict(), current));
}

void
InitialI::opMDict2Async(
    optional<StringIntDict> p1,
    function<void(OpMDict2MarshaledResult)> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    response(OpMDict2MarshaledResult(p1, p1, current));
}

void
InitialI::supportsJavaSerializableAsync(
    function<void(bool)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(true);
}
