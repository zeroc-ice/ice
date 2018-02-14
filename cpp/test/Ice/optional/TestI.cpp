// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

ObjectPtr
InitialI::pingPong(const ObjectPtr& obj, const Current&)
{
    return obj;
}

void
InitialI::opOptionalException(const Optional<Int>& a,
                              const Optional<string>& b,
                              const Optional<OneOptionalPtr>& o,
                              const Ice::Current&)
{
    OptionalException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    throw ex;
}

void
InitialI::opDerivedException(const Optional<Int>& a,
                             const Optional<string>& b,
                             const Optional<OneOptionalPtr>& o,
                             const Ice::Current&)
{
    DerivedException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    ex.ss = b;
    ex.o2 = o;
    throw ex;
}

void
InitialI::opRequiredException(const Optional<Int>& a,
                              const Optional<string>& b,
                              const Optional<OneOptionalPtr>& o,
                              const Ice::Current&)
{
    RequiredException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    if(b)
    {
        ex.ss = b.get();
    }
    if(o)
    {
        ex.o2 = o.get();
    }
    throw ex;
}

Optional<Ice::Byte>
InitialI::opByte(const Optional<Ice::Byte>& p1, Optional<Ice::Byte>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<bool>
InitialI::opBool(const Optional<bool>& p1, Optional<bool>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Short>
InitialI::opShort(const Optional<Short>& p1, Optional<Short>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Int>
InitialI::opInt(const Optional<Int>& p1, Optional<Int>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Long>
InitialI::opLong(const Optional<Long>& p1, Optional<Long>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Float>
InitialI::opFloat(const Optional<Float>& p1, Optional<Float>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Double>
InitialI::opDouble(const Optional<Double>& p1, Optional<Double>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<string>
InitialI::opString(const Optional<string>& p1, Optional<string>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<string>
InitialI::opCustomString(const Optional<Util::string_view>& p1, Optional<string>& p3, const Current&)
{
    if(p1)
    {
        p3 = p1->to_string();
    }
    return p3;
}


Optional<MyEnum>
InitialI::opMyEnum(const Optional<MyEnum>& p1, Optional<MyEnum>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<SmallStruct>
InitialI::opSmallStruct(const Optional<SmallStruct>& p1, Optional<SmallStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<FixedStruct>
InitialI::opFixedStruct(const Optional<FixedStruct>& p1, Optional<FixedStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<VarStruct>
InitialI::opVarStruct(const Optional<VarStruct>& p1, Optional<VarStruct>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<OneOptionalPtr>
InitialI::opOneOptional(const Optional<OneOptionalPtr>& p1, Optional<OneOptionalPtr>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<OneOptionalPrx>
InitialI::opOneOptionalProxy(const Optional<OneOptionalPrx>& p1, Optional<OneOptionalPrx>& p3, const Current&)
{
    p3 = p1;
    return p1;
}

Optional<Test::ByteSeq>
InitialI::opByteSeq(const Optional<pair<const Ice::Byte*, const Ice::Byte*> >& p1, Optional<Test::ByteSeq>& p3,
                    const Current&)
{
    if(p1)
    {
        p3 = Ice::ByteSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::BoolSeq>
InitialI::opBoolSeq(const Optional<pair<const bool*, const bool*> >& p1, Optional<Test::BoolSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Ice::BoolSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::ShortSeq>
InitialI::opShortSeq(const Optional<pair<const Short*, const Short*> >& p1, Optional<Test::ShortSeq>& p3,
                     const Current&)
{
    if(p1)
    {
        p3 = Ice::ShortSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::IntSeq>
InitialI::opIntSeq(const Optional<pair<const Int*, const Int*> >& p1, Optional<Test::IntSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Test::IntSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::LongSeq>
InitialI::opLongSeq(const Optional<pair<const Long*, const Long*> >& p1, Optional<Test::LongSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Test::LongSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::FloatSeq>
InitialI::opFloatSeq(const Optional<pair<const Float*, const Float*> >& p1, Optional<Test::FloatSeq>& p3,
                     const Current&)
{
    if(p1)
    {
        p3 = Test::FloatSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Test::DoubleSeq>
InitialI::opDoubleSeq(const Optional<pair<const Double*, const Double*> >& p1, Optional<Test::DoubleSeq>& p3,
                      const Current&)
{
    if(p1)
    {
        p3 = Test::DoubleSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Ice::StringSeq>
InitialI::opStringSeq(const Optional<pair<Ice::StringSeq::const_iterator, Ice::StringSeq::const_iterator> >& p1,
                      Optional<Ice::StringSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = Ice::StringSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<SmallStructSeq>
InitialI::opSmallStructSeq(const Optional<pair<const SmallStruct*, const SmallStruct*> >& p1,
                           Optional<SmallStructSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = SmallStructSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<SmallStructList>
InitialI::opSmallStructList(const Optional<pair<const SmallStruct*, const SmallStruct*> >& p1,
                            Optional<SmallStructList>& p3, const Current&)
{
    if(p1)
    {
        p3 = SmallStructList(p1->first, p1->second);
    }
    return p3;
}

Optional<FixedStructSeq>
InitialI::opFixedStructSeq(const Optional<pair<const FixedStruct*, const FixedStruct*> >& p1,
                           Optional<FixedStructSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = FixedStructSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<FixedStructList>
InitialI::opFixedStructList(const Optional<pair<const FixedStruct*, const FixedStruct*> >& p1,
                            Optional<FixedStructList>& p3, const Current&)
{
    if(p1)
    {
        p3 = FixedStructList(p1->first, p1->second);
    }
    return p3;
}

Optional<VarStructSeq>
InitialI::opVarStructSeq(const Optional<pair<VarStructSeq::const_iterator, VarStructSeq::const_iterator> >& p1,
                         Optional<VarStructSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = VarStructSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<Serializable>
InitialI::opSerializable(const Optional<Serializable>& p1, Optional<Serializable>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

Optional<IntIntDict>
InitialI::opIntIntDict(const Optional<IntIntDict>& p1, Optional<IntIntDict>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

Optional<StringIntDict>
InitialI::opStringIntDict(const Optional<StringIntDict>& p1, Optional<StringIntDict>& p3, const Current&)
{
    p3 = p1;
    return p3;
}

Optional<IntStringDict>
InitialI::opCustomIntStringDict(const Optional<std::map<int, Util::string_view> >& p1,
                                Optional<IntStringDict>& p3, const Current&)
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
InitialI::opClassAndUnknownOptional(const APtr&, const Ice::Current&)
{
}

void
InitialI::sendOptionalClass(bool, const Optional<OneOptionalPtr>&, const Ice::Current&)
{
}

void
InitialI::returnOptionalClass(bool, Optional<OneOptionalPtr>& o, const Ice::Current&)
{
    o = new OneOptional(53);
}

GPtr
InitialI::opG(const GPtr& g, const Ice::Current&)
{
    return g;
}

void
InitialI::opVoid(const Ice::Current&)
{
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
