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

    virtual void shutdownAsync(::std::function<void()>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void pingPongAsync(::std::shared_ptr<::Ice::Value>,
                               ::std::function<void(const ::std::shared_ptr<::Ice::Value>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opOptionalExceptionAsync(std::optional<int>, std::optional<::std::string>, std::optional<::std::shared_ptr<::Test::OneOptional>>,
                                          ::std::function<void()>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDerivedExceptionAsync(std::optional<int>, std::optional<::std::string>, std::optional<::std::shared_ptr<::Test::OneOptional>>,
                                         ::std::function<void()>,
                                         ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opRequiredExceptionAsync(std::optional<int>, std::optional<::std::string>, std::optional<::std::shared_ptr<::Test::OneOptional>>,
                                          ::std::function<void()>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opByteAsync(std::optional<::Ice::Byte>,
                             ::std::function<void(const std::optional<::Ice::Byte>&, const std::optional<::Ice::Byte>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opBoolAsync(std::optional<bool>,
                             ::std::function<void(const std::optional<bool>&, const std::optional<bool>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opShortAsync(std::optional<short>,
                              ::std::function<void(const std::optional<short>&, const std::optional<short>&)>,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntAsync(std::optional<int>,
                            ::std::function<void(const std::optional<int>&, const std::optional<int>&)>,
                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opLongAsync(std::optional<std::int64_t>,
                             ::std::function<void(const std::optional<std::int64_t>&, const std::optional<std::int64_t>&)>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFloatAsync(std::optional<float>,
                              ::std::function<void(const std::optional<float>&, const std::optional<float>&)>,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDoubleAsync(std::optional<double>,
                               ::std::function<void(const std::optional<double>&, const std::optional<double>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringAsync(std::optional<::std::string>,
                               ::std::function<void(const std::optional<::std::string>&, const std::optional<::std::string>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opCustomStringAsync(std::optional<Util::string_view>,
                                     ::std::function<void(const std::optional<Util::string_view>&, const std::optional<Util::string_view>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opMyEnumAsync(std::optional<::Test::MyEnum>,
                               ::std::function<void(const std::optional<::Test::MyEnum>&, const std::optional<::Test::MyEnum>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructAsync(std::optional<::Test::SmallStruct>,
                                    ::std::function<void(const std::optional<::Test::SmallStruct>&, const std::optional<::Test::SmallStruct>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructAsync(std::optional<::Test::FixedStruct>,
                                    ::std::function<void(const std::optional<::Test::FixedStruct>&, const std::optional<::Test::FixedStruct>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVarStructAsync(std::optional<::Test::VarStruct>,
                                  ::std::function<void(const std::optional<::Test::VarStruct>&, const std::optional<::Test::VarStruct>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opOneOptionalAsync(std::optional<::std::shared_ptr<::Test::OneOptional>>,
                                    ::std::function<void(const std::optional<::std::shared_ptr<::Test::OneOptional>>&, const std::optional<::std::shared_ptr<::Test::OneOptional>>&)>,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opMyInterfaceProxyAsync(std::optional<::std::shared_ptr<Test::MyInterfacePrx>>,
                                         ::std::function<void(const std::optional<::std::shared_ptr<Test::MyInterfacePrx>>&, const std::optional<::std::shared_ptr<Test::MyInterfacePrx>>&)>,
                                         ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opByteSeqAsync(std::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>,
                                ::std::function<void(const std::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&, const std::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opBoolSeqAsync(std::optional<::std::pair<const bool*, const bool*>>,
                                ::std::function<void(const std::optional<::std::pair<const bool*, const bool*>>&, const std::optional<::std::pair<const bool*, const bool*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opShortSeqAsync(std::optional<::std::pair<const short*, const short*>>,
                                 ::std::function<void(const std::optional<::std::pair<const short*, const short*>>&, const std::optional<::std::pair<const short*, const short*>>&)>,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntSeqAsync(std::optional<::std::pair<const int*, const int*>>,
                               ::std::function<void(const std::optional<::std::pair<const int*, const int*>>&, const std::optional<::std::pair<const int*, const int*>>&)>,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opLongSeqAsync(std::optional<::std::pair<const std::int64_t*, const std::int64_t*>>,
                                ::std::function<void(const std::optional<::std::pair<const std::int64_t*, const std::int64_t*>>&, const std::optional<::std::pair<const std::int64_t*, const std::int64_t*>>&)>,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFloatSeqAsync(std::optional<::std::pair<const float*, const float*>>,
                                 ::std::function<void(const std::optional<::std::pair<const float*, const float*>>&, const std::optional<::std::pair<const float*, const float*>>&)>,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opDoubleSeqAsync(std::optional<::std::pair<const double*, const double*>>,
                                  ::std::function<void(const std::optional<::std::pair<const double*, const double*>>&, const std::optional<::std::pair<const double*, const double*>>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringSeqAsync(std::optional<::Test::StringSeq>,
                                  ::std::function<void(const std::optional<::Test::StringSeq>&, const std::optional<::Test::StringSeq>&)>,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructSeqAsync(std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
                                       ::std::function<void(const std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>,
                                       ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSmallStructListAsync(std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
                                        ::std::function<void(const std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const std::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructSeqAsync(std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
                                       ::std::function<void(const std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>,
                                       ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opFixedStructListAsync(std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
                                        ::std::function<void(const std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const std::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVarStructSeqAsync(std::optional<::Test::VarStructSeq>,
                                     ::std::function<void(const std::optional<::Test::VarStructSeq>&, const std::optional<::Test::VarStructSeq>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opSerializableAsync(std::optional<::Test::Serializable>,
                                     ::std::function<void(const std::optional<::Test::Serializable>&, const std::optional<::Test::Serializable>&)>,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntIntDictAsync(std::optional<::Test::IntIntDict>,
                                   ::std::function<void(const std::optional<::Test::IntIntDict>&, const std::optional<::Test::IntIntDict>&)>,
                                   ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opStringIntDictAsync(std::optional<::Test::StringIntDict>,
                                      ::std::function<void(const std::optional<::Test::StringIntDict>&, const std::optional<::Test::StringIntDict>&)>,
                                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opIntOneOptionalDictAsync(std::optional<::Test::IntOneOptionalDict>,
                                           ::std::function<void(const std::optional<::Test::IntOneOptionalDict>&, const std::optional<::Test::IntOneOptionalDict>&)>,
                                           ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opCustomIntStringDictAsync(std::optional<::std::map< int, ::Util::string_view>>,
                                            ::std::function<void(const std::optional<::std::map< int, ::Util::string_view>>&, const std::optional<::std::map< int, ::Util::string_view>>&)>,
                                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opClassAndUnknownOptionalAsync(::std::shared_ptr<::Test::A>,
                                                ::std::function<void()>,
                                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void sendOptionalClassAsync(bool, std::optional<::std::shared_ptr<::Test::OneOptional>>,
                                        ::std::function<void()>,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void returnOptionalClassAsync(bool,
                                          ::std::function<void(const std::optional<::std::shared_ptr<::Test::OneOptional>>&)>,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opGAsync(::std::shared_ptr<::Test::G>,
                          ::std::function<void(const ::std::shared_ptr<::Test::G>&)>,
                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opVoidAsync(::std::function<void()>,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&) override;

    virtual void opMStruct1Async(::std::function<void(const OpMStruct1MarshaledResult&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMStruct2Async(std::optional<Test::SmallStruct>,
                                 ::std::function<void(const OpMStruct2MarshaledResult&)>,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&) override;

    virtual void opMSeq1Async(::std::function<void(const OpMSeq1MarshaledResult&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMSeq2Async(std::optional<Test::StringSeq>,
                              ::std::function<void(const OpMSeq2MarshaledResult&)>,
                              ::std::function<void(::std::exception_ptr)>,
                              const Ice::Current&) override;

    virtual void opMDict1Async(::std::function<void(const OpMDict1MarshaledResult&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMDict2Async(std::optional<Test::StringIntDict>,
                               ::std::function<void(const OpMDict2MarshaledResult&)>,
                               ::std::function<void(::std::exception_ptr)>,
                               const Ice::Current&) override;

    virtual void opMG1Async(::std::function<void(const OpMG1MarshaledResult&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;

    virtual void opMG2Async(std::optional<Test::GPtr>,
                            ::std::function<void(const OpMG2MarshaledResult&)>,
                            ::std::function<void(::std::exception_ptr)>,
                            const Ice::Current&) override;

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
};

#endif
