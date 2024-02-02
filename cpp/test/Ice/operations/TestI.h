//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    virtual bool ice_isA(std::string, const Ice::Current&) const;
    virtual void ice_ping(const Ice::Current&) const;
    virtual std::vector<std::string> ice_ids(const Ice::Current&) const;
    virtual std::string ice_id(const Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

    virtual bool supportsCompress(const Ice::Current&);

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

    virtual std::string opString(std::string,
                                 std::string,
                                 std::string&,
                                 const Ice::Current&);

    virtual Test::MyEnum opMyEnum(Test::MyEnum,
                                  Test::MyEnum&,
                                  const Ice::Current&);

    virtual Test::MyClassPrxPtr opMyClass(Test::MyClassPrxPtr,
                                          Test::MyClassPrxPtr&, Test::MyClassPrxPtr&,
                                          const Ice::Current&);

    virtual Test::Structure opStruct(Test::Structure,
                                     Test::Structure,
                                     Test::Structure&,
                                     const Ice::Current&);

    virtual Test::ByteS opByteS(Test::ByteS,
                                Test::ByteS,
                                Test::ByteS&,
                                const Ice::Current&);

    virtual Test::BoolS opBoolS(Test::BoolS,
                                Test::BoolS,
                                Test::BoolS&,
                                const Ice::Current&);

    virtual Test::LongS opShortIntLongS(Test::ShortS,
                                        Test::IntS,
                                        Test::LongS,
                                        Test::ShortS&,
                                        Test::IntS&,
                                        Test::LongS&,
                                        const Ice::Current&);

    virtual Test::DoubleS opFloatDoubleS(Test::FloatS,
                                         Test::DoubleS,
                                         Test::FloatS&,
                                         Test::DoubleS&,
                                         const Ice::Current&);

    virtual Test::StringS opStringS(Test::StringS,
                                    Test::StringS,
                                    Test::StringS&,
                                    const Ice::Current&);

    virtual Test::ByteSS opByteSS(Test::ByteSS,
                                  Test::ByteSS,
                                  Test::ByteSS&,
                                  const Ice::Current&);

    virtual Test::BoolSS opBoolSS(Test::BoolSS,
                                  Test::BoolSS,
                                  Test::BoolSS&,
                                  const Ice::Current&);

    virtual Test::LongSS opShortIntLongSS(Test::ShortSS,
                                          Test::IntSS,
                                          Test::LongSS,
                                          Test::ShortSS&,
                                          Test::IntSS&,
                                          Test::LongSS&,
                                          const Ice::Current&);

    virtual Test::DoubleSS opFloatDoubleSS(Test::FloatSS,
                                           Test::DoubleSS,
                                           Test::FloatSS&,
                                           Test::DoubleSS&,
                                           const Ice::Current&);

    virtual Test::StringSS opStringSS(Test::StringSS,
                                      Test::StringSS,
                                      Test::StringSS&,
                                      const Ice::Current&);

    virtual Test::StringSSS opStringSSS(Test::StringSSS,
                                        Test::StringSSS,
                                        Test::StringSSS&,
                                        const ::Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD(Test::ByteBoolD,
                                        Test::ByteBoolD,
                                        Test::ByteBoolD&,
                                        const Ice::Current&);

    virtual Test::ShortIntD opShortIntD(Test::ShortIntD,
                                        Test::ShortIntD,
                                        Test::ShortIntD&,
                                        const Ice::Current&);

    virtual Test::LongFloatD opLongFloatD(Test::LongFloatD,
                                          Test::LongFloatD,
                                          Test::LongFloatD&,
                                          const Ice::Current&);

    virtual Test::StringStringD opStringStringD(Test::StringStringD,
                                                Test::StringStringD,
                                                Test::StringStringD&,
                                                const Ice::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(Test::StringMyEnumD,
                                                Test::StringMyEnumD,
                                                Test::StringMyEnumD&,
                                                const Ice::Current&);

    virtual Test::MyEnumStringD opMyEnumStringD(Test::MyEnumStringD,
                                                Test::MyEnumStringD,
                                                Test::MyEnumStringD&,
                                                const Ice::Current&);

    virtual Test::MyStructMyEnumD opMyStructMyEnumD(Test::MyStructMyEnumD,
                                                    Test::MyStructMyEnumD,
                                                    Test::MyStructMyEnumD&,
                                                    const Ice::Current&);

    virtual Test::ByteBoolDS opByteBoolDS(Test::ByteBoolDS,
                                          Test::ByteBoolDS,
                                          Test::ByteBoolDS&,
                                          const Ice::Current&);

    virtual Test::ShortIntDS opShortIntDS(Test::ShortIntDS,
                                          Test::ShortIntDS,
                                          Test::ShortIntDS&,
                                          const Ice::Current&);

    virtual Test::LongFloatDS opLongFloatDS(Test::LongFloatDS,
                                            Test::LongFloatDS,
                                            Test::LongFloatDS&,
                                            const Ice::Current&);

    virtual Test::StringStringDS opStringStringDS(Test::StringStringDS,
                                                  Test::StringStringDS,
                                                  Test::StringStringDS&,
                                                  const Ice::Current&);

    virtual Test::StringMyEnumDS opStringMyEnumDS(Test::StringMyEnumDS,
                                                  Test::StringMyEnumDS,
                                                  Test::StringMyEnumDS&,
                                                  const Ice::Current&);

    virtual Test::MyStructMyEnumDS opMyStructMyEnumDS(Test::MyStructMyEnumDS,
                                                      Test::MyStructMyEnumDS,
                                                      Test::MyStructMyEnumDS&,
                                                      const Ice::Current&);

    virtual Test::MyEnumStringDS opMyEnumStringDS(Test::MyEnumStringDS,
                                                  Test::MyEnumStringDS,
                                                  Test::MyEnumStringDS&,
                                                  const Ice::Current&);

    virtual Test::ByteByteSD opByteByteSD(Test::ByteByteSD,
                                          Test::ByteByteSD,
                                          Test::ByteByteSD&,
                                          const Ice::Current&);

    virtual Test::BoolBoolSD opBoolBoolSD(Test::BoolBoolSD,
                                          Test::BoolBoolSD,
                                          Test::BoolBoolSD&,
                                          const Ice::Current&);

    virtual Test::ShortShortSD opShortShortSD(Test::ShortShortSD,
                                              Test::ShortShortSD,
                                              Test::ShortShortSD&,
                                              const Ice::Current&);

    virtual Test::IntIntSD opIntIntSD(Test::IntIntSD,
                                      Test::IntIntSD,
                                      Test::IntIntSD&,
                                      const Ice::Current&);

    virtual Test::LongLongSD opLongLongSD(Test::LongLongSD,
                                          Test::LongLongSD,
                                          Test::LongLongSD&,
                                          const Ice::Current&);

    virtual Test::StringFloatSD opStringFloatSD(Test::StringFloatSD,
                                                Test::StringFloatSD,
                                                Test::StringFloatSD&,
                                                const Ice::Current&);

    virtual Test::StringDoubleSD opStringDoubleSD(Test::StringDoubleSD,
                                                  Test::StringDoubleSD,
                                                  Test::StringDoubleSD&,
                                                  const Ice::Current&);

    virtual Test::StringStringSD opStringStringSD(Test::StringStringSD,
                                                  Test::StringStringSD,
                                                  Test::StringStringSD&,
                                                  const Ice::Current&);

    virtual Test::MyEnumMyEnumSD opMyEnumMyEnumSD(Test::MyEnumMyEnumSD,
                                                  Test::MyEnumMyEnumSD,
                                                  Test::MyEnumMyEnumSD&,
                                                  const Ice::Current&);

    virtual Test::IntS opIntS(Test::IntS, const Ice::Current&);

    virtual void opByteSOneway(Test::ByteS, const Ice::Current&);
    virtual int opByteSOnewayCallCount(const Ice::Current&);

    virtual Ice::Context opContext(const Ice::Current&);

    virtual void opDoubleMarshaling(Ice::Double, Test::DoubleS, const Ice::Current&);

    virtual void opIdempotent(const Ice::Current&);

    virtual void opNonmutating(const Ice::Current&);

    virtual void opDerived(const Ice::Current&);

    virtual Ice::Byte opByte1(Ice::Byte, const Ice::Current&);

    virtual Ice::Short opShort1(Ice::Short, const Ice::Current&);

    virtual Ice::Int opInt1(Ice::Int, const Ice::Current&);

    virtual Ice::Long opLong1(Ice::Long, const Ice::Current&);

    virtual Ice::Float opFloat1(Ice::Float, const Ice::Current&);

    virtual Ice::Double opDouble1(Ice::Double, const Ice::Current&);

    virtual std::string opString1(std::string, const Ice::Current&);

    virtual Test::StringS opStringS1(Test::StringS, const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD1(Test::ByteBoolD, const Ice::Current&);

    virtual Test::StringS opStringS2(Test::StringS, const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD2(Test::ByteBoolD, const Ice::Current&);

    virtual Test::MyStruct1 opMyStruct1(Test::MyStruct1, const Ice::Current&);

    virtual Test::MyClass1Ptr opMyClass1(Test::MyClass1Ptr, const Ice::Current&);

    virtual Test::StringS opStringLiterals(const Ice::Current&);

    virtual Test::WStringS opWStringLiterals(const Ice::Current&);

    virtual OpMStruct1MarshaledResult opMStruct1(const Ice::Current&);

    virtual OpMStruct2MarshaledResult opMStruct2(Test::Structure, const Ice::Current&);

    virtual OpMSeq1MarshaledResult opMSeq1(const Ice::Current&);

    virtual OpMSeq2MarshaledResult opMSeq2(Test::StringS, const Ice::Current&);

    virtual OpMDict1MarshaledResult opMDict1(const Ice::Current&);

    virtual OpMDict2MarshaledResult opMDict2(Test::StringStringD, const Ice::Current&);

private:

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

#endif
