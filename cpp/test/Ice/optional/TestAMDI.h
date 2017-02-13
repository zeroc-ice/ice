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

#include <TestAMD.h>


class InitialI : public Test::Initial
{
public:

    InitialI();

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

    virtual void supportsRequiredParams_async(const ::Test::AMD_Initial_supportsRequiredParamsPtr&,
                                              const Ice::Current&);

    virtual void supportsJavaSerializable_async(const ::Test::AMD_Initial_supportsJavaSerializablePtr&,
                                                const Ice::Current&);

    virtual void supportsCsharpSerializable_async(const ::Test::AMD_Initial_supportsCsharpSerializablePtr&,
                                                  const Ice::Current&);

    virtual void supportsCppStringView_async(const ::Test::AMD_Initial_supportsCppStringViewPtr&,
                                             const Ice::Current&);
};

#endif
