//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;
using namespace IceUtil;
using namespace Ice;
using namespace std;

InitialI::InitialI()
{
}

void
InitialI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

Test::Initial::PingPongMarshaledResult
InitialI::pingPong(shared_ptr<Value> obj, const Current& current)
{
    auto result = PingPongMarshaledResult(obj, current);
    if(dynamic_pointer_cast<MultiOptional>(obj))
    {
        // Break cyclic reference count
        dynamic_pointer_cast<MultiOptional>(obj)->k = shared_ptr<MultiOptional>();
    }
    return result;
}

void
InitialI::opOptionalException(optional<Int> a,
                              optional<string> b,
                              optional<OneOptionalPtr> o,
                              const Ice::Current&)
{
    OptionalException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    throw ex;
}

void
InitialI::opDerivedException(optional<Int> a,
                             optional<string> b,
                             optional<OneOptionalPtr> o,
                             const Ice::Current&)
{
    DerivedException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    ex.ss = b;
    ex.o2 = o;
    ex.d1 = "d1";
    ex.d2 = "d2";
    throw ex;
}

void
InitialI::opRequiredException(optional<Int> a,
                              optional<string> b,
                              optional<OneOptionalPtr> o,
                              const Ice::Current&)
{
    RequiredException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    if(b)
    {
        ex.ss = b.value();
    }
    if(o)
    {
        ex.o2 = o.value();
    }
    throw ex;
}

optional<Ice::Byte>
InitialI::opByte(optional<Ice::Byte> p1, optional<Ice::Byte>& p3, const Current&)
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

optional<Short>
InitialI::opShort(optional<Short> p1, optional<Short>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Int>
InitialI::opInt(optional<Int> p1, optional<Int>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Long>
InitialI::opLong(optional<Long> p1, optional<Long>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Float>
InitialI::opFloat(optional<Float> p1, optional<Float>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Double>
InitialI::opDouble(optional<Double> p1, optional<Double>& p3, const Current&)
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

optional<string>
InitialI::opCustomString(optional<Util::string_view> p1, optional<string>& p3, const Current&)
{
    if(p1)
    {
        p3 = p1->to_string();
    }
    return p3;
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

optional<OneOptionalPtr>
InitialI::opOneOptional(optional<OneOptionalPtr> p1, optional<OneOptionalPtr>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<MyInterfacePrxPtr>
InitialI::opMyInterfaceProxy(optional<MyInterfacePrxPtr> p1, optional<MyInterfacePrxPtr>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

optional<Test::ByteSeq>
InitialI::opByteSeq(optional<pair<const Ice::Byte*, const Ice::Byte*> > p1, optional<Test::ByteSeq>& p3,
                    const Current&)
{
    if(p1)
    {
        p3 = Ice::ByteSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::BoolSeq>
InitialI::opBoolSeq(optional<pair<const bool*, const bool*> > p1, optional<Test::BoolSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Ice::BoolSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::ShortSeq>
InitialI::opShortSeq(optional<pair<const Short*, const Short*> > p1, optional<Test::ShortSeq>& p3,
                     const Current&)
{
    if(p1)
    {
        p3 = Ice::ShortSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::IntSeq>
InitialI::opIntSeq(optional<pair<const Int*, const Int*> > p1, optional<Test::IntSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Test::IntSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::LongSeq>
InitialI::opLongSeq(optional<pair<const Long*, const Long*> > p1, optional<Test::LongSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Test::LongSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::FloatSeq>
InitialI::opFloatSeq(optional<pair<const Float*, const Float*> > p1, optional<Test::FloatSeq>& p3,
                     const Current&)
{
    if(p1)
    {
        p3 = Test::FloatSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Test::DoubleSeq>
InitialI::opDoubleSeq(optional<pair<const Double*, const Double*> > p1, optional<Test::DoubleSeq>& p3,
                      const Current&)
{
    if(p1)
    {
        p3 = Test::DoubleSeq(p1->first, p1->second);
    }
    return p3;
}

optional<Ice::StringSeq>
InitialI::opStringSeq(optional<Ice::StringSeq> p1,
                      optional<Ice::StringSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = p1;
    }
    return p3;
}

optional<SmallStructSeq>
InitialI::opSmallStructSeq(optional<pair<const SmallStruct*, const SmallStruct*> > p1,
                           optional<SmallStructSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = SmallStructSeq(p1->first, p1->second);
    }
    return p3;
}

optional<SmallStructList>
InitialI::opSmallStructList(optional<pair<const SmallStruct*, const SmallStruct*> > p1,
                            optional<SmallStructList>& p3, const Current&)
{
    if(p1)
    {
        p3 = SmallStructList(p1->first, p1->second);
    }
    return p3;
}

optional<FixedStructSeq>
InitialI::opFixedStructSeq(optional<pair<const FixedStruct*, const FixedStruct*> > p1,
                           optional<FixedStructSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = FixedStructSeq(p1->first, p1->second);
    }
    return p3;
}

optional<FixedStructList>
InitialI::opFixedStructList(optional<pair<const FixedStruct*, const FixedStruct*> > p1,
                            optional<FixedStructList>& p3, const Current&)
{
    if(p1)
    {
        p3 = FixedStructList(p1->first, p1->second);
    }
    return p3;
}

optional<VarStructSeq>
InitialI::opVarStructSeq(optional<VarStructSeq> p1,
                         optional<VarStructSeq>& p3, const Current&)
{
    if(p1)
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

optional<IntOneOptionalDict>
InitialI::opIntOneOptionalDict(optional<IntOneOptionalDict> p1, optional<IntOneOptionalDict>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

optional<IntStringDict>
InitialI::opCustomIntStringDict(optional<std::map<int, Util::string_view> > p1,
                                optional<IntStringDict>& p3, const Current&)
{
    if(p1)
    {
        p3 = IntStringDict();
        for(std::map<int, Util::string_view>::const_iterator p = p1->begin();
            p != p1->end(); ++p)
        {
            (*p3)[p->first] = p->second.to_string();
        }
    }
    return p3;
}

void
InitialI::opClassAndUnknownOptional(APtr, const Ice::Current&)
{
}

void
InitialI::sendOptionalClass(bool, optional<OneOptionalPtr>, const Ice::Current&)
{
}

void
InitialI::returnOptionalClass(bool, optional<OneOptionalPtr>& o, const Ice::Current&)
{
    o = make_shared<OneOptional>(53);
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
    return OpMStruct1MarshaledResult(Test::SmallStruct(), current);
}

InitialI::OpMStruct2MarshaledResult
InitialI::opMStruct2(optional<Test::SmallStruct> p1, const Ice::Current& current)
{
    return OpMStruct2MarshaledResult(p1, p1, current);
}

InitialI::OpMSeq1MarshaledResult
InitialI::opMSeq1(const Ice::Current& current)
{
    return OpMSeq1MarshaledResult(Test::StringSeq(), current);
}

InitialI::OpMSeq2MarshaledResult
InitialI::opMSeq2(optional<Test::StringSeq> p1, const Ice::Current& current)
{
    return OpMSeq2MarshaledResult(p1, p1, current);
}

InitialI::OpMDict1MarshaledResult
InitialI::opMDict1(const Ice::Current& current)
{
    return OpMDict1MarshaledResult(Test::StringIntDict(), current);
}

InitialI::OpMDict2MarshaledResult
InitialI::opMDict2(optional<Test::StringIntDict> p1, const Ice::Current& current)
{
    return OpMDict2MarshaledResult(p1, p1, current);
}

InitialI::OpMG1MarshaledResult
InitialI::opMG1(const Ice::Current& current)
{
    return OpMG1MarshaledResult(make_shared<G>(), current);
}

InitialI::OpMG2MarshaledResult
InitialI::opMG2(optional<Test::GPtr> p1, const Ice::Current& current)
{
    return OpMG2MarshaledResult(p1, p1, current);
}

bool
InitialI::supportsRequiredParams(const Ice::Current&)
{
    return false;
}

bool
InitialI::supportsJavaSerializable(const Ice::Current&)
{
    return true;
}

bool
InitialI::supportsCsharpSerializable(const Ice::Current&)
{
    return true;
}

bool
InitialI::supportsCppStringView(const Ice::Current&)
{
    return true;
}

bool
InitialI::supportsNullOptional(const Ice::Current&)
{
    return true;
}
