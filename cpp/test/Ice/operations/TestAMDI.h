// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include <IceUtil/Thread.h>
#include <TestAMD.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //
    virtual bool ice_isA(const std::string&, const Ice::Current&) const;
    virtual void ice_ping(const Ice::Current&) const;
    virtual std::vector<std::string> ice_ids(const Ice::Current&) const;
    virtual const std::string& ice_id(const Ice::Current&) const;

    virtual void shutdown_async(const Test::AMD_MyClass_shutdownPtr&,
                                const Ice::Current&);

    virtual void delay_async(const Test::AMD_MyClass_delayPtr&, Ice::Int,
                             const Ice::Current&);

    virtual void opVoid_async(const Test::AMD_MyClass_opVoidPtr&,
                              const Ice::Current&);

    virtual void opByte_async(const Test::AMD_MyClass_opBytePtr&,
                              Ice::Byte, Ice::Byte,
                              const Ice::Current&);

    virtual void opBool_async(const Test::AMD_MyClass_opBoolPtr&,
                              bool, bool,
                              const Ice::Current&);

    virtual void opShortIntLong_async(const Test::AMD_MyClass_opShortIntLongPtr&,
                                      Ice::Short, Ice::Int, Ice::Long,
                                      const Ice::Current&);

    virtual void opFloatDouble_async(const Test::AMD_MyClass_opFloatDoublePtr&,
                                     Ice::Float, Ice::Double,
                                     const Ice::Current&);

    virtual void opString_async(const Test::AMD_MyClass_opStringPtr&,
                                const std::string&, const std::string&,
                                const Ice::Current&);

    virtual void opMyEnum_async(const Test::AMD_MyClass_opMyEnumPtr&,
                                Test::MyEnum,
                                const Ice::Current&);

    virtual void opMyClass_async(const Test::AMD_MyClass_opMyClassPtr&,
                                 const Test::MyClassPrx&,
                                 const Ice::Current&);

    virtual void opStruct_async(const Test::AMD_MyClass_opStructPtr&,
                                const Test::Structure&, const Test::Structure&,
                                const Ice::Current&);

    virtual void opByteS_async(const Test::AMD_MyClass_opByteSPtr&,
                               const Test::ByteS&, const Test::ByteS&,
                               const Ice::Current&);

    virtual void opBoolS_async(const Test::AMD_MyClass_opBoolSPtr&,
                               const Test::BoolS&, const Test::BoolS&,
                               const Ice::Current&);

    virtual void opShortIntLongS_async(const Test::AMD_MyClass_opShortIntLongSPtr&,
                                       const Test::ShortS&, const Test::IntS&, const Test::LongS&,
                                       const Ice::Current&);

    virtual void opFloatDoubleS_async(const Test::AMD_MyClass_opFloatDoubleSPtr&,
                                      const Test::FloatS&, const Test::DoubleS&,
                                      const Ice::Current&);

    virtual void opStringS_async(const Test::AMD_MyClass_opStringSPtr&,
                                 const Test::StringS&, const Test::StringS&,
                                 const Ice::Current&);

    virtual void opByteSS_async(const Test::AMD_MyClass_opByteSSPtr&,
                                const Test::ByteSS&, const Test::ByteSS&,
                                const Ice::Current&);

    virtual void opBoolSS_async(const Test::AMD_MyClass_opBoolSSPtr&,
                                const Test::BoolSS&, const Test::BoolSS&,
                                const Ice::Current&);

    virtual void opShortIntLongSS_async(const Test::AMD_MyClass_opShortIntLongSSPtr&,
                                        const Test::ShortSS&, const Test::IntSS&, const Test::LongSS&,
                                        const Ice::Current&);

    virtual void opFloatDoubleSS_async(const Test::AMD_MyClass_opFloatDoubleSSPtr&,
                                       const Test::FloatSS&, const Test::DoubleSS&,
                                       const Ice::Current&);

    virtual void opStringSS_async(const Test::AMD_MyClass_opStringSSPtr&,
                                  const Test::StringSS&, const Test::StringSS&,
                                  const Ice::Current&);

    virtual void opStringSSS_async(const Test::AMD_MyClass_opStringSSSPtr&,
                                   const Test::StringSSS&, const Test::StringSSS&,
                                   const Ice::Current&);

    virtual void opByteBoolD_async(const Test::AMD_MyClass_opByteBoolDPtr&,
                                   const Test::ByteBoolD&, const Test::ByteBoolD&,
                                   const Ice::Current&);

    virtual void opShortIntD_async(const Test::AMD_MyClass_opShortIntDPtr&,
                                   const Test::ShortIntD&, const Test::ShortIntD&,
                                   const Ice::Current&);

    virtual void opLongFloatD_async(const Test::AMD_MyClass_opLongFloatDPtr&,
                                    const Test::LongFloatD&, const Test::LongFloatD&,
                                    const Ice::Current&);

    virtual void opStringStringD_async(const Test::AMD_MyClass_opStringStringDPtr&,
                                       const Test::StringStringD&, const Test::StringStringD&,
                                       const Ice::Current&);

    virtual void opStringMyEnumD_async(const Test::AMD_MyClass_opStringMyEnumDPtr&,
                                       const Test::StringMyEnumD&, const Test::StringMyEnumD&,
                                       const Ice::Current&);

    virtual void opMyEnumStringD_async(const Test::AMD_MyClass_opMyEnumStringDPtr&,
                                       const Test::MyEnumStringD&, const Test::MyEnumStringD&,
                                       const Ice::Current&);

    virtual void opMyStructMyEnumD_async(const Test::AMD_MyClass_opMyStructMyEnumDPtr&,
                                         const Test::MyStructMyEnumD&, const Test::MyStructMyEnumD&,
                                         const Ice::Current&);

    virtual void opIntS_async(const Test::AMD_MyClass_opIntSPtr&, const Test::IntS&, const Ice::Current&);

    virtual void opByteSOneway_async(const Test::AMD_MyClass_opByteSOnewayPtr&, const Test::ByteS&,
                                     const Ice::Current&);

    virtual void opContext_async(const Test::AMD_MyClass_opContextPtr&, const Ice::Current&);

    virtual void opDoubleMarshaling_async(const Test::AMD_MyClass_opDoubleMarshalingPtr&,
                                          Ice::Double, const Test::DoubleS&, const Ice::Current&);

    virtual void opIdempotent_async(const Test::AMD_MyClass_opIdempotentPtr&,
                                    const Ice::Current&);

    virtual void opNonmutating_async(const Test::AMD_MyClass_opNonmutatingPtr&,
                                     const Ice::Current&);

    virtual void opDerived_async(const Test::AMD_MyDerivedClass_opDerivedPtr&,
                                 const Ice::Current&);

private:

    IceUtil::ThreadPtr _opVoidThread;
    IceUtil::Mutex _opVoidMutex;
};

#endif
