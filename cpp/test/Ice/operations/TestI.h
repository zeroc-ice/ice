// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

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

    virtual void shutdown(const Ice::Current&);

    virtual void delay(Ice::Int, const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

    virtual Ice::Byte opByte(Ice::Byte,
                             Ice::Byte,
                             Ice::Byte&,
                             const Ice::Current&);

    virtual bool opBool(bool,
                        bool,
                        bool&,
                        const Ice::Current&);

    virtual Ice::Long opShortIntLong(Ice::Short,
                                     Ice::Int,
                                     Ice::Long,
                                     Ice::Short&,
                                     Ice::Int&,
                                     Ice::Long&,
                                     const Ice::Current&);

    virtual Ice::Double opFloatDouble(Ice::Float,
                                      Ice::Double,
                                      Ice::Float&,
                                      Ice::Double&,
                                      const Ice::Current&);

    virtual std::string opString(const std::string&,
                                 const std::string&,
                                 std::string&,
                                 const Ice::Current&);

    virtual Test::MyEnum opMyEnum(Test::MyEnum,
                                  Test::MyEnum&,
                                  const Ice::Current&);

    virtual Test::MyClassPrx opMyClass(const Test::MyClassPrx&,
                                       Test::MyClassPrx&, Test::MyClassPrx&,
                                       const Ice::Current&);

    virtual Test::Structure opStruct(const Test::Structure&, const Test::Structure&,
                                  Test::Structure&,
                                  const Ice::Current&);

    virtual Test::ByteS opByteS(const Test::ByteS&,
                                const Test::ByteS&,
                                Test::ByteS&,
                                const Ice::Current&);

    virtual Test::BoolS opBoolS(const Test::BoolS&,
                                const Test::BoolS&,
                                Test::BoolS&,
                                const Ice::Current&);

    virtual Test::LongS opShortIntLongS(const Test::ShortS&,
                                        const Test::IntS&,
                                        const Test::LongS&,
                                        Test::ShortS&,
                                        Test::IntS&,
                                        Test::LongS&,
                                        const Ice::Current&);

    virtual Test::DoubleS opFloatDoubleS(const Test::FloatS&,
                                         const Test::DoubleS&,
                                         Test::FloatS&,
                                         Test::DoubleS&,
                                         const Ice::Current&);

    virtual Test::StringS opStringS(const Test::StringS&,
                                    const Test::StringS&,
                                    Test::StringS&,
                                    const Ice::Current&);

    virtual Test::ByteSS opByteSS(const Test::ByteSS&,
                                  const Test::ByteSS&,
                                  Test::ByteSS&,
                                  const Ice::Current&);

    virtual Test::BoolSS opBoolSS(const Test::BoolSS&,
                                  const Test::BoolSS&,
                                  Test::BoolSS&,
                                  const Ice::Current&);

    virtual Test::LongSS opShortIntLongSS(const Test::ShortSS&,
                                          const Test::IntSS&,
                                          const Test::LongSS&,
                                          Test::ShortSS&,
                                          Test::IntSS&,
                                          Test::LongSS&,
                                          const Ice::Current&);

    virtual Test::DoubleSS opFloatDoubleSS(const Test::FloatSS&,
                                           const Test::DoubleSS&,
                                           Test::FloatSS&,
                                           Test::DoubleSS&,
                                           const Ice::Current&);

    virtual Test::StringSS opStringSS(const Test::StringSS&,
                                      const Test::StringSS&,
                                      Test::StringSS&,
                                      const Ice::Current&);

    virtual Test::StringSSS opStringSSS(const Test::StringSSS&,
                                        const Test::StringSSS&,
                                        Test::StringSSS&,
                                        const ::Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD(const Test::ByteBoolD&, const Test::ByteBoolD&,
                                        Test::ByteBoolD&,
                                        const Ice::Current&);

    virtual Test::ShortIntD opShortIntD(const Test::ShortIntD&, const Test::ShortIntD&,
                                        Test::ShortIntD&,
                                        const Ice::Current&);

    virtual Test::LongFloatD opLongFloatD(const Test::LongFloatD&, const Test::LongFloatD&,
                                          Test::LongFloatD&,
                                          const Ice::Current&);

    virtual Test::StringStringD opStringStringD(const Test::StringStringD&, const Test::StringStringD&,
                                                Test::StringStringD&,
                                                const Ice::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(const Test::StringMyEnumD&, const Test::StringMyEnumD&,
                                                Test::StringMyEnumD&,
                                                const Ice::Current&);

    virtual Test::MyEnumStringD opMyEnumStringD(const Test::MyEnumStringD&, const Test::MyEnumStringD&,
                                                Test::MyEnumStringD&,
                                                const Ice::Current&);

    virtual Test::MyStructMyEnumD opMyStructMyEnumD(const Test::MyStructMyEnumD&, const Test::MyStructMyEnumD&,
                                                    Test::MyStructMyEnumD&,
                                                    const Ice::Current&);

    virtual Test::ByteBoolDS opByteBoolDS(const Test::ByteBoolDS&, const Test::ByteBoolDS&,
                                          Test::ByteBoolDS&,
                                          const Ice::Current&);

    virtual Test::ShortIntDS opShortIntDS(const Test::ShortIntDS&, const Test::ShortIntDS&,
                                          Test::ShortIntDS&,
                                          const Ice::Current&);

    virtual Test::LongFloatDS opLongFloatDS(const Test::LongFloatDS&, const Test::LongFloatDS&,
                                            Test::LongFloatDS&,
                                            const Ice::Current&);

    virtual Test::StringStringDS opStringStringDS(const Test::StringStringDS&, const Test::StringStringDS&,
                                                  Test::StringStringDS&,
                                                  const Ice::Current&);

    virtual Test::StringMyEnumDS opStringMyEnumDS(const Test::StringMyEnumDS&, const Test::StringMyEnumDS&,
                                                  Test::StringMyEnumDS&,
                                                  const Ice::Current&);

    virtual Test::MyStructMyEnumDS opMyStructMyEnumDS(const Test::MyStructMyEnumDS&, const Test::MyStructMyEnumDS&,
                                                      Test::MyStructMyEnumDS&,
                                                      const Ice::Current&);

    virtual Test::MyEnumStringDS opMyEnumStringDS(const Test::MyEnumStringDS&, const Test::MyEnumStringDS&,
                                                  Test::MyEnumStringDS&,
                                                  const Ice::Current&);

    virtual Test::ByteByteSD opByteByteSD(const Test::ByteByteSD&, const Test::ByteByteSD&,
                                          Test::ByteByteSD&,
                                          const Ice::Current&);

    virtual Test::BoolBoolSD opBoolBoolSD(const Test::BoolBoolSD&, const Test::BoolBoolSD&,
                                          Test::BoolBoolSD&,
                                          const Ice::Current&);

    virtual Test::ShortShortSD opShortShortSD(const Test::ShortShortSD&, const Test::ShortShortSD&,
                                              Test::ShortShortSD&,
                                              const Ice::Current&);

    virtual Test::IntIntSD opIntIntSD(const Test::IntIntSD&, const Test::IntIntSD&,
                                      Test::IntIntSD&,
                                      const Ice::Current&);

    virtual Test::LongLongSD opLongLongSD(const Test::LongLongSD&, const Test::LongLongSD&,
                                          Test::LongLongSD&,
                                          const Ice::Current&);

    virtual Test::StringFloatSD opStringFloatSD(const Test::StringFloatSD&, const Test::StringFloatSD&,
                                                Test::StringFloatSD&,
                                                const Ice::Current&);

    virtual Test::StringDoubleSD opStringDoubleSD(const Test::StringDoubleSD&, const Test::StringDoubleSD&,
                                                  Test::StringDoubleSD&,
                                                  const Ice::Current&);

    virtual Test::StringStringSD opStringStringSD(const Test::StringStringSD&, const Test::StringStringSD&,
                                                  Test::StringStringSD&,
                                                  const Ice::Current&);

    virtual Test::MyEnumMyEnumSD opMyEnumMyEnumSD(const Test::MyEnumMyEnumSD&, const Test::MyEnumMyEnumSD&,
                                                  Test::MyEnumMyEnumSD&,
                                                  const Ice::Current&);

    virtual Test::IntS opIntS(const Test::IntS&, const Ice::Current&);

    virtual void opByteSOneway(const Test::ByteS&, const Ice::Current&);
    virtual int opByteSOnewayCallCount(const Ice::Current&);

    virtual Ice::Context opContext(const Ice::Current&);

    virtual void opDoubleMarshaling(Ice::Double, const Test::DoubleS&, const Ice::Current&);

    virtual void opIdempotent(const Ice::Current&);

    virtual void opNonmutating(const Ice::Current&);

    virtual void opDerived(const Ice::Current&);

    virtual Ice::Byte opByte1(Ice::Byte, const Ice::Current&);

    virtual Ice::Short opShort1(Ice::Short, const Ice::Current&);

    virtual Ice::Int opInt1(Ice::Int, const Ice::Current&);

    virtual Ice::Long opLong1(Ice::Long, const Ice::Current&);

    virtual Ice::Float opFloat1(Ice::Float, const Ice::Current&);

    virtual Ice::Double opDouble1(Ice::Double, const Ice::Current&);

    virtual std::string opString1(const std::string&, const Ice::Current&);

    virtual Test::StringS opStringS1(const Test::StringS&, const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD1(const Test::ByteBoolD&, const Ice::Current&);
    
    virtual Test::StringS opStringS2(const Test::StringS&, const Ice::Current&);
    
    virtual Test::ByteBoolD opByteBoolD2(const Test::ByteBoolD&, const Ice::Current&);
    
    virtual Test::MyStruct1 opMyStruct1(const Test::MyStruct1&, const Ice::Current&);

    virtual Test::MyClass1Ptr opMyClass1(const Test::MyClass1Ptr&, const Ice::Current&);
    
    virtual Test::StringS opStringLiterals(const Ice::Current&);
    
    virtual Test::WStringS opWStringLiterals(const Ice::Current&);

private:

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

#endif
