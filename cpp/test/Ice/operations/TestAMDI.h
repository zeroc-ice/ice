// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    
    MyDerivedClassI();

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

    virtual void opByteBoolDS_async(const Test::AMD_MyClass_opByteBoolDSPtr&,
                                    const Test::ByteBoolDS&, const Test::ByteBoolDS&,
                                    const Ice::Current&);

    virtual void opShortIntDS_async(const Test::AMD_MyClass_opShortIntDSPtr&,
                                    const Test::ShortIntDS&, const Test::ShortIntDS&,
                                    const Ice::Current&);

    virtual void opLongFloatDS_async(const Test::AMD_MyClass_opLongFloatDSPtr&,
                                     const Test::LongFloatDS&, const Test::LongFloatDS&,
                                     const Ice::Current&);

    virtual void opStringStringDS_async(const Test::AMD_MyClass_opStringStringDSPtr&,
                                        const Test::StringStringDS&, const Test::StringStringDS&,
                                        const Ice::Current&);

    virtual void opStringMyEnumDS_async(const Test::AMD_MyClass_opStringMyEnumDSPtr&,
                                        const Test::StringMyEnumDS&, const Test::StringMyEnumDS&,
                                        const Ice::Current&);

    virtual void opMyEnumStringDS_async(const Test::AMD_MyClass_opMyEnumStringDSPtr&,
                                        const Test::MyEnumStringDS&, const Test::MyEnumStringDS&,
                                        const Ice::Current&);

    virtual void opMyStructMyEnumDS_async(const Test::AMD_MyClass_opMyStructMyEnumDSPtr&,
                                          const Test::MyStructMyEnumDS&, const Test::MyStructMyEnumDS&,
                                          const Ice::Current&);

    virtual void opByteByteSD_async(const Test::AMD_MyClass_opByteByteSDPtr&,
                                    const Test::ByteByteSD&, const Test::ByteByteSD&,
                                    const Ice::Current&);

    virtual void opBoolBoolSD_async(const Test::AMD_MyClass_opBoolBoolSDPtr&,
                                    const Test::BoolBoolSD&, const Test::BoolBoolSD&,
                                    const Ice::Current&);

    virtual void opShortShortSD_async(const Test::AMD_MyClass_opShortShortSDPtr&,
                                      const Test::ShortShortSD&, const Test::ShortShortSD&,
                                      const Ice::Current&);

    virtual void opIntIntSD_async(const Test::AMD_MyClass_opIntIntSDPtr&,
                                  const Test::IntIntSD&, const Test::IntIntSD&,
                                  const Ice::Current&);

    virtual void opLongLongSD_async(const Test::AMD_MyClass_opLongLongSDPtr&,
                                    const Test::LongLongSD&, const Test::LongLongSD&,
                                    const Ice::Current&);

    virtual void opStringFloatSD_async(const Test::AMD_MyClass_opStringFloatSDPtr&,
                                       const Test::StringFloatSD&, const Test::StringFloatSD&,
                                       const Ice::Current&);

    virtual void opStringDoubleSD_async(const Test::AMD_MyClass_opStringDoubleSDPtr&,
                                        const Test::StringDoubleSD&, const Test::StringDoubleSD&,
                                        const Ice::Current&);

    virtual void opStringStringSD_async(const Test::AMD_MyClass_opStringStringSDPtr&,
                                        const Test::StringStringSD&, const Test::StringStringSD&,
                                        const Ice::Current&);

    virtual void opMyEnumMyEnumSD_async(const Test::AMD_MyClass_opMyEnumMyEnumSDPtr&,
                                        const Test::MyEnumMyEnumSD&, const Test::MyEnumMyEnumSD&,
                                        const Ice::Current&);

    virtual void opIntS_async(const Test::AMD_MyClass_opIntSPtr&, const Test::IntS&, const Ice::Current&);

    virtual void opByteSOneway_async(const Test::AMD_MyClass_opByteSOnewayPtr&, const Test::ByteS&,
                                     const Ice::Current&);
    virtual void opByteSOnewayCallCount_async(const Test::AMD_MyClass_opByteSOnewayCallCountPtr&, const Ice::Current&);

    virtual void opContext_async(const Test::AMD_MyClass_opContextPtr&, const Ice::Current&);

    virtual void opDoubleMarshaling_async(const Test::AMD_MyClass_opDoubleMarshalingPtr&,
                                          Ice::Double, const Test::DoubleS&, const Ice::Current&);

    virtual void opIdempotent_async(const Test::AMD_MyClass_opIdempotentPtr&,
                                    const Ice::Current&);

    virtual void opNonmutating_async(const Test::AMD_MyClass_opNonmutatingPtr&,
                                     const Ice::Current&);

    virtual void opDerived_async(const Test::AMD_MyDerivedClass_opDerivedPtr&,
                                 const Ice::Current&);

    virtual void opByte1_async(const Test::AMD_MyClass_opByte1Ptr&,
                               Ice::Byte,
                               const Ice::Current&);

    virtual void opShort1_async(const Test::AMD_MyClass_opShort1Ptr&,
                                Ice::Short,
                                const Ice::Current&);

    virtual void opInt1_async(const Test::AMD_MyClass_opInt1Ptr&,
                              Ice::Int,
                              const Ice::Current&);

    virtual void opLong1_async(const Test::AMD_MyClass_opLong1Ptr&,
                               Ice::Long,
                               const Ice::Current&);

    virtual void opFloat1_async(const Test::AMD_MyClass_opFloat1Ptr&,
                                Ice::Float,
                                const Ice::Current&);

    virtual void opDouble1_async(const Test::AMD_MyClass_opDouble1Ptr&,
                                 Ice::Double,
                                 const Ice::Current&);

    virtual void opString1_async(const Test::AMD_MyClass_opString1Ptr&,
                                 const std::string&,
                                 const Ice::Current&);

    virtual void opStringS1_async(const Test::AMD_MyClass_opStringS1Ptr&,
                                  const Test::StringS&,
                                  const Ice::Current&);

    virtual void opByteBoolD1_async(const Test::AMD_MyClass_opByteBoolD1Ptr&,
                                    const Test::ByteBoolD&,
                                    const Ice::Current&);
    
    virtual void opStringS2_async(const Test::AMD_MyClass_opStringS2Ptr&,
                                  const Test::StringS&,
                                  const Ice::Current&);
    
    virtual void opByteBoolD2_async(const Test::AMD_MyClass_opByteBoolD2Ptr&,
                                    const Test::ByteBoolD&,
                                    const Ice::Current&);

    virtual void opMyStruct1_async(const Test::AMD_MyDerivedClass_opMyStruct1Ptr&, 
                                   const Test::MyStruct1&,
                                   const Ice::Current&);

    virtual void opMyClass1_async(const Test::AMD_MyDerivedClass_opMyClass1Ptr&,
                                  const Test::MyClass1Ptr&,
                                  const Ice::Current&);
    
    virtual void opStringLiterals_async(const Test::AMD_MyClass_opStringLiteralsPtr&,
                                       const Ice::Current&);
    
    virtual void opWStringLiterals_async(const Test::AMD_MyClass_opWStringLiteralsPtr&,
                                         const Ice::Current&);

private:
    IceUtil::ThreadPtr _opVoidThread;
    IceUtil::Mutex _opVoidMutex;

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

#endif
