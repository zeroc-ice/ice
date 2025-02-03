// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;
using namespace Ice;
using namespace std;

InitialI::InitialI() = default;

void
InitialI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

Test::Initial::PingPongMarshaledResult
InitialI::pingPong(shared_ptr<Value> obj, const Current& current)
{
    return {obj, current};
}

void
InitialI::opOptionalException(optional<int32_t> a, optional<string> b, const Ice::Current&)
{
    OptionalException ex;
    ex.a = a;
    ex.b = b;
    throw ex; // NOLINT(cert-err09-cpp, cert-err61-cpp)
}

void
InitialI::opDerivedException(optional<int32_t> a, optional<string> b, const Ice::Current&)
{
    DerivedException ex;
    ex.a = a;
    ex.b = b;
    ex.ss = b;
    ex.d1 = "d1";
    ex.d2 = "d2";
    throw ex; // NOLINT(cert-err09-cpp, cert-err61-cpp)
}

void
InitialI::opRequiredException(optional<int32_t> a, optional<string> b, const Ice::Current&)
{
    RequiredException ex;
    ex.a = a;
    ex.b = b;
    if (b)
    {
        ex.ss = b.value();
    }
    throw ex; // NOLINT(cert-err09-cpp, cert-err61-cpp)
}

optional<uint8_t>
InitialI::opByte(optional<uint8_t> p1, optional<uint8_t>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<bool>
InitialI::opBool(optional<bool> p1, optional<bool>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<int16_t>
InitialI::opShort(optional<int16_t> p1, optional<int16_t>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<int32_t>
InitialI::opInt(optional<int32_t> p1, optional<int32_t>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<int64_t>
InitialI::opLong(optional<int64_t> p1, optional<int64_t>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<float>
InitialI::opFloat(optional<float> p1, optional<float>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<double>
InitialI::opDouble(optional<double> p1, optional<double>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<string>
InitialI::opString(optional<string> p1, optional<string>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<MyEnum>
InitialI::opMyEnum(optional<MyEnum> p1, optional<MyEnum>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<SmallStruct>
InitialI::opSmallStruct(optional<SmallStruct> p1, optional<SmallStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<FixedStruct>
InitialI::opFixedStruct(optional<FixedStruct> p1, optional<FixedStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<VarStruct>
InitialI::opVarStruct(optional<VarStruct> p1, optional<VarStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<MyInterfacePrx>
InitialI::opMyInterfaceProxy(optional<MyInterfacePrx> p1, optional<MyInterfacePrx>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

OneOptionalPtr
InitialI::opOneOptional(OneOptionalPtr p1, OneOptionalPtr& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Test::ByteSeq>
InitialI::opByteSeq(optional<pair<const byte*, const byte*>> p1, optional<Test::ByteSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Ice::ByteSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::BoolSeq>
InitialI::opBoolSeq(optional<pair<const bool*, const bool*>> p1, optional<Test::BoolSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Ice::BoolSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::ShortSeq>
InitialI::opShortSeq(optional<pair<const int16_t*, const int16_t*>> p1, optional<Test::ShortSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Ice::ShortSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::IntSeq>
InitialI::opIntSeq(optional<pair<const int32_t*, const int32_t*>> p1, optional<Test::IntSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Test::IntSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::LongSeq>
InitialI::opLongSeq(optional<pair<const int64_t*, const int64_t*>> p1, optional<Test::LongSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Test::LongSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::FloatSeq>
InitialI::opFloatSeq(optional<pair<const float*, const float*>> p1, optional<Test::FloatSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Test::FloatSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::DoubleSeq>
InitialI::opDoubleSeq(optional<pair<const double*, const double*>> p1, optional<Test::DoubleSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = Test::DoubleSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Ice::StringSeq>
InitialI::opStringSeq(optional<Ice::StringSeq> p1, optional<Ice::StringSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = p1;
    }
    return p3;
}

optional<SmallStructSeq>
InitialI::opSmallStructSeq(
    optional<pair<const SmallStruct*, const SmallStruct*>> p1,
    optional<SmallStructSeq>& p3,
    const Current&)
{
    if (p1)
    {
        p3 = SmallStructSeq(p1->first, p1->second);
    }
    return p3;
}

optional<SmallStructList>
InitialI::opSmallStructList(
    optional<pair<const SmallStruct*, const SmallStruct*>> p1,
    optional<SmallStructList>& p3,
    const Current&)
{
    if (p1)
    {
        p3 = SmallStructList(p1->first, p1->second);
    }
    return p3;
}

optional<FixedStructSeq>
InitialI::opFixedStructSeq(
    optional<pair<const FixedStruct*, const FixedStruct*>> p1,
    optional<FixedStructSeq>& p3,
    const Current&)
{
    if (p1)
    {
        p3 = FixedStructSeq(p1->first, p1->second);
    }
    return p3;
}

optional<FixedStructList>
InitialI::opFixedStructList(
    optional<pair<const FixedStruct*, const FixedStruct*>> p1,
    optional<FixedStructList>& p3,
    const Current&)
{
    if (p1)
    {
        p3 = FixedStructList(p1->first, p1->second);
    }
    return p3;
}

optional<VarStructSeq>
InitialI::opVarStructSeq(optional<VarStructSeq> p1, optional<VarStructSeq>& p3, const Current&)
{
    if (p1)
    {
        p3 = p1;
    }
    return p3;
}

optional<Serializable>
InitialI::opSerializable(optional<Serializable> p1, optional<Serializable>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

optional<IntIntDict>
InitialI::opIntIntDict(optional<IntIntDict> p1, optional<IntIntDict>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

optional<StringIntDict>
InitialI::opStringIntDict(optional<StringIntDict> p1, optional<StringIntDict>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

void
InitialI::opClassAndUnknownOptional(APtr, const Ice::Current&)
{
}

GPtr
InitialI::opG(GPtr g, const Ice::Current&)
{
    return g;
}

void
InitialI::opVoid(const Ice::Current&)
{
}

InitialI::OpMStruct1MarshaledResult
InitialI::opMStruct1(const Ice::Current& current)
{
    return {Test::SmallStruct(), current};
}

InitialI::OpMStruct2MarshaledResult
InitialI::opMStruct2(optional<Test::SmallStruct> p1, const Ice::Current& current)
{
    return {p1, p1, current};
}

InitialI::OpMSeq1MarshaledResult
InitialI::opMSeq1(const Ice::Current& current)
{
    return {Test::StringSeq{}, current};
}

InitialI::OpMSeq2MarshaledResult
InitialI::opMSeq2(optional<Test::StringSeq> p1, const Ice::Current& current)
{
    return {p1, p1, current};
}

InitialI::OpMDict1MarshaledResult
InitialI::opMDict1(const Ice::Current& current)
{
    return {Test::StringIntDict(), current};
}

InitialI::OpMDict2MarshaledResult
InitialI::opMDict2(optional<Test::StringIntDict> p1, const Ice::Current& current)
{
    return {p1, p1, current};
}

bool
InitialI::supportsJavaSerializable(const Ice::Current&)
{
    return true;
}
