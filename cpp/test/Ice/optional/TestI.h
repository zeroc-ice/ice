// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>


class InitialI : public Test::Initial
{
public:

    InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual Ice::ObjectPtr pingPong(const Ice::ObjectPtr&, const Ice::Current&);

    virtual void opOptionalException(const IceUtil::Optional< ::Ice::Int>&,
                                     const IceUtil::Optional< ::std::string>&,
                                     const IceUtil::Optional<Test::OneOptionalPtr>&,
                                     const Ice::Current&);

    virtual void opDerivedException(const IceUtil::Optional< ::Ice::Int>&,
                                    const IceUtil::Optional< ::std::string>&,
                                    const IceUtil::Optional<Test::OneOptionalPtr>&,
                                    const Ice::Current&);

    virtual void opRequiredException(const IceUtil::Optional< ::Ice::Int>&,
                                     const IceUtil::Optional< ::std::string>&,
                                     const IceUtil::Optional<Test::OneOptionalPtr>&,
                                     const Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Byte> opByte(const IceUtil::Optional< ::Ice::Byte>&,
                                                   IceUtil::Optional< ::Ice::Byte>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional< bool> opBool(const IceUtil::Optional< bool>&, IceUtil::Optional< bool>&,
                                            const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Short> opShort(const IceUtil::Optional< ::Ice::Short>&,
                                                     IceUtil::Optional< ::Ice::Short>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Int> opInt(const IceUtil::Optional< ::Ice::Int>&,
                                                 IceUtil::Optional< ::Ice::Int>&,
                                                 const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Long> opLong(const IceUtil::Optional< ::Ice::Long>&,
                                                   IceUtil::Optional< ::Ice::Long>&,
                                                   const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Float> opFloat(const IceUtil::Optional< ::Ice::Float>&,
                                                     IceUtil::Optional< ::Ice::Float>&,
                                                     const ::Ice::Current&);

    virtual IceUtil::Optional< ::Ice::Double> opDouble(const IceUtil::Optional< ::Ice::Double>&,
                                                       IceUtil::Optional< ::Ice::Double>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opString(const IceUtil::Optional< ::std::string>&,
                                                       IceUtil::Optional< ::std::string>&,
                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::std::string> opCustomString(const IceUtil::Optional< Util::string_view>&,
                                                               IceUtil::Optional< ::std::string>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional< Test::MyEnum> opMyEnum(const IceUtil::Optional<Test::MyEnum>&,
                                                      IceUtil::Optional<Test::MyEnum>&,
                                                      const ::Ice::Current&);

    virtual IceUtil::Optional<Test::SmallStruct> opSmallStruct(const IceUtil::Optional<Test::SmallStruct>&,
                                                               IceUtil::Optional<Test::SmallStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::FixedStruct> opFixedStruct(const IceUtil::Optional<Test::FixedStruct>&,
                                                               IceUtil::Optional<Test::FixedStruct>&,
                                                               const ::Ice::Current&);

    virtual IceUtil::Optional<Test::VarStruct> opVarStruct(const IceUtil::Optional<Test::VarStruct>&,
                                                           IceUtil::Optional<Test::VarStruct>&,
                                                           const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPtr> opOneOptional(const IceUtil::Optional< Test::OneOptionalPtr>&,
                                                                  IceUtil::Optional< Test::OneOptionalPtr>&,
                                                                  const ::Ice::Current&);

    virtual IceUtil::Optional<Test::OneOptionalPrx> opOneOptionalProxy(const IceUtil::Optional< Test::OneOptionalPrx>&,
                                                                       IceUtil::Optional< Test::OneOptionalPrx>&,
                                                                       const ::Ice::Current&);

    virtual IceUtil::Optional< ::Test::ByteSeq> opByteSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> >&,
        IceUtil::Optional< ::Test::ByteSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::BoolSeq> opBoolSeq(
        const IceUtil::Optional< ::std::pair<const bool*, const bool*> >&,
        IceUtil::Optional< ::Test::BoolSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::ShortSeq> opShortSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Short*, const ::Ice::Short*> >&,
        IceUtil::Optional< ::Test::ShortSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::IntSeq> opIntSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Int*, const ::Ice::Int*> >&,
        IceUtil::Optional< ::Test::IntSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::LongSeq> opLongSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Long*, const ::Ice::Long*> >&,
        IceUtil::Optional< ::Test::LongSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::FloatSeq> opFloatSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Float*, const ::Ice::Float*> >&,
        IceUtil::Optional< ::Test::FloatSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::DoubleSeq> opDoubleSeq(
        const IceUtil::Optional< ::std::pair<const ::Ice::Double*, const ::Ice::Double*> >&,
        IceUtil::Optional< ::Test::DoubleSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::StringSeq> opStringSeq(
        const IceUtil::Optional< ::std::pair< ::Test::StringSeq::const_iterator,
                                              ::Test::StringSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::StringSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::SmallStructSeq> opSmallStructSeq(
        const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >&,
        IceUtil::Optional< ::Test::SmallStructSeq>&, const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::SmallStructList> opSmallStructList(
        const IceUtil::Optional< ::std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*> >&,
        IceUtil::Optional< ::Test::SmallStructList>&, const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::FixedStructSeq> opFixedStructSeq(
        const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >&,
        IceUtil::Optional< ::Test::FixedStructSeq>&, const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::FixedStructList> opFixedStructList(
        const IceUtil::Optional< ::std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*> >&,
        IceUtil::Optional< ::Test::FixedStructList>&, const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::VarStructSeq> opVarStructSeq(
        const IceUtil::Optional< ::std::pair< ::Test::VarStructSeq::const_iterator,
                                              ::Test::VarStructSeq::const_iterator> >&,
        IceUtil::Optional< ::Test::VarStructSeq>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::Serializable> opSerializable(
        const IceUtil::Optional< ::Test::Serializable>&,
        IceUtil::Optional< ::Test::Serializable>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::IntIntDict> opIntIntDict(
        const IceUtil::Optional< ::Test::IntIntDict>&,
        IceUtil::Optional< ::Test::IntIntDict>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::StringIntDict> opStringIntDict(
        const IceUtil::Optional< ::Test::StringIntDict>&,
        IceUtil::Optional< ::Test::StringIntDict>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual IceUtil::Optional< ::Test::IntStringDict> opCustomIntStringDict(
        const IceUtil::Optional<std::map<int, Util::string_view> >&,
        IceUtil::Optional< ::Test::IntStringDict>&,
        const ::Ice::Current& = ::Ice::Current());

    virtual void opClassAndUnknownOptional(const Test::APtr&, const Ice::Current&);

    virtual void sendOptionalClass(bool, const IceUtil::Optional<Test::OneOptionalPtr>&, const Ice::Current&);

    virtual void returnOptionalClass(bool, IceUtil::Optional<Test::OneOptionalPtr>&, const Ice::Current&);

    virtual ::Test::GPtr opG(const ::Test::GPtr& g, const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

    virtual bool supportsRequiredParams(const Ice::Current&);

    virtual bool supportsJavaSerializable(const Ice::Current&);

    virtual bool supportsCsharpSerializable(const Ice::Current&);

    virtual bool supportsCppStringView(const Ice::Current&);
};

#endif
