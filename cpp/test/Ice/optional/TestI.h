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

#ifdef ICE_CPP11_MAPPING
namespace Test
{
using OneOptionalPrxPtr = std::shared_ptr<Ice::ObjectPrx>;
}
#endif


class InitialI : public Test::Initial
{
public:

   InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual Ice::ValuePtr pingPong(ICE_IN(Ice::ValuePtr), const Ice::Current&);

    virtual void opOptionalException(ICE_IN(IceUtil::Optional< ::Ice::Int>),
                                     ICE_IN(IceUtil::Optional< ::std::string>),
                                     ICE_IN(IceUtil::Optional<Test::OneOptionalPtr>),
                                     const Ice::Current&);

    virtual void opDerivedException(ICE_IN(IceUtil::Optional< ::Ice::Int>),
                                    ICE_IN(IceUtil::Optional< ::std::string>),
                                    ICE_IN(IceUtil::Optional<Test::OneOptionalPtr>),
                                    const Ice::Current&);

    virtual void opRequiredException(ICE_IN(IceUtil::Optional< ::Ice::Int>),
                                     ICE_IN(IceUtil::Optional< ::std::string>),
                                     ICE_IN(IceUtil::Optional<Test::OneOptionalPtr>),
                                     const Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Byte> opByte(ICE_IN(IceUtil::Optional< ::Ice::Byte>),
                                                   IceUtil::Optional< ::Ice::Byte>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional<bool> opBool(ICE_IN(IceUtil::Optional<bool>), IceUtil::Optional<bool>&,
                                           const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Short> opShort(ICE_IN(IceUtil::Optional< ::Ice::Short>),
                                                     IceUtil::Optional< ::Ice::Short>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Int> opInt(ICE_IN(IceUtil::Optional< ::Ice::Int>),
                                                 IceUtil::Optional< ::Ice::Int>&,
                                                 const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Long> opLong(ICE_IN(IceUtil::Optional< ::Ice::Long>),
                                                   IceUtil::Optional< ::Ice::Long>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Float> opFloat(ICE_IN(IceUtil::Optional< ::Ice::Float>),
                                                     IceUtil::Optional< ::Ice::Float>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Double> opDouble(ICE_IN(IceUtil::Optional< ::Ice::Double>),
                                                       IceUtil::Optional< ::Ice::Double>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opString(ICE_IN(IceUtil::Optional< ::std::string>),
                                                       IceUtil::Optional< ::std::string>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opCustomString(ICE_IN(IceUtil::Optional< Util::string_view>),
                                                               IceUtil::Optional< ::std::string>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional< Test::MyEnum> opMyEnum(ICE_IN(IceUtil::Optional<Test::MyEnum>),
                                                      IceUtil::Optional<Test::MyEnum>&,
                                                      const ::Ice::Current&);

    virtual IceUtil::Optional<Test::SmallStruct> opSmallStruct(ICE_IN(IceUtil::Optional<Test::SmallStruct>),
                                                               IceUtil::Optional<Test::SmallStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::FixedStruct> opFixedStruct(ICE_IN(IceUtil::Optional<Test::FixedStruct>),
                                                               IceUtil::Optional<Test::FixedStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::VarStruct> opVarStruct(ICE_IN(IceUtil::Optional<Test::VarStruct>),
                                                           IceUtil::Optional<Test::VarStruct>&,
                                                           const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPtr> opOneOptional(ICE_IN(IceUtil::Optional< Test::OneOptionalPtr>),
                                                                  IceUtil::Optional< Test::OneOptionalPtr>&,
                                                                  const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPrxPtr> opOneOptionalProxy(ICE_IN(IceUtil::Optional< Test::OneOptionalPrxPtr>),
                                                                          IceUtil::Optional< Test::OneOptionalPrxPtr>&,
                                                                          const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::ByteSeq> opByteSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >),
        IceUtil::Optional< ::Test::ByteSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::BoolSeq> opBoolSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const bool*, const bool*> >),
        IceUtil::Optional< ::Test::BoolSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::ShortSeq> opShortSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >),
        IceUtil::Optional< ::Test::ShortSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntSeq> opIntSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Int*, const ::Ice::Int*> >),
        IceUtil::Optional< ::Test::IntSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::LongSeq> opLongSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Long*, const ::Ice::Long*> >),
        IceUtil::Optional< ::Test::LongSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FloatSeq> opFloatSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Float*, const ::Ice::Float*> >),
        IceUtil::Optional< ::Test::FloatSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::DoubleSeq> opDoubleSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Ice::Double*, const ::Ice::Double*> >),
        IceUtil::Optional< ::Test::DoubleSeq>&,
        const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
     virtual Ice::optional<::Test::StringSeq> opStringSeq(
         Ice::optional<::Test::StringSeq>,
         Ice::optional<::Test::StringSeq>&, const ::Ice::Current&) ;
#else
    virtual IceUtil::Optional< ::Test::StringSeq> opStringSeq(
        const IceUtil::Optional< ::std::pair< ::Test::StringSeq::const_iterator,
                                              ::Test::StringSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::StringSeq>&,
        const ::Ice::Current&);
#endif

    virtual IceUtil::Optional< ::Test::SmallStructSeq> opSmallStructSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >),
        IceUtil::Optional< ::Test::SmallStructSeq>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::SmallStructList> opSmallStructList(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >),
        IceUtil::Optional< ::Test::SmallStructList>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FixedStructSeq> opFixedStructSeq(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >),
        IceUtil::Optional< ::Test::FixedStructSeq>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FixedStructList> opFixedStructList(
        ICE_IN(IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >),
        IceUtil::Optional< ::Test::FixedStructList>&, const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual Ice::optional<::Test::VarStructSeq> opVarStructSeq(
        Ice::optional<::Test::VarStructSeq>, Ice::optional<::Test::VarStructSeq>&,
        const ::Ice::Current&);
#else
    virtual IceUtil::Optional< ::Test::VarStructSeq> opVarStructSeq(
        const IceUtil::Optional< ::std::pair< ::Test::VarStructSeq::const_iterator,
                                              ::Test::VarStructSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::VarStructSeq>&,
        const ::Ice::Current&);
#endif

    virtual IceUtil::Optional< ::Test::Serializable> opSerializable(
        ICE_IN(IceUtil::Optional< ::Test::Serializable>),
        IceUtil::Optional< ::Test::Serializable>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntIntDict> opIntIntDict(
        ICE_IN(IceUtil::Optional< ::Test::IntIntDict>),
        IceUtil::Optional< ::Test::IntIntDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::StringIntDict> opStringIntDict(
        ICE_IN(IceUtil::Optional< ::Test::StringIntDict>),
        IceUtil::Optional< ::Test::StringIntDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntOneOptionalDict> opIntOneOptionalDict(
        ICE_IN(IceUtil::Optional< ::Test::IntOneOptionalDict>),
        IceUtil::Optional< ::Test::IntOneOptionalDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntStringDict> opCustomIntStringDict(
        ICE_IN(IceUtil::Optional<std::map<int, Util::string_view> >),
        IceUtil::Optional< ::Test::IntStringDict>&,
        const ::Ice::Current&);

    virtual void opClassAndUnknownOptional(ICE_IN(Test::APtr), const Ice::Current&);

    virtual void sendOptionalClass(bool, ICE_IN(IceUtil::Optional<Test::OneOptionalPtr>), const Ice::Current&);

    virtual void returnOptionalClass(bool, IceUtil::Optional<Test::OneOptionalPtr>&, const Ice::Current&);

    virtual ::Test::GPtr opG(ICE_IN(::Test::GPtr) g, const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual OpMStruct1MarshaledResult opMStruct1(const Ice::Current&);

    virtual OpMStruct2MarshaledResult opMStruct2(ICE_IN(IceUtil::Optional<Test::SmallStruct>), const Ice::Current&);

    virtual OpMSeq1MarshaledResult opMSeq1(const Ice::Current&);

    virtual OpMSeq2MarshaledResult opMSeq2(ICE_IN(IceUtil::Optional<Test::StringSeq>), const Ice::Current&);

    virtual OpMDict1MarshaledResult opMDict1(const Ice::Current&);

    virtual OpMDict2MarshaledResult opMDict2(ICE_IN(IceUtil::Optional<Test::StringIntDict>), const Ice::Current&);

    virtual OpMG1MarshaledResult opMG1(const Ice::Current&);

    virtual OpMG2MarshaledResult opMG2(ICE_IN(IceUtil::Optional<Test::GPtr>), const Ice::Current&);
#else
    virtual IceUtil::Optional<Test::SmallStruct> opMStruct1(const Ice::Current&);

    virtual IceUtil::Optional<Test::SmallStruct> opMStruct2(ICE_IN(IceUtil::Optional<Test::SmallStruct>),
                                                            IceUtil::Optional<Test::SmallStruct>&,
                                                            const Ice::Current&);

    virtual IceUtil::Optional<Test::StringSeq> opMSeq1(const Ice::Current&);

    virtual IceUtil::Optional<Test::StringSeq> opMSeq2(ICE_IN(IceUtil::Optional<Test::StringSeq>),
                                                       IceUtil::Optional<Test::StringSeq>&,
                                                       const Ice::Current&);

    virtual IceUtil::Optional<Test::StringIntDict> opMDict1(const Ice::Current&);

    virtual IceUtil::Optional<Test::StringIntDict> opMDict2(ICE_IN(IceUtil::Optional<Test::StringIntDict>),
                                                            IceUtil::Optional<Test::StringIntDict>&,
                                                            const Ice::Current&);

    virtual IceUtil::Optional<Test::GPtr> opMG1(const Ice::Current&);

    virtual IceUtil::Optional<Test::GPtr> opMG2(ICE_IN(IceUtil::Optional<Test::GPtr>),
                                                IceUtil::Optional<Test::GPtr>&,
                                                const Ice::Current&);
#endif

    virtual bool supportsRequiredParams(const Ice::Current&);

    virtual bool supportsJavaSerializable(const Ice::Current&);

    virtual bool supportsCsharpSerializable(const Ice::Current&);

    virtual bool supportsCppStringView(const Ice::Current&);

    virtual bool supportsNullOptional(const Ice::Current&);
};

#endif
