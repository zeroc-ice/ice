// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>

using namespace Test;
using namespace IceUtil;
using namespace Ice;
using namespace std;

InitialI::InitialI()
{
}

#ifdef ICE_CPP11_MAPPING

void
InitialI::shutdownAsync(::std::function<void()> response,
                        ::std::function<void(::std::exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
InitialI::pingPongAsync(::std::shared_ptr<::Ice::Value> obj,
                        ::std::function<void(const ::std::shared_ptr<::Ice::Value>&)> response,
                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(obj);
    if(dynamic_pointer_cast<MultiOptional>(obj))
    {
        // Break cyclic reference count
        dynamic_pointer_cast<MultiOptional>(obj)->k = shared_ptr<MultiOptional>();
    }
}

void
InitialI::opOptionalExceptionAsync(Ice::optional<int> a, Ice::optional<::std::string> b, Ice::optional<::std::shared_ptr<::Test::OneOptional>> o,
                                   ::std::function<void()>,
                                   ::std::function<void(::std::exception_ptr)> ex, const Ice::Current&)
{
    ex(make_exception_ptr(OptionalException(false, a, b, o)));
}

void
InitialI::opDerivedExceptionAsync(Ice::optional<int> a, Ice::optional<::std::string> b, Ice::optional<::std::shared_ptr<::Test::OneOptional>> o,
                                  ::std::function<void()>,
                                  ::std::function<void(::std::exception_ptr)> ex, const Ice::Current&)
{
    ex(make_exception_ptr(DerivedException(false, a, b, o, b, o)));
}

void
InitialI::opRequiredExceptionAsync(Ice::optional<int> a, Ice::optional<::std::string> b, Ice::optional<::std::shared_ptr<::Test::OneOptional>> o,
                                   ::std::function<void()>,
                                   ::std::function<void(::std::exception_ptr)> ex, const Ice::Current&)
{
    RequiredException e;
    e.a = a;
    e.b = b;
    e.o = o;
    if(b)
    {
        e.ss = b.value();
    }
    if(o)
    {
        e.o2 = o.value();
    }

    ex(make_exception_ptr(e));
}

void
InitialI::opByteAsync(Ice::optional<::Ice::Byte> p1,
                      ::std::function<void(const Ice::optional<::Ice::Byte>&, const Ice::optional<::Ice::Byte>&)> response,
                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolAsync(Ice::optional<bool> p1,
                      ::std::function<void(const Ice::optional<bool>&, const Ice::optional<bool>&)> response,
                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortAsync(Ice::optional<short> p1,
                            ::std::function<void(const Ice::optional<short>&, const Ice::optional<short>&)> response,
                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntAsync(Ice::optional<int> p1,
                     ::std::function<void(const Ice::optional<int>&, const Ice::optional<int>&)> response,
                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongAsync(Ice::optional<long long int> p1,
                           ::std::function<void(const Ice::optional<long long int>&, const Ice::optional<long long int>&)> response,
                           ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatAsync(Ice::optional<float> p1,
                            ::std::function<void(const Ice::optional<float>&, const Ice::optional<float>&)> response,
                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleAsync(Ice::optional<double> p1,
                             ::std::function<void(const Ice::optional<double>&, const Ice::optional<double>&)> response,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringAsync(Ice::optional<::std::string> p1,
                             ::std::function<void(const Ice::optional<::std::string>&, const Ice::optional<::std::string>&)> response,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opCustomStringAsync(Ice::optional<Util::string_view> p1,
                                   ::std::function<void(const Ice::optional<Util::string_view>&, const Ice::optional<Util::string_view>&)> response,
                                   ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMyEnumAsync(Ice::optional<::Test::MyEnum> p1,
                             ::std::function<void(const Ice::optional<::Test::MyEnum>&, const Ice::optional<::Test::MyEnum>&)> response,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructAsync(Ice::optional<::Test::SmallStruct> p1,
                                  ::std::function<void(const Ice::optional<::Test::SmallStruct>&, const Ice::optional<::Test::SmallStruct>&)> response,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructAsync(Ice::optional<::Test::FixedStruct> p1,
                                  ::std::function<void(const Ice::optional<::Test::FixedStruct>&, const Ice::optional<::Test::FixedStruct>&)> response,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructAsync(Ice::optional<::Test::VarStruct> p1,
                                ::std::function<void(const Ice::optional<::Test::VarStruct>&, const Ice::optional<::Test::VarStruct>&)> response,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opOneOptionalAsync(Ice::optional<::std::shared_ptr<::Test::OneOptional>> p1,
                                  ::std::function<void(const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&, const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&)> response,
                                  ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opOneOptionalProxyAsync(Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>> p1,
                                       ::std::function<void(const Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>>&, const Ice::optional<::std::shared_ptr<::Ice::ObjectPrx>>&)> response,
                                       ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opByteSeqAsync(Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>> p1,
                              ::std::function<void(const Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&, const Ice::optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&)> response,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolSeqAsync(Ice::optional<::std::pair<const bool*, const bool*>> p1,
                              ::std::function<void(const Ice::optional<::std::pair<const bool*, const bool*>>&, const Ice::optional<::std::pair<const bool*, const bool*>>&)> response,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortSeqAsync(Ice::optional<::std::pair<const short*, const short*>> p1,
                               ::std::function<void(const Ice::optional<::std::pair<const short*, const short*>>&, const Ice::optional<::std::pair<const short*, const short*>>&)> response,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntSeqAsync(Ice::optional<::std::pair<const int*, const int*>> p1,
                             ::std::function<void(const Ice::optional<::std::pair<const int*, const int*>>&, const Ice::optional<::std::pair<const int*, const int*>>&)> response,
                             ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongSeqAsync(Ice::optional<::std::pair<const long long int*, const long long int*>> p1,
                              ::std::function<void(const Ice::optional<::std::pair<const long long int*, const long long int*>>&, const Ice::optional<::std::pair<const long long int*, const long long int*>>&)> response,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatSeqAsync(Ice::optional<::std::pair<const float*, const float*>> p1,
                               ::std::function<void(const Ice::optional<::std::pair<const float*, const float*>>&, const Ice::optional<::std::pair<const float*, const float*>>&)> response,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleSeqAsync(Ice::optional<::std::pair<const double*, const double*>> p1,
                                ::std::function<void(const Ice::optional<::std::pair<const double*, const double*>>&, const Ice::optional<::std::pair<const double*, const double*>>&)> response,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringSeqAsync(Ice::optional<::Test::StringSeq> p1,
                                ::std::function<void(const Ice::optional<::Test::StringSeq>&, const Ice::optional<::Test::StringSeq>&)> response,
                                ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructSeqAsync(Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>> p1,
                                     ::std::function<void(const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)> response,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructListAsync(Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>> p1,
                                      ::std::function<void(const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&, const Ice::optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)> response,
                                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructSeqAsync(Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>> p1,
                                     ::std::function<void(const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)> response,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructListAsync(Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>> p1,
                                 ::std::function<void(const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&, const Ice::optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)> response,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructSeqAsync(Ice::optional<::Test::VarStructSeq> p1,
                              ::std::function<void(const Ice::optional<::Test::VarStructSeq>&, const Ice::optional<::Test::VarStructSeq>&)> response,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSerializableAsync(Ice::optional<::Test::Serializable> p1,
                              ::std::function<void(const Ice::optional<::Test::Serializable>&, const Ice::optional<::Test::Serializable>&)> response,
                              ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntIntDictAsync(Ice::optional<::Test::IntIntDict> p1,
                            ::std::function<void(const Ice::optional<::Test::IntIntDict>&, const Ice::optional<::Test::IntIntDict>&)> response,
                            ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringIntDictAsync(Ice::optional<::Test::StringIntDict> p1,
                               ::std::function<void(const Ice::optional<::Test::StringIntDict>&, const Ice::optional<::Test::StringIntDict>&)> response,
                               ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntOneOptionalDictAsync(Ice::optional<::Test::IntOneOptionalDict> p1,
                                    ::std::function<void(const Ice::optional<::Test::IntOneOptionalDict>&, const Ice::optional<::Test::IntOneOptionalDict>&)> response,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opCustomIntStringDictAsync(Ice::optional<::std::map< int, ::Util::string_view>> p1,
                                     ::std::function<void(const Ice::optional<::std::map< int, ::Util::string_view>>&, const Ice::optional<::std::map< int, ::Util::string_view>>&)> response,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opClassAndUnknownOptionalAsync(::std::shared_ptr<::Test::A>,
                                         ::std::function<void()> response,
                                         ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response();
}

void
InitialI::sendOptionalClassAsync(bool, Ice::optional<::std::shared_ptr<::Test::OneOptional>>,
                                 ::std::function<void()> response,
                                 ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response();
}

void
InitialI::returnOptionalClassAsync(bool,
                                   ::std::function<void(const Ice::optional<::std::shared_ptr<::Test::OneOptional>>&)> response,
                                   ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(make_shared<OneOptional>(53));
}

void
InitialI::opGAsync(::std::shared_ptr<::Test::G> g,
                   ::std::function<void(const ::std::shared_ptr<::Test::G>&)> response,
                   ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(g);
}

void
InitialI::opVoidAsync(::std::function<void()> response,
                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response();
}

#ifdef ICE_CPP11_MAPPING
void
InitialI::opMStruct1Async(function<void(const OpMStruct1MarshaledResult&)> response,
                          function<void(exception_ptr)>,
                          const Ice::Current& current)
{
    response(OpMStruct1MarshaledResult(Test::SmallStruct(), current));
}

void
InitialI::opMStruct2Async(Ice::optional<SmallStruct> p1,
                         function<void(const OpMStruct2MarshaledResult&)> response,
                         function<void(exception_ptr)>,
                         const Ice::Current& current)
{
    response(OpMStruct2MarshaledResult(p1, p1, current));
}

void
InitialI::opMSeq1Async(function<void(const OpMSeq1MarshaledResult&)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current& current)
{
    response(OpMSeq1MarshaledResult(Test::StringSeq(), current));
}

void
InitialI::opMSeq2Async(Ice::optional<Test::StringSeq> p1,
                       function<void(const OpMSeq2MarshaledResult&)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current& current)
{
    response(OpMSeq2MarshaledResult(p1, p1, current));
}

void
InitialI::opMDict1Async(function<void(const OpMDict1MarshaledResult&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current& current)
{
    response(OpMDict1MarshaledResult(StringIntDict(), current));
}

void
InitialI::opMDict2Async(Ice::optional<StringIntDict> p1,
                        function<void(const OpMDict2MarshaledResult&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current& current)
{
    response(OpMDict2MarshaledResult(p1, p1, current));
}

void
InitialI::opMG1Async(function<void(const OpMG1MarshaledResult&)> response,
                     function<void(exception_ptr)>,
                     const Ice::Current& current)
{
    response(OpMG1MarshaledResult(ICE_MAKE_SHARED(G), current));
}

void
InitialI::opMG2Async(Ice::optional<GPtr> p1,
                     function<void(const OpMG2MarshaledResult&)> response,
                     function<void(exception_ptr)>,
                     const Ice::Current& current)
{
    response(OpMG2MarshaledResult(p1, p1, current));
}
#else
void
InitialI::opMStruct1Async(function<void(const Ice::optional<SmallStruct>&)> response,
                          function<void(exception_ptr)>,
                          const Ice::Current&)
{
    response(Test::SmallStruct());
}

void
InitialI::opMStruct2Async(Ice::optional<SmallStruct> p1,
                          function<void(const Ice::optional<SmallStruct>&, const Ice::optional<SmallStruct>&)> response,
                          function<void(exception_ptr)>,
                          const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMSeq1Async(function<void(const Ice::optional<Test::StringSeq>&)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current&)
{
    response(Test::StringSeq());
}

void
InitialI::opMSeq2Async(Ice::optional<Test::StringSeq> p1,
                       function<void(const Ice::optional<Test::StringSeq>&, const Ice::optional<Test::StringSeq>&)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMDict1Async(function<void(const Ice::optional<StringIntDict>&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current&)
{
    response(StringIntDict());
}

void
InitialI::opMDict2Async(Ice::optional<StringIntDict> p1,
                        function<void(const Ice::optional<StringIntDict>&, const Ice::optional<StringIntDict>&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMG1Async(function<void(const Ice::optional<shared_ptr<G>>&)> response,
                     function<void(exception_ptr)>,
                     const Ice::Current&)
{
    response(ICE_MAKE_SHARED(G));
}

void
InitialI::opMG2Async(Ice::optional<GPtr> p1,
                     function<void(const Ice::optional<shared_ptr<G>>&, const Ice::optional<shared_ptr<G>>&)> response,
                     function<void(exception_ptr)>,
                     const Ice::Current&)
{
    response(p1, p1);
}
#endif

void
InitialI::supportsRequiredParamsAsync(::std::function<void(bool)> response,
                                      ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(false);
}

void
InitialI::supportsJavaSerializableAsync(::std::function<void(bool)> response,
                                        ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(true);
}

void
InitialI::supportsCsharpSerializableAsync(::std::function<void(bool)> response,
                                          ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(true);
}

void
InitialI::supportsCppStringViewAsync(::std::function<void(bool)> response,
                                     ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(true);
}

void
InitialI::supportsNullOptionalAsync(::std::function<void(bool)> response,
                                    ::std::function<void(::std::exception_ptr)>, const Ice::Current&)
{
    response(true);
}

#else // C++98 mapping

void
InitialI::shutdown_async(const ::Test::AMD_Initial_shutdownPtr& cb,
                         const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    cb->ice_response();
}

void
InitialI::pingPong_async(const ::Test::AMD_Initial_pingPongPtr& cb,
                         const Ice::ObjectPtr& obj,
                         const Ice::Current&)
{
    cb->ice_response(obj);
}

void
InitialI::opOptionalException_async(const ::Test::AMD_Initial_opOptionalExceptionPtr& cb,
                                    const IceUtil::Optional< ::Ice::Int>& a,
                                    const IceUtil::Optional< ::std::string>& b,
                                    const IceUtil::Optional<Test::OneOptionalPtr>& o,
                                    const Ice::Current&)
{
    OptionalException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    cb->ice_exception(ex);
}

void
InitialI::opDerivedException_async(const ::Test::AMD_Initial_opDerivedExceptionPtr& cb,
                                   const IceUtil::Optional< ::Ice::Int>& a,
                                   const IceUtil::Optional< ::std::string>& b,
                                   const IceUtil::Optional<Test::OneOptionalPtr>& o,
                                   const Ice::Current&)
{
    DerivedException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    ex.ss = b;
    ex.o2 = o;
    cb->ice_exception(ex);
}

void
InitialI::opRequiredException_async(const ::Test::AMD_Initial_opRequiredExceptionPtr& cb,
                                    const IceUtil::Optional< ::Ice::Int>& a,
                                    const IceUtil::Optional< ::std::string>& b,
                                    const IceUtil::Optional<Test::OneOptionalPtr>& o,
                                    const Ice::Current&)
{
    RequiredException ex;
    ex.a = a;
    ex.b = b;
    ex.o = o;
    if(b)
    {
        ex.ss = b.get();
    }
    if(o)
    {
        ex.o2 = o.get();
    }
    cb->ice_exception(ex);
}

void
InitialI::opByte_async(const ::Test::AMD_Initial_opBytePtr& cb,
                       const IceUtil::Optional< ::Ice::Byte>& p1,
                       const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opBool_async(const ::Test::AMD_Initial_opBoolPtr& cb,
                       const IceUtil::Optional<bool>& p1,
                       const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opShort_async(const ::Test::AMD_Initial_opShortPtr& cb,
                        const IceUtil::Optional< ::Ice::Short>& p1,
                        const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opInt_async(const ::Test::AMD_Initial_opIntPtr& cb,
                      const IceUtil::Optional< ::Ice::Int>& p1,
                      const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opLong_async(const ::Test::AMD_Initial_opLongPtr& cb,
                       const IceUtil::Optional< ::Ice::Long>& p1,
                       const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opFloat_async(const ::Test::AMD_Initial_opFloatPtr& cb,
                        const IceUtil::Optional< ::Ice::Float>& p1,
                        const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opDouble_async(const ::Test::AMD_Initial_opDoublePtr& cb,
                         const IceUtil::Optional< ::Ice::Double>& p1,
                         const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opString_async(const ::Test::AMD_Initial_opStringPtr& cb,
                         const IceUtil::Optional< ::std::string>& p1,
                         const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opCustomString_async(const ::Test::AMD_Initial_opCustomStringPtr& cb,
                               const IceUtil::Optional< Util::string_view>& p1,
                               const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opMyEnum_async(const ::Test::AMD_Initial_opMyEnumPtr& cb,
                         const IceUtil::Optional<Test::MyEnum>& p1,
                         const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opSmallStruct_async(const ::Test::AMD_Initial_opSmallStructPtr& cb,
                              const IceUtil::Optional<Test::SmallStruct>& p1,
                              const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opFixedStruct_async(const ::Test::AMD_Initial_opFixedStructPtr& cb,
                              const IceUtil::Optional<Test::FixedStruct>& p1,
                              const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opVarStruct_async(const ::Test::AMD_Initial_opVarStructPtr& cb,
                            const IceUtil::Optional<Test::VarStruct>& p1,
                            const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opOneOptional_async(const ::Test::AMD_Initial_opOneOptionalPtr& cb,
                              const IceUtil::Optional< Test::OneOptionalPtr>& p1,
                              const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opOneOptionalProxy_async(const ::Test::AMD_Initial_opOneOptionalProxyPtr& cb,
                                   const IceUtil::Optional< Test::OneOptionalPrx>& p1,
                                   const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opByteSeq_async(const ::Test::AMD_Initial_opByteSeqPtr& cb,
                          const IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >& p1,
                          const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opBoolSeq_async(const ::Test::AMD_Initial_opBoolSeqPtr& cb,
                          const IceUtil::Optional< ::std::pair<const bool*, const bool*> >& p1,
                          const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opShortSeq_async(const ::Test::AMD_Initial_opShortSeqPtr& cb,
                           const IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >& p1,
                           const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opIntSeq_async(const ::Test::AMD_Initial_opIntSeqPtr& cb,
                         const IceUtil::Optional< ::std::pair<const ::Ice::Int*, const ::Ice::Int*> >& p1,
                         const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opLongSeq_async(const ::Test::AMD_Initial_opLongSeqPtr& cb,
                          const IceUtil::Optional< ::std::pair<const ::Ice::Long*, const ::Ice::Long*> >& p1,
                          const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opFloatSeq_async(const ::Test::AMD_Initial_opFloatSeqPtr& cb,
                           const IceUtil::Optional< ::std::pair<const ::Ice::Float*, const ::Ice::Float*> >& p1,
                           const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opDoubleSeq_async(const ::Test::AMD_Initial_opDoubleSeqPtr& cb,
                            const IceUtil::Optional< ::std::pair<const ::Ice::Double*, const ::Ice::Double*> >& p1,
                            const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opStringSeq_async(const ::Test::AMD_Initial_opStringSeqPtr& cb,
                            const IceUtil::Optional< ::std::pair< ::Test::StringSeq::const_iterator,
                                                                  ::Test::StringSeq::const_iterator> >& p1,
                            const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opSmallStructSeq_async(const ::Test::AMD_Initial_opSmallStructSeqPtr& cb,
                                 const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*,
                                                                      const ::Test::SmallStruct*> >& p1,
                                 const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opSmallStructList_async(const ::Test::AMD_Initial_opSmallStructListPtr& cb,
                                  const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*,
                                                                       const ::Test::SmallStruct*> >& p1,
                                  const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opFixedStructSeq_async(const ::Test::AMD_Initial_opFixedStructSeqPtr& cb,
                                 const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*,
                                                                      const ::Test::FixedStruct*> >& p1,
                                 const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opFixedStructList_async(const ::Test::AMD_Initial_opFixedStructListPtr& cb,
                                  const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*,
                                                                       const ::Test::FixedStruct*> >& p1,
                                  const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opVarStructSeq_async(const ::Test::AMD_Initial_opVarStructSeqPtr& cb,
                               const IceUtil::Optional< ::std::pair< ::Test::VarStructSeq::const_iterator,
                                                                     ::Test::VarStructSeq::const_iterator> >& p1,
                               const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opSerializable_async(const ::Test::AMD_Initial_opSerializablePtr& cb,
                               const IceUtil::Optional< ::Test::Serializable>& p1,
                               const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opIntIntDict_async(const ::Test::AMD_Initial_opIntIntDictPtr& cb,
                             const IceUtil::Optional< ::Test::IntIntDict>& p1,
                             const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opStringIntDict_async(const ::Test::AMD_Initial_opStringIntDictPtr& cb,
                                const IceUtil::Optional< ::Test::StringIntDict>& p1,
                                const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opIntOneOptionalDict_async(const ::Test::AMD_Initial_opIntOneOptionalDictPtr& cb,
                                     const IceUtil::Optional< ::Test::IntOneOptionalDict>& p1,
                                     const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opCustomIntStringDict_async(const ::Test::AMD_Initial_opCustomIntStringDictPtr& cb,
                                      const IceUtil::Optional<std::map<int, Util::string_view> >& p1,
                                      const ::Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opClassAndUnknownOptional_async(const ::Test::AMD_Initial_opClassAndUnknownOptionalPtr& cb,
                                          const Test::APtr&,
                                          const Ice::Current&)
{
    cb->ice_response();
}

void
InitialI::sendOptionalClass_async(const ::Test::AMD_Initial_sendOptionalClassPtr& cb,
                                  bool,
                                  const IceUtil::Optional<Test::OneOptionalPtr>&,
                                  const Ice::Current&)
{
    cb->ice_response();
}

void
InitialI::returnOptionalClass_async(const ::Test::AMD_Initial_returnOptionalClassPtr& cb,
                                    bool,
                                    const Ice::Current&)
{
    Optional<OneOptionalPtr> o = new OneOptional(53);
    cb->ice_response(o);
}

void
InitialI::opG_async(const ::Test::AMD_Initial_opGPtr& cb,
                    const ::Test::GPtr& g,
                    const Ice::Current&)
{
    cb->ice_response(g);
}

void
InitialI::opVoid_async(const ::Test::AMD_Initial_opVoidPtr& cb,
                       const Ice::Current&)
{
    cb->ice_response();
}

void
InitialI::opMStruct1_async(const Test::AMD_Initial_opMStruct1Ptr& cb, const Ice::Current&)
{
    cb->ice_response(Test::SmallStruct());
}

void
InitialI::opMStruct2_async(const Test::AMD_Initial_opMStruct2Ptr& cb,
                           const IceUtil::Optional< Test::SmallStruct>& p1,
                           const Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opMSeq1_async(const Test::AMD_Initial_opMSeq1Ptr& cb, const Ice::Current&)
{
    cb->ice_response(Test::StringSeq());
}

void
InitialI::opMSeq2_async(const Test::AMD_Initial_opMSeq2Ptr& cb,
                        const IceUtil::Optional< Test::StringSeq>& p1,
                        const Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opMDict1_async(const Test::AMD_Initial_opMDict1Ptr& cb, const Ice::Current&)
{
    cb->ice_response(Test::StringIntDict());
}

void
InitialI::opMDict2_async(const Test::AMD_Initial_opMDict2Ptr& cb,
                         const IceUtil::Optional< Test::StringIntDict>& p1,
                         const Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::opMG1_async(const Test::AMD_Initial_opMG1Ptr& cb, const Ice::Current&)
{
    cb->ice_response(new Test::G());
}

void
InitialI::opMG2_async(const Test::AMD_Initial_opMG2Ptr& cb,
                      const IceUtil::Optional<Test::GPtr>& p1,
                      const Ice::Current&)
{
    cb->ice_response(p1, p1);
}

void
InitialI::supportsRequiredParams_async(const ::Test::AMD_Initial_supportsRequiredParamsPtr& cb,
                                       const Ice::Current&)
{
    cb->ice_response(false);
}

void
InitialI::supportsJavaSerializable_async(const ::Test::AMD_Initial_supportsJavaSerializablePtr& cb,
                                         const Ice::Current&)
{
    cb->ice_response(true);
}

void
InitialI::supportsCsharpSerializable_async(const ::Test::AMD_Initial_supportsCsharpSerializablePtr& cb,
                                           const Ice::Current&)
{
    cb->ice_response(true);
}

void
InitialI::supportsCppStringView_async(const ::Test::AMD_Initial_supportsCppStringViewPtr& cb,
                                      const Ice::Current&)
{
    cb->ice_response(true);
}

void
InitialI::supportsNullOptional_async(const ::Test::AMD_Initial_supportsNullOptionalPtr& cb, const Ice::Current&)
{
    cb->ice_response(true);
}

#endif
