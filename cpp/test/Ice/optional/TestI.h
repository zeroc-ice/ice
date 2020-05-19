//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

namespace Test
{
using OneOptionalPrxPtr = std::shared_ptr<Ice::ObjectPrx>;
}

class InitialI : public Test::Initial
{
public:

   InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual PingPongMarshaledResult pingPong(Ice::ValuePtr, const Ice::Current&);

    virtual void opOptionalException(IceUtil::Optional< ::Ice::Int>,
                                     IceUtil::Optional< ::std::string>,
                                     IceUtil::Optional<Test::OneOptionalPtr>,
                                     const Ice::Current&);

    virtual void opDerivedException(IceUtil::Optional< ::Ice::Int>,
                                    IceUtil::Optional< ::std::string>,
                                    IceUtil::Optional<Test::OneOptionalPtr>,
                                    const Ice::Current&);

    virtual void opRequiredException(IceUtil::Optional< ::Ice::Int>,
                                     IceUtil::Optional< ::std::string>,
                                     IceUtil::Optional<Test::OneOptionalPtr>,
                                     const Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Byte> opByte(IceUtil::Optional< ::Ice::Byte>,
                                                   IceUtil::Optional< ::Ice::Byte>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional<bool> opBool(IceUtil::Optional<bool>, IceUtil::Optional<bool>&,
                                           const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Short> opShort(IceUtil::Optional< ::Ice::Short>,
                                                     IceUtil::Optional< ::Ice::Short>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Int> opInt(IceUtil::Optional< ::Ice::Int>,
                                                 IceUtil::Optional< ::Ice::Int>&,
                                                 const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Long> opLong(IceUtil::Optional< ::Ice::Long>,
                                                   IceUtil::Optional< ::Ice::Long>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Float> opFloat(IceUtil::Optional< ::Ice::Float>,
                                                     IceUtil::Optional< ::Ice::Float>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Double> opDouble(IceUtil::Optional< ::Ice::Double>,
                                                       IceUtil::Optional< ::Ice::Double>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opString(IceUtil::Optional< ::std::string>,
                                                       IceUtil::Optional< ::std::string>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opCustomString(IceUtil::Optional< Util::string_view>,
                                                               IceUtil::Optional< ::std::string>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional< Test::MyEnum> opMyEnum(IceUtil::Optional<Test::MyEnum>,
                                                      IceUtil::Optional<Test::MyEnum>&,
                                                      const ::Ice::Current&);

    virtual IceUtil::Optional<Test::SmallStruct> opSmallStruct(IceUtil::Optional<Test::SmallStruct>,
                                                               IceUtil::Optional<Test::SmallStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::FixedStruct> opFixedStruct(IceUtil::Optional<Test::FixedStruct>,
                                                               IceUtil::Optional<Test::FixedStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::VarStruct> opVarStruct(IceUtil::Optional<Test::VarStruct>,
                                                           IceUtil::Optional<Test::VarStruct>&,
                                                           const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPtr> opOneOptional(IceUtil::Optional< Test::OneOptionalPtr>,
                                                                  IceUtil::Optional< Test::OneOptionalPtr>&,
                                                                  const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::ByteSeq> opByteSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >,
        IceUtil::Optional< ::Test::ByteSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::BoolSeq> opBoolSeq(
        IceUtil::Optional< ::std::pair<const bool*, const bool*> >,
        IceUtil::Optional< ::Test::BoolSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::ShortSeq> opShortSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >,
        IceUtil::Optional< ::Test::ShortSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntSeq> opIntSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Int*, const ::Ice::Int*> >,
        IceUtil::Optional< ::Test::IntSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::LongSeq> opLongSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Long*, const ::Ice::Long*> >,
        IceUtil::Optional< ::Test::LongSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FloatSeq> opFloatSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Float*, const ::Ice::Float*> >,
        IceUtil::Optional< ::Test::FloatSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::DoubleSeq> opDoubleSeq(
        IceUtil::Optional< ::std::pair<const ::Ice::Double*, const ::Ice::Double*> >,
        IceUtil::Optional< ::Test::DoubleSeq>&,
        const ::Ice::Current&);

     virtual Ice::optional<::Test::StringSeq> opStringSeq(
         Ice::optional<::Test::StringSeq>,
         Ice::optional<::Test::StringSeq>&, const ::Ice::Current&) ;

    virtual IceUtil::Optional< ::Test::SmallStructSeq> opSmallStructSeq(
        IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >,
        IceUtil::Optional< ::Test::SmallStructSeq>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::SmallStructList> opSmallStructList(
        IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >,
        IceUtil::Optional< ::Test::SmallStructList>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FixedStructSeq> opFixedStructSeq(
        IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >,
        IceUtil::Optional< ::Test::FixedStructSeq>&, const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::FixedStructList> opFixedStructList(
        IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >,
        IceUtil::Optional< ::Test::FixedStructList>&, const ::Ice::Current&);

    virtual Ice::optional<::Test::VarStructSeq> opVarStructSeq(
        Ice::optional<::Test::VarStructSeq>, Ice::optional<::Test::VarStructSeq>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::Serializable> opSerializable(
        IceUtil::Optional< ::Test::Serializable>,
        IceUtil::Optional< ::Test::Serializable>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntIntDict> opIntIntDict(
        IceUtil::Optional< ::Test::IntIntDict>,
        IceUtil::Optional< ::Test::IntIntDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::StringIntDict> opStringIntDict(
        IceUtil::Optional< ::Test::StringIntDict>,
        IceUtil::Optional< ::Test::StringIntDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntOneOptionalDict> opIntOneOptionalDict(
        IceUtil::Optional< ::Test::IntOneOptionalDict>,
        IceUtil::Optional< ::Test::IntOneOptionalDict>&,
        const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::IntStringDict> opCustomIntStringDict(
        IceUtil::Optional<std::map<int, Util::string_view> >,
        IceUtil::Optional< ::Test::IntStringDict>&,
        const ::Ice::Current&);

    virtual void opClassAndUnknownOptional(Test::APtr, const Ice::Current&);

    virtual void sendOptionalClass(bool, IceUtil::Optional<Test::OneOptionalPtr>, const Ice::Current&);

    virtual void returnOptionalClass(bool, IceUtil::Optional<Test::OneOptionalPtr>&, const Ice::Current&);

    virtual ::Test::GPtr opG(::Test::GPtr g, const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

    virtual OpMStruct1MarshaledResult opMStruct1(const Ice::Current&);

    virtual OpMStruct2MarshaledResult opMStruct2(IceUtil::Optional<Test::SmallStruct>, const Ice::Current&);

    virtual OpMSeq1MarshaledResult opMSeq1(const Ice::Current&);

    virtual OpMSeq2MarshaledResult opMSeq2(IceUtil::Optional<Test::StringSeq>, const Ice::Current&);

    virtual OpMDict1MarshaledResult opMDict1(const Ice::Current&);

    virtual OpMDict2MarshaledResult opMDict2(IceUtil::Optional<Test::StringIntDict>, const Ice::Current&);

    virtual OpMG1MarshaledResult opMG1(const Ice::Current&);

    virtual OpMG2MarshaledResult opMG2(IceUtil::Optional<Test::GPtr>, const Ice::Current&);

    virtual bool supportsRequiredParams(const Ice::Current&);

    virtual bool supportsJavaSerializable(const Ice::Current&);

    virtual bool supportsCsharpSerializable(const Ice::Current&);

    virtual bool supportsCppStringView(const Ice::Current&);

    virtual bool supportsNullOptional(const Ice::Current&);
};

#endif
