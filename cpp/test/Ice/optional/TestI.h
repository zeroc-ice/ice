// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>


class InitialI : public Test::Initial
{
public:

    InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual Ice::ObjectPtr pingPong(const Ice::ObjectPtr&, const Ice::Current&);

    virtual void opOptionalException(const IceUtil::Optional< ::Ice::Int>&, 
                                     const IceUtil::Optional< ::std::string>&,
                                     const IceUtil::Optional<Test::OneOptionalPtr>&,
                                     const Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Byte> opByte(const IceUtil::Optional< ::Ice::Byte>&, 
                                                   IceUtil::Optional< ::Ice::Byte>&, 
                                                   const ::Ice::Current&);
    
    virtual IceUtil::Optional< ::Ice::Long> opLong(const IceUtil::Optional< ::Ice::Long>&, 
                                                   IceUtil::Optional< ::Ice::Long>&, 
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opString(const IceUtil::Optional< ::std::string>&, 
                                                       IceUtil::Optional< ::std::string>&, 
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPtr> opOneOptional(const IceUtil::Optional< Test::OneOptionalPtr>&, 
                                                                  IceUtil::Optional< Test::OneOptionalPtr>&, 
                                                                  const ::Ice::Current&);
    
    virtual IceUtil::Optional<Test::OneOptionalPrx> opOneOptionalProxy(const IceUtil::Optional< Test::OneOptionalPrx>&, 
                                                                       IceUtil::Optional< Test::OneOptionalPrx>&, 
                                                                       const ::Ice::Current&);
    
    virtual IceUtil::Optional< ::Test::ByteSeq> opByteSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >&, 
        IceUtil::Optional< ::Test::ByteSeq>&, 
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::ShortSeq> opShortSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >&, 
        IceUtil::Optional< ::Test::ShortSeq>&, 
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::BoolSeq> opBoolSeq(
        const IceUtil::Optional< ::std::pair<const bool*, const bool*> >&, 
        IceUtil::Optional< ::Test::BoolSeq>&, 
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::StringSeq> opStringSeq(
        const IceUtil::Optional< ::std::pair< ::Test::StringSeq::const_iterator, 
                                              ::Test::StringSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::StringSeq>&, 
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::FixedStructSeq> opFixedStructSeq(
        const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >&,
        IceUtil::Optional< ::Test::FixedStructSeq>&, const ::Ice::Current& = ::Ice::Current());
    
    virtual IceUtil::Optional< ::Test::VarStructSeq> opVarStructSeq(
        const IceUtil::Optional< ::std::pair< ::Test::VarStructSeq::const_iterator, 
                                              ::Test::VarStructSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::VarStructSeq>&, 
        const ::Ice::Current& = ::Ice::Current());

    virtual void opClassAndUnknownOptional(const Test::APtr&, const Ice::Current&);
};

#endif
