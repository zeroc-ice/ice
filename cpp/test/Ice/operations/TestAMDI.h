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
#ifdef ICE_CPP11_MAPPING
    virtual std::string ice_id(const Ice::Current&) const;
#else
    virtual const std::string& ice_id(const Ice::Current&) const;
#endif

#ifdef ICE_CPP11_MAPPING
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

#else
    virtual void shutdown_async(const Test::AMD_MyClass_shutdownPtr&,
                                const Ice::Current&);

    virtual void supportsCompress_async(const Test::AMD_MyClass_supportsCompressPtr&,
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
                                 const Test::MyClassPrxPtr&,
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

    virtual void opMStruct1_async(const Test::AMD_MyClass_opMStruct1Ptr&,
                                  const Ice::Current&);

    virtual void opMStruct2_async(const Test::AMD_MyClass_opMStruct2Ptr&,
                                  const Test::Structure&,
                                  const Ice::Current&);

    virtual void opMSeq1_async(const Test::AMD_MyClass_opMSeq1Ptr&,
                               const Ice::Current&);

    virtual void opMSeq2_async(const Test::AMD_MyClass_opMSeq2Ptr&,
                               const Test::StringS&,
                               const Ice::Current&);

    virtual void opMDict1_async(const Test::AMD_MyClass_opMDict1Ptr&,
                                const Ice::Current&);

    virtual void opMDict2_async(const Test::AMD_MyClass_opMDict2Ptr&,
                                const Test::StringStringD&,
                                const Ice::Current&);
#endif

private:
    IceUtil::ThreadPtr _opVoidThread;
    IceUtil::Mutex _opVoidMutex;

    IceUtil::Mutex _mutex;
    int _opByteSOnewayCallCount;
};

#endif
