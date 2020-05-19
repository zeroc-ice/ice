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
