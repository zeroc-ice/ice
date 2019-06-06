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
    virtual bool ice_isA(ICE_IN(std::string), const Ice::Current&) const;
    virtual void ice_ping(const Ice::Current&) const;
    virtual std::vector<std::string> ice_ids(const Ice::Current&) const;
#ifdef ICE_CPP11_MAPPING
    virtual std::string ice_id(const Ice::Current&) const;
#else
    virtual const std::string& ice_id(const Ice::Current&) const;
#endif

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

    virtual std::string opString(ICE_IN(std::string),
                                 ICE_IN(std::string),
                                 std::string&,
                                 const Ice::Current&);

    virtual Test::MyEnum opMyEnum(Test::MyEnum,
                                  Test::MyEnum&,
                                  const Ice::Current&);

    virtual Test::MyClassPrxPtr opMyClass(ICE_IN(Test::MyClassPrxPtr),
                                          Test::MyClassPrxPtr&, Test::MyClassPrxPtr&,
                                          const Ice::Current&);

    virtual Test::Structure opStruct(ICE_IN(Test::Structure),
                                     ICE_IN(Test::Structure),
                                     Test::Structure&,
                                     const Ice::Current&);

    virtual Test::ByteS opByteS(ICE_IN(Test::ByteS),
                                ICE_IN(Test::ByteS),
                                Test::ByteS&,
                                const Ice::Current&);

    virtual Test::BoolS opBoolS(ICE_IN(Test::BoolS),
                                ICE_IN(Test::BoolS),
                                Test::BoolS&,
                                const Ice::Current&);

    virtual Test::LongS opShortIntLongS(ICE_IN(Test::ShortS),
                                        ICE_IN(Test::IntS),
                                        ICE_IN(Test::LongS),
                                        Test::ShortS&,
                                        Test::IntS&,
                                        Test::LongS&,
                                        const Ice::Current&);

    virtual Test::DoubleS opFloatDoubleS(ICE_IN(Test::FloatS),
                                         ICE_IN(Test::DoubleS),
                                         Test::FloatS&,
                                         Test::DoubleS&,
                                         const Ice::Current&);

    virtual Test::StringS opStringS(ICE_IN(Test::StringS),
                                    ICE_IN(Test::StringS),
                                    Test::StringS&,
                                    const Ice::Current&);

    virtual Test::ByteSS opByteSS(ICE_IN(Test::ByteSS),
                                  ICE_IN(Test::ByteSS),
                                  Test::ByteSS&,
                                  const Ice::Current&);

    virtual Test::BoolSS opBoolSS(ICE_IN(Test::BoolSS),
                                  ICE_IN(Test::BoolSS),
                                  Test::BoolSS&,
                                  const Ice::Current&);

    virtual Test::LongSS opShortIntLongSS(ICE_IN(Test::ShortSS),
                                          ICE_IN(Test::IntSS),
                                          ICE_IN(Test::LongSS),
                                          Test::ShortSS&,
                                          Test::IntSS&,
                                          Test::LongSS&,
                                          const Ice::Current&);

    virtual Test::DoubleSS opFloatDoubleSS(ICE_IN(Test::FloatSS),
                                           ICE_IN(Test::DoubleSS),
                                           Test::FloatSS&,
                                           Test::DoubleSS&,
                                           const Ice::Current&);

    virtual Test::StringSS opStringSS(ICE_IN(Test::StringSS),
                                      ICE_IN(Test::StringSS),
                                      Test::StringSS&,
                                      const Ice::Current&);

    virtual Test::StringSSS opStringSSS(ICE_IN(Test::StringSSS),
                                        ICE_IN(Test::StringSSS),
                                        Test::StringSSS&,
                                        const ::Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD(ICE_IN(Test::ByteBoolD),
                                        ICE_IN(Test::ByteBoolD),
                                        Test::ByteBoolD&,
                                        const Ice::Current&);

    virtual Test::ShortIntD opShortIntD(ICE_IN(Test::ShortIntD),
                                        ICE_IN(Test::ShortIntD),
                                        Test::ShortIntD&,
                                        const Ice::Current&);

    virtual Test::LongFloatD opLongFloatD(ICE_IN(Test::LongFloatD),
                                          ICE_IN(Test::LongFloatD),
                                          Test::LongFloatD&,
                                          const Ice::Current&);

    virtual Test::StringStringD opStringStringD(ICE_IN(Test::StringStringD),
                                                ICE_IN(Test::StringStringD),
                                                Test::StringStringD&,
                                                const Ice::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(ICE_IN(Test::StringMyEnumD),
                                                ICE_IN(Test::StringMyEnumD),
                                                Test::StringMyEnumD&,
                                                const Ice::Current&);

    virtual Test::MyEnumStringD opMyEnumStringD(ICE_IN(Test::MyEnumStringD),
                                                ICE_IN(Test::MyEnumStringD),
                                                Test::MyEnumStringD&,
                                                const Ice::Current&);

    virtual Test::MyStructMyEnumD opMyStructMyEnumD(ICE_IN(Test::MyStructMyEnumD),
                                                    ICE_IN(Test::MyStructMyEnumD),
                                                    Test::MyStructMyEnumD&,
                                                    const Ice::Current&);

    virtual Test::ByteBoolDS opByteBoolDS(ICE_IN(Test::ByteBoolDS),
                                          ICE_IN(Test::ByteBoolDS),
                                          Test::ByteBoolDS&,
                                          const Ice::Current&);

    virtual Test::ShortIntDS opShortIntDS(ICE_IN(Test::ShortIntDS),
                                          ICE_IN(Test::ShortIntDS),
                                          Test::ShortIntDS&,
                                          const Ice::Current&);

    virtual Test::LongFloatDS opLongFloatDS(ICE_IN(Test::LongFloatDS),
                                            ICE_IN(Test::LongFloatDS),
                                            Test::LongFloatDS&,
                                            const Ice::Current&);

    virtual Test::StringStringDS opStringStringDS(ICE_IN(Test::StringStringDS),
                                                  ICE_IN(Test::StringStringDS),
                                                  Test::StringStringDS&,
                                                  const Ice::Current&);

    virtual Test::StringMyEnumDS opStringMyEnumDS(ICE_IN(Test::StringMyEnumDS),
                                                  ICE_IN(Test::StringMyEnumDS),
                                                  Test::StringMyEnumDS&,
                                                  const Ice::Current&);

    virtual Test::MyStructMyEnumDS opMyStructMyEnumDS(ICE_IN(Test::MyStructMyEnumDS),
                                                      ICE_IN(Test::MyStructMyEnumDS),
                                                      Test::MyStructMyEnumDS&,
                                                      const Ice::Current&);

    virtual Test::MyEnumStringDS opMyEnumStringDS(ICE_IN(Test::MyEnumStringDS),
                                                  ICE_IN(Test::MyEnumStringDS),
                                                  Test::MyEnumStringDS&,
                                                  const Ice::Current&);

    virtual Test::ByteByteSD opByteByteSD(ICE_IN(Test::ByteByteSD),
                                          ICE_IN(Test::ByteByteSD),
                                          Test::ByteByteSD&,
                                          const Ice::Current&);

    virtual Test::BoolBoolSD opBoolBoolSD(ICE_IN(Test::BoolBoolSD),
                                          ICE_IN(Test::BoolBoolSD),
                                          Test::BoolBoolSD&,
                                          const Ice::Current&);

    virtual Test::ShortShortSD opShortShortSD(ICE_IN(Test::ShortShortSD),
                                              ICE_IN(Test::ShortShortSD),
                                              Test::ShortShortSD&,
                                              const Ice::Current&);

    virtual Test::IntIntSD opIntIntSD(ICE_IN(Test::IntIntSD),
                                      ICE_IN(Test::IntIntSD),
                                      Test::IntIntSD&,
                                      const Ice::Current&);

    virtual Test::LongLongSD opLongLongSD(ICE_IN(Test::LongLongSD),
                                          ICE_IN(Test::LongLongSD),
                                          Test::LongLongSD&,
                                          const Ice::Current&);

    virtual Test::StringFloatSD opStringFloatSD(ICE_IN(Test::StringFloatSD),
                                                ICE_IN(Test::StringFloatSD),
                                                Test::StringFloatSD&,
                                                const Ice::Current&);

    virtual Test::StringDoubleSD opStringDoubleSD(ICE_IN(Test::StringDoubleSD),
                                                  ICE_IN(Test::StringDoubleSD),
                                                  Test::StringDoubleSD&,
                                                  const Ice::Current&);

    virtual Test::StringStringSD opStringStringSD(ICE_IN(Test::StringStringSD),
                                                  ICE_IN(Test::StringStringSD),
                                                  Test::StringStringSD&,
                                                  const Ice::Current&);

    virtual Test::MyEnumMyEnumSD opMyEnumMyEnumSD(ICE_IN(Test::MyEnumMyEnumSD),
                                                  ICE_IN(Test::MyEnumMyEnumSD),
                                                  Test::MyEnumMyEnumSD&,
                                                  const Ice::Current&);

    virtual Test::IntS opIntS(ICE_IN(Test::IntS), const Ice::Current&);

    virtual void opByteSOneway(ICE_IN(Test::ByteS), const Ice::Current&);
    virtual int opByteSOnewayCallCount(const Ice::Current&);

    virtual Ice::Context opContext(const Ice::Current&);

    virtual void opDoubleMarshaling(Ice::Double, ICE_IN(Test::DoubleS), const Ice::Current&);

    virtual void opIdempotent(const Ice::Current&);

    virtual void opNonmutating(const Ice::Current&);

    virtual void opDerived(const Ice::Current&);

    virtual Ice::Byte opByte1(Ice::Byte, const Ice::Current&);

    virtual Ice::Short opShort1(Ice::Short, const Ice::Current&);

    virtual Ice::Int opInt1(Ice::Int, const Ice::Current&);

    virtual Ice::Long opLong1(Ice::Long, const Ice::Current&);

    virtual Ice::Float opFloat1(Ice::Float, const Ice::Current&);

    virtual Ice::Double opDouble1(Ice::Double, const Ice::Current&);

    virtual std::string opString1(ICE_IN(std::string), const Ice::Current&);

    virtual Test::StringS opStringS1(ICE_IN(Test::StringS), const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD1(ICE_IN(Test::ByteBoolD), const Ice::Current&);

    virtual Test::StringS opStringS2(ICE_IN(Test::StringS), const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD2(ICE_IN(Test::ByteBoolD), const Ice::Current&);

    virtual Test::MyStruct1 opMyStruct1(ICE_IN(Test::MyStruct1), const Ice::Current&);

    virtual Test::MyClass1Ptr opMyClass1(ICE_IN(Test::MyClass1Ptr), const Ice::Current&);

    virtual Test::StringS opStringLiterals(const Ice::Current&);

    virtual Test::WStringS opWStringLiterals(const Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual OpMStruct1MarshaledResult opMStruct1(const Ice::Current&);

    virtual OpMStruct2MarshaledResult opMStruct2(ICE_IN(Test::Structure), const Ice::Current&);

    virtual OpMSeq1MarshaledResult opMSeq1(const Ice::Current&);

    virtual OpMSeq2MarshaledResult opMSeq2(ICE_IN(Test::StringS), const Ice::Current&);

    virtual OpMDict1MarshaledResult opMDict1(const Ice::Current&);

    virtual OpMDict2MarshaledResult opMDict2(ICE_IN(Test::StringStringD), const Ice::Current&);
#else
    virtual Test::Structure opMStruct1(const Ice::Current&);

    virtual Test::Structure opMStruct2(ICE_IN(Test::Structure), Test::Structure&, const Ice::Current&);

    virtual Test::StringS opMSeq1(const Ice::Current&);

    virtual Test::StringS opMSeq2(ICE_IN(Test::StringS), Test::StringS&, const Ice::Current&);

    virtual Test::StringStringD opMDict1(const Ice::Current&);

    virtual Test::StringStringD opMDict2(ICE_IN(Test::StringStringD), Test::StringStringD&, const Ice::Current&);
#endif

private:

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

#ifdef ICE_CPP11_MAPPING
class BI : public M::BDisp
#else
class BI : public M::B
#endif
{
public:

    void opIntf(const Ice::Current&);
    void opB(const Ice::Current&);
};

#endif
