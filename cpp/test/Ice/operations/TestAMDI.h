//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    virtual bool ice_isA(ICE_IN(std::string), const Ice::Current&) const;
    virtual void ice_ping(const Ice::Current&) const;
    virtual std::vector<std::string> ice_ids(const Ice::Current&) const;
    virtual std::string ice_id(const Ice::Current&) const;

    virtual void shutdownAsync(::std::function<void()>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void supportsCompressAsync(std::function<void(bool)>,
                                       std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opVoidAsync(::std::function<void()>,
                             ::std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void opByteAsync(Ice::Byte, Ice::Byte,
                             ::std::function<void(Ice::Byte, Ice::Byte)>,
                             ::std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void opBoolAsync(bool, bool,
                             ::std::function<void(bool, bool)>,
                             ::std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void opShortIntLongAsync(short, int, long long int,
                                     ::std::function<void(long long int, short, int, long long int)>,
                                     ::std::function<void(std::exception_ptr)>,
                                     const Ice::Current&);

    virtual void opFloatDoubleAsync(float, double,
                                    ::std::function<void(double, float, double)>,
                                    ::std::function<void(std::exception_ptr)>,
                                    const Ice::Current&);

    virtual void opStringAsync(std::string, std::string,
                               ::std::function<void(const ::std::string&, const ::std::string&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opMyEnumAsync(Test::MyEnum,
                               std::function<void(Test::MyEnum, Test::MyEnum)>,
                               std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opMyClassAsync(std::shared_ptr<Test::MyClassPrx>,
                                std::function<void(const std::shared_ptr<Test::MyClassPrx>&,
                                                    const std::shared_ptr<Test::MyClassPrx>&,
                                                    const std::shared_ptr<Test::MyClassPrx>&)>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opStructAsync(Test::Structure, Test::Structure,
                               ::std::function<void(const Test::Structure&, const Test::Structure&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opByteSAsync(Test::ByteS, Test::ByteS,
                              ::std::function<void(const Test::ByteS&, const Test::ByteS&)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opBoolSAsync(Test::BoolS, Test::BoolS,
                              ::std::function<void(const Test::BoolS&, const Test::BoolS&)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opShortIntLongSAsync(Test::ShortS, Test::IntS, Test::LongS,
                                      ::std::function<void(const Test::LongS&,
                                                            const Test::ShortS&,
                                                            const Test::IntS&,
                                                            const Test::LongS&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opFloatDoubleSAsync(Test::FloatS, Test::DoubleS,
                                     ::std::function<void(const Test::DoubleS&,
                                                           const Test::FloatS&,
                                                           const Test::DoubleS&)>,
                                     ::std::function<void(std::exception_ptr)>,
                                     const Ice::Current&);

    virtual void opStringSAsync(Test::StringS, Test::StringS,
                                ::std::function<void(const Test::StringS&, const Test::StringS&)>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opByteSSAsync(Test::ByteSS, Test::ByteSS,
                               ::std::function<void(const Test::ByteSS&, const Test::ByteSS&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opBoolSSAsync(Test::BoolSS, Test::BoolSS,
                               ::std::function<void(const Test::BoolSS&, const Test::BoolSS&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opShortIntLongSSAsync(Test::ShortSS, Test::IntSS, Test::LongSS,
                                       ::std::function<void(const Test::LongSS&,
                                                             const Test::ShortSS&,
                                                             const Test::IntSS&,
                                                             const Test::LongSS&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opFloatDoubleSSAsync(Test::FloatSS, Test::DoubleSS,
                                      ::std::function<void(const Test::DoubleSS&,
                                                            const Test::FloatSS&,
                                                            const Test::DoubleSS&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opStringSSAsync(Test::StringSS, Test::StringSS,
                                 ::std::function<void(const Test::StringSS&, const Test::StringSS&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opStringSSSAsync(Test::StringSSS, Test::StringSSS,
                                  ::std::function<void(const Test::StringSSS&, const Test::StringSSS&)>,
                                  ::std::function<void(std::exception_ptr)>,
                                  const Ice::Current&);

    virtual void opByteBoolDAsync(Test::ByteBoolD, Test::ByteBoolD,
                                  ::std::function<void(const Test::ByteBoolD&, const Test::ByteBoolD&)>,
                                  ::std::function<void(std::exception_ptr)>,
                                  const Ice::Current&);

    virtual void opShortIntDAsync(Test::ShortIntD, Test::ShortIntD,
                                  ::std::function<void(const Test::ShortIntD&, const Test::ShortIntD&)>,
                                  ::std::function<void(std::exception_ptr)>,
                                  const Ice::Current&);

    virtual void opLongFloatDAsync(Test::LongFloatD, Test::LongFloatD,
                                   ::std::function<void(const Test::LongFloatD&, const Test::LongFloatD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opStringStringDAsync(Test::StringStringD, Test::StringStringD,
                                      ::std::function<void(const Test::StringStringD&, const Test::StringStringD&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opStringMyEnumDAsync(Test::StringMyEnumD, Test::StringMyEnumD,
                                      ::std::function<void(const Test::StringMyEnumD&, const Test::StringMyEnumD&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opMyEnumStringDAsync(Test::MyEnumStringD, Test::MyEnumStringD,
                                      ::std::function<void(const Test::MyEnumStringD&, const Test::MyEnumStringD&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opMyStructMyEnumDAsync(Test::MyStructMyEnumD, Test::MyStructMyEnumD,
                                        ::std::function<void(const Test::MyStructMyEnumD&,
                                                              const Test::MyStructMyEnumD&)>,
                                        ::std::function<void(std::exception_ptr)>,
                                        const Ice::Current&);

    virtual void opByteBoolDSAsync(Test::ByteBoolDS, Test::ByteBoolDS,
                                   ::std::function<void(const Test::ByteBoolDS&, const Test::ByteBoolDS&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opShortIntDSAsync(Test::ShortIntDS, Test::ShortIntDS,
                                   ::std::function<void(const Test::ShortIntDS&, const Test::ShortIntDS&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opLongFloatDSAsync(Test::LongFloatDS, Test::LongFloatDS,
                                    ::std::function<void(const Test::LongFloatDS&, const Test::LongFloatDS&)>,
                                    ::std::function<void(std::exception_ptr)>,
                                    const Ice::Current&);

    virtual void opStringStringDSAsync(Test::StringStringDS, Test::StringStringDS,
                                       ::std::function<void(const Test::StringStringDS&, const Test::StringStringDS&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opStringMyEnumDSAsync(Test::StringMyEnumDS, Test::StringMyEnumDS,
                                       ::std::function<void(const Test::StringMyEnumDS&,
                                                             const Test::StringMyEnumDS&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opMyEnumStringDSAsync(Test::MyEnumStringDS, Test::MyEnumStringDS,
                                       ::std::function<void(const Test::MyEnumStringDS&,
                                                             const Test::MyEnumStringDS&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opMyStructMyEnumDSAsync(Test::MyStructMyEnumDS, Test::MyStructMyEnumDS,
                                         ::std::function<void(const Test::MyStructMyEnumDS&,
                                                               const Test::MyStructMyEnumDS&)>,
                                         ::std::function<void(std::exception_ptr)>,
                                         const Ice::Current&);

    virtual void opByteByteSDAsync(Test::ByteByteSD, Test::ByteByteSD,
                                   ::std::function<void(const Test::ByteByteSD&, const Test::ByteByteSD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opBoolBoolSDAsync(Test::BoolBoolSD, Test::BoolBoolSD,
                                   ::std::function<void(const Test::BoolBoolSD&, const Test::BoolBoolSD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opShortShortSDAsync(Test::ShortShortSD, Test::ShortShortSD,
                                     ::std::function<void(const Test::ShortShortSD&, const Test::ShortShortSD&)>,
                                     ::std::function<void(std::exception_ptr)>,
                                     const Ice::Current&);

    virtual void opIntIntSDAsync(Test::IntIntSD, Test::IntIntSD,
                                 ::std::function<void(const Test::IntIntSD&, const Test::IntIntSD&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opLongLongSDAsync(Test::LongLongSD, Test::LongLongSD,
                                   ::std::function<void(const Test::LongLongSD&, const Test::LongLongSD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opStringFloatSDAsync(Test::StringFloatSD, Test::StringFloatSD,
                                      ::std::function<void(const Test::StringFloatSD&, const Test::StringFloatSD&)>,
                                      ::std::function<void(std::exception_ptr)>,
                                      const Ice::Current&);

    virtual void opStringDoubleSDAsync(Test::StringDoubleSD, Test::StringDoubleSD,
                                       ::std::function<void(const Test::StringDoubleSD&,
                                                             const Test::StringDoubleSD&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opStringStringSDAsync(Test::StringStringSD, Test::StringStringSD,
                                       ::std::function<void(const Test::StringStringSD&,
                                                             const Test::StringStringSD&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opMyEnumMyEnumSDAsync(Test::MyEnumMyEnumSD, Test::MyEnumMyEnumSD,
                                       ::std::function<void(const Test::MyEnumMyEnumSD&,
                                                             const Test::MyEnumMyEnumSD&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opIntSAsync(Test::IntS,
                             ::std::function<void(const Test::IntS&)>,
                             ::std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void opByteSOnewayAsync(Test::ByteS,
                                    ::std::function<void()>,
                                    ::std::function<void(std::exception_ptr)>,
                                    const Ice::Current&);

    virtual void opByteSOnewayCallCountAsync(::std::function<void(int)>,
                                             ::std::function<void(std::exception_ptr)>,
                                             const Ice::Current&);

    virtual void opContextAsync(::std::function<void(const Ice::Context&)>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opDoubleMarshalingAsync(Ice::Double, Test::DoubleS,
                                         ::std::function<void()>,
                                         ::std::function<void(std::exception_ptr)>,
                                         const Ice::Current&);

    virtual void opIdempotentAsync(::std::function<void()>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opNonmutatingAsync(::std::function<void()>,
                                    ::std::function<void(std::exception_ptr)>,
                                    const Ice::Current&);

    virtual void opDerivedAsync(::std::function<void()>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opByte1Async(Ice::Byte,
                              ::std::function<void(Ice::Byte)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opShort1Async(Ice::Short,
                               ::std::function<void(Ice::Short)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opInt1Async(Ice::Int,
                             ::std::function<void(Ice::Int)>,
                             ::std::function<void(std::exception_ptr)>,
                             const Ice::Current&);

    virtual void opLong1Async(Ice::Long,
                              ::std::function<void(Ice::Long)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opFloat1Async(Ice::Float,
                               ::std::function<void(Ice::Float)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opDouble1Async(Ice::Double,
                                ::std::function<void(Ice::Double)>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opString1Async(std::string,
                                ::std::function<void(const ::std::string&)>,
                                ::std::function<void(std::exception_ptr)>,
                                const Ice::Current&);

    virtual void opStringS1Async(Test::StringS,
                                 ::std::function<void(const Test::StringS&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opByteBoolD1Async(Test::ByteBoolD,
                                   ::std::function<void(const Test::ByteBoolD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opStringS2Async(Test::StringS,
                                 ::std::function<void(const Test::StringS&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opByteBoolD2Async(Test::ByteBoolD,
                                   ::std::function<void(const Test::ByteBoolD&)>,
                                   ::std::function<void(std::exception_ptr)>,
                                   const Ice::Current&);

    virtual void opMyStruct1Async(Test::MyStruct1,
                                  ::std::function<void(const Test::MyStruct1&)>,
                                  ::std::function<void(std::exception_ptr)>,
                                  const Ice::Current&);

    virtual void opMyClass1Async(::std::shared_ptr<Test::MyClass1>,
                                 ::std::function<void(const ::std::shared_ptr<Test::MyClass1>&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opStringLiteralsAsync(::std::function<void(const Test::StringS&)>,
                                       ::std::function<void(std::exception_ptr)>,
                                       const Ice::Current&);

    virtual void opWStringLiteralsAsync(::std::function<void(const Test::WStringS&)>,
                                        ::std::function<void(std::exception_ptr)>,
                                        const Ice::Current&);

    virtual void opMStruct1Async(::std::function<void(const OpMStruct1MarshaledResult&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opMStruct2Async(ICE_IN(Test::Structure),
                                 ::std::function<void(const OpMStruct2MarshaledResult&)>,
                                 ::std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);

    virtual void opMSeq1Async(::std::function<void(const OpMSeq1MarshaledResult&)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opMSeq2Async(ICE_IN(Test::StringS),
                              ::std::function<void(const OpMSeq2MarshaledResult&)>,
                              ::std::function<void(std::exception_ptr)>,
                              const Ice::Current&);

    virtual void opMDict1Async(::std::function<void(const OpMDict1MarshaledResult&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void opMDict2Async(ICE_IN(Test::StringStringD),
                               ::std::function<void(const OpMDict2MarshaledResult&)>,
                               ::std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

private:
    IceUtil::ThreadPtr _opVoidThread;
    IceUtil::Mutex _opVoidMutex;

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

class BI : public M::BDisp
{
public:

    void opIntfAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    void opBAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&);
};

#endif
