//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestAMDI.h>

using namespace Test;
using namespace IceUtil;
using namespace Ice;
using namespace std;

InitialI::InitialI() {}

void
InitialI::shutdownAsync(::std::function<void()> response,
                        ::std::function<void(::std::exception_ptr)>,
                        const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
InitialI::pingPongAsync(::std::shared_ptr<::Ice::Value> obj,
                        ::std::function<void(const ::std::shared_ptr<::Ice::Value>&)> response,
                        ::std::function<void(::std::exception_ptr)>,
                        const Ice::Current&)
{
    response(obj);
    if (dynamic_pointer_cast<MultiOptional>(obj))
    {
        // Break cyclic reference count
        dynamic_pointer_cast<MultiOptional>(obj)->k = shared_ptr<MultiOptional>();
    }
}

void
InitialI::opOptionalExceptionAsync(optional<int> a,
                                   optional<::std::string> b,
                                   optional<::std::shared_ptr<::Test::OneOptional>> o,
                                   ::std::function<void()>,
                                   ::std::function<void(::std::exception_ptr)> ex,
                                   const Ice::Current&)
{
    ex(make_exception_ptr(OptionalException(false, a, b, o)));
}

void
InitialI::opDerivedExceptionAsync(optional<int> a,
                                  optional<::std::string> b,
                                  optional<::std::shared_ptr<::Test::OneOptional>> o,
                                  ::std::function<void()>,
                                  ::std::function<void(::std::exception_ptr)> ex,
                                  const Ice::Current&)
{
    ex(make_exception_ptr(DerivedException(false, a, b, o, "d1", b, o, "d2")));
}

void
InitialI::opRequiredExceptionAsync(optional<int> a,
                                   optional<::std::string> b,
                                   optional<::std::shared_ptr<::Test::OneOptional>> o,
                                   ::std::function<void()>,
                                   ::std::function<void(::std::exception_ptr)> ex,
                                   const Ice::Current&)
{
    RequiredException e;
    e.a = a;
    e.b = b;
    e.o = o;
    if (b)
    {
        e.ss = b.value();
    }
    if (o)
    {
        e.o2 = o.value();
    }

    ex(make_exception_ptr(e));
}

void
InitialI::opByteAsync(optional<::Ice::Byte> p1,
                      ::std::function<void(const optional<::Ice::Byte>&, const optional<::Ice::Byte>&)> response,
                      ::std::function<void(::std::exception_ptr)>,
                      const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolAsync(optional<bool> p1,
                      ::std::function<void(const optional<bool>&, const optional<bool>&)> response,
                      ::std::function<void(::std::exception_ptr)>,
                      const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortAsync(optional<short> p1,
                       ::std::function<void(const optional<short>&, const optional<short>&)> response,
                       ::std::function<void(::std::exception_ptr)>,
                       const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntAsync(optional<int> p1,
                     ::std::function<void(const optional<int>&, const optional<int>&)> response,
                     ::std::function<void(::std::exception_ptr)>,
                     const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongAsync(optional<int64_t> p1,
                      ::std::function<void(const optional<int64_t>&, const optional<int64_t>&)> response,
                      ::std::function<void(::std::exception_ptr)>,
                      const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatAsync(optional<float> p1,
                       ::std::function<void(const optional<float>&, const optional<float>&)> response,
                       ::std::function<void(::std::exception_ptr)>,
                       const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleAsync(optional<double> p1,
                        ::std::function<void(const optional<double>&, const optional<double>&)> response,
                        ::std::function<void(::std::exception_ptr)>,
                        const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringAsync(optional<::std::string> p1,
                        ::std::function<void(const optional<::std::string>&, const optional<::std::string>&)> response,
                        ::std::function<void(::std::exception_ptr)>,
                        const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMyEnumAsync(
    optional<::Test::MyEnum> p1,
    ::std::function<void(const optional<::Test::MyEnum>&, const optional<::Test::MyEnum>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructAsync(
    optional<::Test::SmallStruct> p1,
    ::std::function<void(const optional<::Test::SmallStruct>&, const optional<::Test::SmallStruct>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructAsync(
    optional<::Test::FixedStruct> p1,
    ::std::function<void(const optional<::Test::FixedStruct>&, const optional<::Test::FixedStruct>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructAsync(
    optional<::Test::VarStruct> p1,
    ::std::function<void(const optional<::Test::VarStruct>&, const optional<::Test::VarStruct>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opOneOptionalAsync(optional<::std::shared_ptr<::Test::OneOptional>> p1,
                             ::std::function<void(const optional<::std::shared_ptr<::Test::OneOptional>>&,
                                                  const optional<::std::shared_ptr<::Test::OneOptional>>&)> response,
                             ::std::function<void(::std::exception_ptr)>,
                             const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opMyInterfaceProxyAsync(
    optional<::MyInterfacePrx> p1,
    ::std::function<void(const optional<::MyInterfacePrx>&, const optional<::MyInterfacePrx>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opByteSeqAsync(
    optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>> p1,
    ::std::function<void(const optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&,
                         const optional<::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opBoolSeqAsync(optional<::std::pair<const bool*, const bool*>> p1,
                         ::std::function<void(const optional<::std::pair<const bool*, const bool*>>&,
                                              const optional<::std::pair<const bool*, const bool*>>&)> response,
                         ::std::function<void(::std::exception_ptr)>,
                         const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opShortSeqAsync(optional<::std::pair<const short*, const short*>> p1,
                          ::std::function<void(const optional<::std::pair<const short*, const short*>>&,
                                               const optional<::std::pair<const short*, const short*>>&)> response,
                          ::std::function<void(::std::exception_ptr)>,
                          const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntSeqAsync(optional<::std::pair<const int*, const int*>> p1,
                        ::std::function<void(const optional<::std::pair<const int*, const int*>>&,
                                             const optional<::std::pair<const int*, const int*>>&)> response,
                        ::std::function<void(::std::exception_ptr)>,
                        const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opLongSeqAsync(optional<::std::pair<const int64_t*, const int64_t*>> p1,
                         ::std::function<void(const optional<::std::pair<const int64_t*, const int64_t*>>&,
                                              const optional<::std::pair<const int64_t*, const int64_t*>>&)> response,
                         ::std::function<void(::std::exception_ptr)>,
                         const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFloatSeqAsync(optional<::std::pair<const float*, const float*>> p1,
                          ::std::function<void(const optional<::std::pair<const float*, const float*>>&,
                                               const optional<::std::pair<const float*, const float*>>&)> response,
                          ::std::function<void(::std::exception_ptr)>,
                          const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opDoubleSeqAsync(optional<::std::pair<const double*, const double*>> p1,
                           ::std::function<void(const optional<::std::pair<const double*, const double*>>&,
                                                const optional<::std::pair<const double*, const double*>>&)> response,
                           ::std::function<void(::std::exception_ptr)>,
                           const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringSeqAsync(
    optional<::Test::StringSeq> p1,
    ::std::function<void(const optional<::Test::StringSeq>&, const optional<::Test::StringSeq>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructSeqAsync(
    optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>> p1,
    ::std::function<void(const optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&,
                         const optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>
        response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSmallStructListAsync(
    optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>> p1,
    ::std::function<void(const optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&,
                         const optional<::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>&)>
        response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructSeqAsync(
    optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>> p1,
    ::std::function<void(const optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&,
                         const optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>
        response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opFixedStructListAsync(
    optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>> p1,
    ::std::function<void(const optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&,
                         const optional<::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>&)>
        response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opVarStructSeqAsync(
    optional<::Test::VarStructSeq> p1,
    ::std::function<void(const optional<::Test::VarStructSeq>&, const optional<::Test::VarStructSeq>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opSerializableAsync(
    optional<::Test::Serializable> p1,
    ::std::function<void(const optional<::Test::Serializable>&, const optional<::Test::Serializable>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntIntDictAsync(
    optional<::Test::IntIntDict> p1,
    ::std::function<void(const optional<::Test::IntIntDict>&, const optional<::Test::IntIntDict>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opStringIntDictAsync(
    optional<::Test::StringIntDict> p1,
    ::std::function<void(const optional<::Test::StringIntDict>&, const optional<::Test::StringIntDict>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opIntOneOptionalDictAsync(optional<::Test::IntOneOptionalDict> p1,
                                    ::std::function<void(const optional<::Test::IntOneOptionalDict>&,
                                                         const optional<::Test::IntOneOptionalDict>&)> response,
                                    ::std::function<void(::std::exception_ptr)>,
                                    const Ice::Current&)
{
    response(p1, p1);
}

void
InitialI::opClassAndUnknownOptionalAsync(::std::shared_ptr<::Test::A>,
                                         ::std::function<void()> response,
                                         ::std::function<void(::std::exception_ptr)>,
                                         const Ice::Current&)
{
    response();
}

void
InitialI::sendOptionalClassAsync(bool,
                                 optional<::std::shared_ptr<::Test::OneOptional>>,
                                 ::std::function<void()> response,
                                 ::std::function<void(::std::exception_ptr)>,
                                 const Ice::Current&)
{
    response();
}

void
InitialI::returnOptionalClassAsync(
    bool,
    ::std::function<void(const optional<::std::shared_ptr<::Test::OneOptional>>&)> response,
    ::std::function<void(::std::exception_ptr)>,
    const Ice::Current&)
{
    response(make_shared<OneOptional>(53));
}

void
InitialI::opGAsync(::std::shared_ptr<::Test::G> g,
                   ::std::function<void(const ::std::shared_ptr<::Test::G>&)> response,
                   ::std::function<void(::std::exception_ptr)>,
                   const Ice::Current&)
{
    response(g);
}

void
InitialI::opVoidAsync(::std::function<void()> response,
                      ::std::function<void(::std::exception_ptr)>,
                      const Ice::Current&)
{
    response();
}

void
InitialI::opMStruct1Async(function<void(const OpMStruct1MarshaledResult&)> response,
                          function<void(exception_ptr)>,
                          const Ice::Current& current)
{
    response(OpMStruct1MarshaledResult(Test::SmallStruct(), current));
}

void
InitialI::opMStruct2Async(optional<SmallStruct> p1,
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
InitialI::opMSeq2Async(optional<Test::StringSeq> p1,
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
InitialI::opMDict2Async(optional<StringIntDict> p1,
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
    response(OpMG1MarshaledResult(std::make_shared<G>(), current));
}

void
InitialI::opMG2Async(optional<GPtr> p1,
                     function<void(const OpMG2MarshaledResult&)> response,
                     function<void(exception_ptr)>,
                     const Ice::Current& current)
{
    response(OpMG2MarshaledResult(p1, p1, current));
}

void
InitialI::supportsRequiredParamsAsync(::std::function<void(bool)> response,
                                      ::std::function<void(::std::exception_ptr)>,
                                      const Ice::Current&)
{
    response(false);
}

void
InitialI::supportsJavaSerializableAsync(::std::function<void(bool)> response,
                                        ::std::function<void(::std::exception_ptr)>,
                                        const Ice::Current&)
{
    response(true);
}

void
InitialI::supportsCsharpSerializableAsync(::std::function<void(bool)> response,
                                          ::std::function<void(::std::exception_ptr)>,
                                          const Ice::Current&)
{
    response(true);
}

void
InitialI::supportsNullOptionalAsync(::std::function<void(bool)> response,
                                    ::std::function<void(::std::exception_ptr)>,
                                    const Ice::Current&)
{
    response(true);
}
