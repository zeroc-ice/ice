// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

Optional<Byte> 
InitialI::opByte(const Optional<Byte>& p1, Optional<Byte>& p3, const Current&)
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

Optional<string> 
InitialI::opString(const Optional<string>& p1, Optional<string>& p3, const Current&)
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

Optional<ByteSeq> 
InitialI::opByteSeq(const Optional<pair<const Byte*, const Byte*> >& p1, Optional<ByteSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = ByteSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<ShortSeq> 
InitialI::opShortSeq(const Optional<pair<const Short*, const Short*> >& p1, Optional<ShortSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = ShortSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<BoolSeq> 
InitialI::opBoolSeq(const Optional<pair<const bool*, const bool*> >& p1, Optional<BoolSeq>& p3, const Current&)
{
    if(p1)
    {
        p3 = BoolSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<StringSeq> 
InitialI::opStringSeq(const Optional<pair<StringSeq::const_iterator, StringSeq::const_iterator> >& p1,
                      Optional<StringSeq>& p3, 
                      const Current&)
{
    if(p1)
    {
        p3 = StringSeq(p1->first, p1->second);
    }
    return p3;
}

Optional<FixedStructSeq> 
InitialI::opFixedStructSeq(const Optional<pair<const FixedStruct*, const FixedStruct*> >& p1,
                           Optional<FixedStructSeq>& p3, 
                           const Current&)
{
    if(p1)
    {
        p3 = FixedStructSeq(p1->first, p1->second);
    }
    return p3;
}
    
Optional<VarStructSeq> 
InitialI::opVarStructSeq(const Optional<pair<VarStructSeq::const_iterator, VarStructSeq::const_iterator> >& p1,
                         Optional<VarStructSeq>& p3, 
                         const Current&)
{
    if(p1)
    {
        p3 = VarStructSeq(p1->first, p1->second);
    }
    return p3;
}

void
InitialI::opClassAndUnknownOptional(const APtr& a, const Ice::Current&)
{
}

