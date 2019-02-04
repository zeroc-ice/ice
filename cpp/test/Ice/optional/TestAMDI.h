//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

class InitialI : public Test::Initial
{
public:

    InitialI();

#ifdef ICE_CPP11_MAPPING

    virtual void shutdownAsync(::std::function<void()>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void pingPongAsync(::std::shared_ptr<::Ice::Value>,
                               ::std::function<void(const ::std::shared_ptr<::Ice::Value>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opOptionalExceptionAsync(Ice::optional<int>, Ice::optional<::std::string>, Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                          ::std::function<void()>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDerivedExceptionAsync(Ice::optional<int>, Ice::optional<::std::string>, Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                         ::std::function<void()>,
                                         ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opRequiredExceptionAsync(Ice::optional<int>, Ice::optional<::std::string>, Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                          ::std::function<void()>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opByteAsync(Ice::optional<::Ice::Byte>,
                             ::std::function<void(const Ice::optional<::Ice::Byte>&, const Ice::optional<::Ice::Byte>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opBoolAsync(Ice::optional<bool>,
                             ::std::function<void(const Ice::optional<bool>&, const Ice::optional<bool>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opShortAsync(Ice::optional<short>,
                              ::std::function<void(const Ice::optional<short>&, const Ice::optional<short>&)>,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntAsync(Ice::optional<int>,
                            ::std::function<void(const Ice::optional<int>&, const Ice::optional<int>&)>,
                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opLongAsync(Ice::optional<long long int>,
                             ::std::function<void(const Ice::optional<long long int>&, const Ice::optional<long long int>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFloatAsync(Ice::optional<float>,
                              ::std::function<void(const Ice::optional<float>&, const Ice::optional<float>&)>,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDoubleAsync(Ice::optional<double>,
                               ::std::function<void(const Ice::optional<double>&, const Ice::optional<double>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringAsync(Ice::optional<::std::string>,
                               ::std::function<void(const Ice::optional<::std::string>&, const Ice::optional<::std::string>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opCustomStringAsync(Ice::optional<Util::string_view>,
                                     ::std::function<void(const Ice::optional<Util::string_view>&, const Ice::optional<Util::string_view>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opMyEnumAsync(Ice::optional<::Test::MyEnum>,
                               ::std::function<void(const Ice::optional<::Test::MyEnum>&, const Ice::optional<::Test::MyEnum>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructAsync(Ice::optional<::Test::SmallStruct>,
                                    ::std::function<void(const Ice::optional<::Test::SmallStruct>&, const Ice::optional<::Test::SmallStruct>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructAsync(Ice::optional<::Test::FixedStruct>,
                                    ::std::function<void(const Ice::optional<::Test::FixedStruct>&, const Ice::optional<::Test::FixedStruct>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVarStructAsync(Ice::optional<::Test::VarStruct>,
                                  ::std::function<void(const Ice::optional<::Test::VarStruct>&, const Ice::optional<::Test::VarStruct>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opOneOptionalAsync(Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                    ::std::function<void(const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&, const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opOneOptionalProxyAsync(Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>>,
                                         ::std::function<void(const Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>>&, const Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>>&)>,
                                         ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opByteSeqAsync(Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>,
                                ::std::function<void(const Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&, const Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opBoolSeqAsync(Ice::optional<::std::pair<const bool*, const bool*>>,
                                ::std::function<void(const Ice::optional<::std::pair<const bool*, const bool*>>&, const Ice::optional<::std::pair<const bool*, const bool*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opShortSeqAsync(Ice::optional<::std::pair<const short*, const short*>>,
                                 ::std::function<void(const Ice::optional<::std::pair<const short*, const short*>>&, const Ice::optional<::std::pair<const short*, const short*>>&)>,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntSeqAsync(Ice::optional<::std::pair<const int*, const int*>>,
                               ::std::function<void(const Ice::optional<::std::pair<const int*, const int*>>&, const Ice::optional<::std::pair<const int*, const int*>>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opLongSeqAsync(Ice::optional<::std::pair<const long long int*, const long long int*>>,
                                ::std::function<void(const Ice::optional<::std::pair<const long long int*, const long long int*>>&, const Ice::optional<::std::pair<const long long int*, const long long int*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFloatSeqAsync(Ice::optional<::std::pair<const float*, const float*>>,
                                 ::std::function<void(const Ice::optional<::std::pair<const float*, const float*>>&, const Ice::optional<::std::pair<const float*, const float*>>&)>,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDoubleSeqAsync(Ice::optional<::std::pair<const double*, const double*>>,
                                  ::std::function<void(const Ice::optional<::std::pair<const double*, const double*>>&, const Ice::optional<::std::pair<const double*, const double*>>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringSeqAsync(Ice::optional<::Test::StringSeq>,
                                  ::std::function<void(const Ice::optional<::Test::StringSeq>&, const Ice::optional<::Test::StringSeq>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructSeqAsync(Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
                                       ::std::function<void(const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>,
                                       ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructListAsync(Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
                                        ::std::function<void(const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructSeqAsync(Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
                                       ::std::function<void(const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>,
                                       ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructListAsync(Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
                                        ::std::function<void(const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVarStructSeqAsync(Ice::optional<::Test::VarStructSeq>,
                                     ::std::function<void(const Ice::optional<::Test::VarStructSeq>&, const Ice::optional<::Test::VarStructSeq>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSerializableAsync(Ice::optional<::Test::Serializable>,
                                     ::std::function<void(const Ice::optional<::Test::Serializable>&, const Ice::optional<::Test::Serializable>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntIntDictAsync(Ice::optional<::Test::IntIntDict>,
                                   ::std::function<void(const Ice::optional<::Test::IntIntDict>&, const Ice::optional<::Test::IntIntDict>&)>,
                                   ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringIntDictAsync(Ice::optional<::Test::StringIntDict>,
                                      ::std::function<void(const Ice::optional<::Test::StringIntDict>&, const Ice::optional<::Test::StringIntDict>&)>,
                                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntOneOptionalDictAsync(Ice::optional<::Test::IntOneOptionalDict>,
                                           ::std::function<void(const Ice::optional<::Test::IntOneOptionalDict>&, const Ice::optional<::Test::IntOneOptionalDict>&)>,
                                           ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opCustomIntStringDictAsync(Ice::optional<::std::map< int, ::Util::string_view>>,
                                            ::std::function<void(const Ice::optional<::std::map< int, ::Util::string_view>>&, const Ice::optional<::std::map< int, ::Util::string_view>>&)>,
                                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opClassAndUnknownOptionalAsync(::std::shared_ptr<::Test::A>,
                                                ::std::function<void()>,
                                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void sendOptionalClassAsync(bool, Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                        ::std::function<void()>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void returnOptionalClassAsync(bool,
                                          ::std::function<void(const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&)>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opGAsync(::std::shared_ptr<::Test::G>,
                          ::std::function<void(const ::std::shared_ptr<::Test::G>&)>,
                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVoidAsync(::std::function<void()>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

#ifdef ICE_CPP11_MAPPING
    virtual void opMStruct1Async(::std::function<void(const OpMStruct1MarshaledResult&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMStruct2Async(Ice::optional<Test::SmallStruct>,
                                 ::std::function<void(const OpMStruct2MarshaledResult&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMSeq1Async(::std::function<void(const OpMSeq1MarshaledResult&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMSeq2Async(Ice::optional<Test::StringSeq>,
                              ::std::function<void(const OpMSeq2MarshaledResult&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMDict1Async(::std::function<void(const OpMDict1MarshaledResult&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMDict2Async(Ice::optional<Test::StringIntDict>,
                               ::std::function<void(const OpMDict2MarshaledResult&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMG1Async(::std::function<void(const OpMG1MarshaledResult&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;

    virtual void opMG2Async(Ice::optional<Test::GPtr>,
                            ::std::function<void(const OpMG2MarshaledResult&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;
#else
    virtual void opMStruct1Async(::std::function<void(const Ice::optional<Test::SmallStruct>&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMStruct2Async(Ice::optional<Test::SmallStruct>,
                                 ::std::function<void(const Ice::optional<Test::SmallStruct>&,
                                                      const Ice::optional<Test::SmallStruct>&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMSeq1Async(::std::function<void(const Ice::optional<Test::StringSeq>&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMSeq2Async(Ice::optional<Test::StringSeq>,
                              ::std::function<void(const Ice::optional<Test::StringSeq>&,
                                                   const Ice::optional<Test::StringSeq>&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMDict1Async(::std::function<void(const Ice::optional<Test::StringIntDict>&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMDict2Async(Ice::optional<Test::StringIntDict>,
                               ::std::function<void(const Ice::optional<Test::StringIntDict>&,
                                                    const Ice::optional<Test::StringIntDict>&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMG1Async(::std::function<void(const Ice::optional<Test::GPtr>&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;

    virtual void opMG2Async(Ice::optional<Test::GPtr>,
                            ::std::function<void(const Ice::optional<Test::GPtr>&,
                                                 const Ice::optional<Test::GPtr>&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;
#endif

    virtual void supportsRequiredParamsAsync(::std::function<void(bool)>,
                                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void supportsJavaSerializableAsync(::std::function<void(bool)>,
                                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void supportsCsharpSerializableAsync(::std::function<void(bool)>,
                                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void supportsCppStringViewAsync(::std::function<void(bool)>,
                                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void supportsNullOptionalAsync(::std::function<void(bool)>,
                                           ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

#else // C++98 mapping

    virtual void shutdown_async(const ::Test::AMD_Initial_shutdownPtr&,
                                const Ice::Current&);

    virtual void pingPong_async(const ::Test::AMD_Initial_pingPongPtr&,
                                const Ice::ObjectPtr&,
                                const Ice::Current&);

    virtual void opOptionalException_async(const ::Test::AMD_Initial_opOptionalExceptionPtr&,
                                           const IceUtil::Optional< ::Ice::Int>&,
                                           const IceUtil::Optional< ::std::string>&,
                                           const IceUtil::Optional<Test::OneOptionalPtr>&,
                                           const Ice::Current&);

    virtual void opDerivedException_async(const ::Test::AMD_Initial_opDerivedExceptionPtr&,
                                          const IceUtil::Optional< ::Ice::Int>&,
                                          const IceUtil::Optional< ::std::string>&,
                                          const IceUtil::Optional<Test::OneOptionalPtr>&,
                                          const Ice::Current&);

    virtual void opRequiredException_async(const ::Test::AMD_Initial_opRequiredExceptionPtr&,
                                           const IceUtil::Optional< ::Ice::Int>&,
                                           const IceUtil::Optional< ::std::string>&,
                                           const IceUtil::Optional<Test::OneOptionalPtr>&,
                                           const Ice::Current&);

    virtual void opByte_async(const ::Test::AMD_Initial_opBytePtr&,
                              const IceUtil::Optional< ::Ice::Byte>&,
                              const ::Ice::Current&);

    virtual void opBool_async(const ::Test::AMD_Initial_opBoolPtr&,
                              const IceUtil::Optional< bool>&,
                              const ::Ice::Current&);

    virtual void opShort_async(const ::Test::AMD_Initial_opShortPtr&,
                               const IceUtil::Optional< ::Ice::Short>&,
                               const ::Ice::Current&);

    virtual void opInt_async(const ::Test::AMD_Initial_opIntPtr&,
                             const IceUtil::Optional< ::Ice::Int>&,
                             const ::Ice::Current&);

    virtual void opLong_async(const ::Test::AMD_Initial_opLongPtr&,
                              const IceUtil::Optional< ::Ice::Long>&,
                              const ::Ice::Current&);

    virtual void opFloat_async(const ::Test::AMD_Initial_opFloatPtr&,
                               const IceUtil::Optional< ::Ice::Float>&,
                               const ::Ice::Current&);

    virtual void opDouble_async(const ::Test::AMD_Initial_opDoublePtr&,
                                const IceUtil::Optional< ::Ice::Double>&,
                                const ::Ice::Current&);

    virtual void opString_async(const ::Test::AMD_Initial_opStringPtr&,
                                const IceUtil::Optional< ::std::string>&,
                                const ::Ice::Current&);

    virtual void opCustomString_async(const ::Test::AMD_Initial_opCustomStringPtr&,
                                      const IceUtil::Optional< Util::string_view>&,
                                      const ::Ice::Current&);

    virtual void opMyEnum_async(const ::Test::AMD_Initial_opMyEnumPtr&,
                                const IceUtil::Optional<Test::MyEnum>&,
                                const ::Ice::Current&);

    virtual void opSmallStruct_async(const ::Test::AMD_Initial_opSmallStructPtr&,
                                     const IceUtil::Optional<Test::SmallStruct>&,
                                     const ::Ice::Current&);

    virtual void opFixedStruct_async(const ::Test::AMD_Initial_opFixedStructPtr&,
                                     const IceUtil::Optional<Test::FixedStruct>&,
                                     const ::Ice::Current&);

    virtual void opVarStruct_async(const ::Test::AMD_Initial_opVarStructPtr&,
                                   const IceUtil::Optional<Test::VarStruct>&,
                                   const ::Ice::Current&);

    virtual void opOneOptional_async(const ::Test::AMD_Initial_opOneOptionalPtr&,
                                     const IceUtil::Optional< Test::OneOptionalPtr>&,
                                     const ::Ice::Current&);

    virtual void opOneOptionalProxy_async(const ::Test::AMD_Initial_opOneOptionalProxyPtr&,
                                          const IceUtil::Optional< Test::OneOptionalPrx>&,
                                          const ::Ice::Current&);

    virtual void opByteSeq_async(const ::Test::AMD_Initial_opByteSeqPtr&,
                                 const IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >&,
                                 const ::Ice::Current&);

    virtual void opBoolSeq_async(const ::Test::AMD_Initial_opBoolSeqPtr&,
                                 const IceUtil::Optional< ::std::pair<const bool*, const bool*> >&,
                                 const ::Ice::Current&);

    virtual void opShortSeq_async(const ::Test::AMD_Initial_opShortSeqPtr&,
                                  const IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >&,
                                  const ::Ice::Current&);

    virtual void opIntSeq_async(const ::Test::AMD_Initial_opIntSeqPtr&,
                                const IceUtil::Optional< ::std::pair<const ::Ice::Int*, const ::Ice::Int*> >&,
                                const ::Ice::Current&);

    virtual void opLongSeq_async(const ::Test::AMD_Initial_opLongSeqPtr&,
                                 const IceUtil::Optional< ::std::pair<const ::Ice::Long*, const ::Ice::Long*> >&,
                                 const ::Ice::Current&);

    virtual void opFloatSeq_async(const ::Test::AMD_Initial_opFloatSeqPtr&,
                                  const IceUtil::Optional< ::std::pair<const ::Ice::Float*, const ::Ice::Float*> >&,
                                  const ::Ice::Current&);

    virtual void opDoubleSeq_async(const ::Test::AMD_Initial_opDoubleSeqPtr&,
                                   const IceUtil::Optional< ::std::pair<const ::Ice::Double*, const ::Ice::Double*> >&,
                                   const ::Ice::Current&);

    virtual void opStringSeq_async(const ::Test::AMD_Initial_opStringSeqPtr&,
                                   const IceUtil::Optional< ::std::pair< ::Test::StringSeq::const_iterator,
                                                                         ::Test::StringSeq::const_iterator> >&,
                                   const ::Ice::Current&);

    virtual void opSmallStructSeq_async(const ::Test::AMD_Initial_opSmallStructSeqPtr&,
                                        const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*,
                                                                             const ::Test::SmallStruct*> >&,
                                        const ::Ice::Current&);

    virtual void opSmallStructList_async(const ::Test::AMD_Initial_opSmallStructListPtr&,
                                         const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*,
                                                                              const ::Test::SmallStruct*> >&,
                                         const ::Ice::Current&);

    virtual void opFixedStructSeq_async(const ::Test::AMD_Initial_opFixedStructSeqPtr&,
                                        const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*,
                                                                             const ::Test::FixedStruct*> >&,
                                        const ::Ice::Current&);

    virtual void opFixedStructList_async(const ::Test::AMD_Initial_opFixedStructListPtr&,
                                         const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*,
                                                                              const ::Test::FixedStruct*> >&,
                                         const ::Ice::Current&);

    virtual void opVarStructSeq_async(const ::Test::AMD_Initial_opVarStructSeqPtr&,
                                      const IceUtil::Optional< ::std::pair< ::Test::VarStructSeq::const_iterator,
                                                                            ::Test::VarStructSeq::const_iterator> >&,
                                      const ::Ice::Current&);

    virtual void opSerializable_async(const ::Test::AMD_Initial_opSerializablePtr&,
                                      const IceUtil::Optional< ::Test::Serializable>&,
                                      const ::Ice::Current&);

    virtual void opIntIntDict_async(const ::Test::AMD_Initial_opIntIntDictPtr&,
                                    const IceUtil::Optional< ::Test::IntIntDict>&,
                                    const ::Ice::Current&);

    virtual void opStringIntDict_async(const ::Test::AMD_Initial_opStringIntDictPtr&,
                                       const IceUtil::Optional< ::Test::StringIntDict>&,
                                       const ::Ice::Current&);

    virtual void opIntOneOptionalDict_async(const ::Test::AMD_Initial_opIntOneOptionalDictPtr&,
                                            const IceUtil::Optional< ::Test::IntOneOptionalDict>&,
                                            const ::Ice::Current&);

    virtual void opCustomIntStringDict_async(const ::Test::AMD_Initial_opCustomIntStringDictPtr&,
                                             const IceUtil::Optional<std::map<int, Util::string_view> >&,
                                             const ::Ice::Current&);

    virtual void opClassAndUnknownOptional_async(const ::Test::AMD_Initial_opClassAndUnknownOptionalPtr&,
                                                 const Test::APtr&,
                                                 const Ice::Current&);

    virtual void sendOptionalClass_async(const ::Test::AMD_Initial_sendOptionalClassPtr&,
                                         bool,
                                         const IceUtil::Optional<Test::OneOptionalPtr>&,
                                         const Ice::Current&);

    virtual void returnOptionalClass_async(const ::Test::AMD_Initial_returnOptionalClassPtr&,
                                           bool,
                                           const Ice::Current&);

    virtual void opG_async(const ::Test::AMD_Initial_opGPtr&,
                           const ::Test::GPtr&,
                           const Ice::Current&);

    virtual void opVoid_async(const ::Test::AMD_Initial_opVoidPtr&,
                              const Ice::Current&);

    virtual void opMStruct1_async(const Test::AMD_Initial_opMStruct1Ptr&, const Ice::Current&);

    virtual void opMStruct2_async(const Test::AMD_Initial_opMStruct2Ptr&,
                                  const IceUtil::Optional< Test::SmallStruct>&,
                                  const Ice::Current&);

    virtual void opMSeq1_async(const Test::AMD_Initial_opMSeq1Ptr&, const Ice::Current&);

    virtual void opMSeq2_async(const Test::AMD_Initial_opMSeq2Ptr&,
                               const IceUtil::Optional< Test::StringSeq>&,
                               const Ice::Current&);

    virtual void opMDict1_async(const Test::AMD_Initial_opMDict1Ptr&, const Ice::Current&);

    virtual void opMDict2_async(const Test::AMD_Initial_opMDict2Ptr&, const
                                IceUtil::Optional< Test::StringIntDict>&,
                                const Ice::Current&);

    virtual void opMG1_async(const Test::AMD_Initial_opMG1Ptr&, const Ice::Current&);

    virtual void opMG2_async(const Test::AMD_Initial_opMG2Ptr&,
                             const IceUtil::Optional< Test::GPtr>&,
                             const Ice::Current&);

    virtual void supportsRequiredParams_async(const ::Test::AMD_Initial_supportsRequiredParamsPtr&,
                                              const Ice::Current&);

    virtual void supportsJavaSerializable_async(const ::Test::AMD_Initial_supportsJavaSerializablePtr&,
                                                const Ice::Current&);

    virtual void supportsCsharpSerializable_async(const ::Test::AMD_Initial_supportsCsharpSerializablePtr&,
                                                  const Ice::Current&);

    virtual void supportsCppStringView_async(const ::Test::AMD_Initial_supportsCppStringViewPtr&,
                                             const Ice::Current&);

    virtual void supportsNullOptional_async(const ::Test::AMD_Initial_supportsNullOptionalPtr&, const Ice::Current&);

#endif

};

#endif
