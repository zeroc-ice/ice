// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : public virtual Test::TestIntf
{
public:

    virtual Test::DoubleSeq opDoubleArray(ICE_IN(std::pair<const Ice::Double*, const Ice::Double*>),
                                          Test::DoubleSeq&,
                                          const Ice::Current&);

    virtual Test::BoolSeq opBoolArray(ICE_IN(std::pair<const bool*, const bool*>),
                                      Test::BoolSeq&,
                                      const Ice::Current&);

    virtual Test::ByteList opByteArray(ICE_IN(std::pair<const Ice::Byte*, const Ice::Byte*>),
                                       Test::ByteList&,
                                       const Ice::Current&);

    virtual Test::VariableList opVariableArray(ICE_IN(std::pair<const Test::Variable*, const Test::Variable*>),
                                               Test::VariableList&,
                                               const Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual Test::BoolSeq opBoolRange(Test::BoolSeq, Test::BoolSeq&, const Ice::Current&);

    virtual Test::ByteList opByteRange(Test::ByteList, Test::ByteList&, const Ice::Current&);

    virtual Test::VariableList
    opVariableRange(Test::VariableList, Test::VariableList&, const Ice::Current&);

    virtual Test::ByteList opByteRangeType(Test::ByteList, Test::ByteList&, const Ice::Current&);

    virtual Test::VariableList
    opVariableRangeType(Test::VariableList, Test::VariableList&, const Ice::Current&);

#else
    virtual Test::BoolSeq opBoolRange(const std::pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>&,
                                      Test::BoolSeq&,
                                      const Ice::Current&);

    virtual Test::ByteList opByteRange(const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>&,
                                      Test::ByteList&,
                                      const Ice::Current&);

    virtual Test::VariableList
    opVariableRange(const std::pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator>&,
                    Test::VariableList&,
                    const Ice::Current&);

    virtual Test::ByteList opByteRangeType(const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>&,
                                           Test::ByteList&,
                                           const Ice::Current&);

    virtual Test::VariableList
    opVariableRangeType(const std::pair<std::deque<Test::Variable>::const_iterator, std::deque<Test::Variable>::const_iterator>&,
                        Test::VariableList&,
                        const Ice::Current&);
#endif

    virtual std::deque<bool> opBoolSeq(ICE_IN(std::deque<bool>),
                                       std::deque<bool>&,
                                       const Ice::Current&);

    virtual std::list<bool> opBoolList(ICE_IN(std::list<bool>),
                                       std::list<bool>&,
                                       const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeList(ICE_IN(::Test::BoolDequeList),
                                                  ::Test::BoolDequeList&,
                                                  const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeListArray(ICE_IN(::std::pair<const std::deque<bool>*, const std::deque<bool>*>),
                                                       ::Test::BoolDequeList&,
                                                       const ::Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual ::Test::BoolDequeList opBoolDequeListRange(::Test::BoolDequeList,
                                                       ::Test::BoolDequeList&, const ::Ice::Current&);

#else
    virtual ::Test::BoolDequeList opBoolDequeListRange(
        const ::std::pair< ::Test::BoolDequeList::const_iterator, ::Test::BoolDequeList::const_iterator>&,
        ::Test::BoolDequeList&,
        const ::Ice::Current&);

#endif

    virtual std::deque< ::Ice::Byte> opByteSeq(ICE_IN(std::deque< ::Ice::Byte>),
                                               std::deque< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual std::list< ::Ice::Byte> opByteList(ICE_IN(std::list< ::Ice::Byte>),
                                               std::list< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual MyByteSeq opMyByteSeq(ICE_IN(MyByteSeq),
                                  MyByteSeq&,
                                  const Ice::Current&);

    virtual std::string opString(ICE_IN(Util::string_view),
                                 std::string&,
                                 const Ice::Current&);

    virtual std::deque< ::std::string> opStringSeq(ICE_IN(std::deque< ::std::string>),
                                                   std::deque< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::list< ::std::string> opStringList(ICE_IN(std::list< ::std::string>),
                                                   std::list< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::deque< ::Test::Fixed> opFixedSeq(ICE_IN(std::deque< ::Test::Fixed>),
                                                  std::deque< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::list< ::Test::Fixed> opFixedList(ICE_IN(std::list< ::Test::Fixed>),
                                                  std::list< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::deque< ::Test::Variable> opVariableSeq(ICE_IN(std::deque< ::Test::Variable>),
                                                        std::deque< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::list< ::Test::Variable> opVariableList(ICE_IN(std::list< ::Test::Variable>),
                                                        std::list< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::deque< ::Test::StringStringDict> opStringStringDictSeq(ICE_IN(std::deque< ::Test::StringStringDict>),
                                                                        std::deque< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::list< ::Test::StringStringDict> opStringStringDictList(ICE_IN(std::list< ::Test::StringStringDict>),
                                                                        std::list< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::deque< ::Test::E> opESeq(ICE_IN(std::deque< ::Test::E>),
                                          std::deque< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::list< ::Test::E> opEList(ICE_IN(std::list< ::Test::E>),
                                          std::list< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::deque< ::Test::DPrxPtr> opDPrxSeq(ICE_IN(std::deque< ::Test::DPrxPtr>),
                                                std::deque< ::Test::DPrxPtr>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::DPrxPtr> opDPrxList(ICE_IN(std::list< ::Test::DPrxPtr>),
                                                std::list< ::Test::DPrxPtr>&,
                                                const Ice::Current&);

    virtual std::deque< ::Test::CPtr> opCSeq(ICE_IN(std::deque< ::Test::CPtr>),
                                                std::deque< ::Test::CPtr>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::CPtr> opCList(ICE_IN(std::list< ::Test::CPtr>),
                                                std::list< ::Test::CPtr>&,
                                                const Ice::Current&);

#ifdef ICE_CPP11_MAPPING
    virtual Test::ClassStruct opClassStruct(Test::ClassStruct,
                                            Test::ClassStructSeq,
                                            Test::ClassStruct& out,
                                            Test::ClassStructSeq& outSeq,
                                            const Ice::Current&);
#else
    virtual ::Test::ClassStructPtr opClassStruct(const ::Test::ClassStructPtr&,
                                                 const ::Test::ClassStructSeq&,
                                                 ::Test::ClassStructPtr& out,
                                                 ::Test::ClassStructSeq& outSeq,
                                                 const Ice::Current&);
#endif

    virtual void opOutArrayByteSeq(ICE_IN(Test::ByteSeq), Test::ByteSeq&, const Ice::Current&);

    virtual void opOutRangeByteSeq(ICE_IN(Test::ByteSeq), Test::ByteSeq&, const Ice::Current&);

    virtual Test::IntStringDict opIntStringDict(ICE_IN(Test::IntStringDict), Test::IntStringDict&,
                                                const Ice::Current&);

    virtual Test::CustomMap<Ice::Long, Ice::Long> opVarDict(ICE_IN(Test::CustomMap<std::string, Ice::Int>),
                                                            Test::CustomMap<std::string, Ice::Int>&,
                                                            const Ice::Current&);

    virtual Test::CustomMap<Ice::Int, std::string> opCustomIntStringDict(
        ICE_IN(std::map<Ice::Int, Util::string_view>), Test::CustomMap<Ice::Int, std::string>&, const Ice::Current&);

    Test::ShortBuffer opShortBuffer(ICE_IN(Test::ShortBuffer), Test::ShortBuffer&, const Ice::Current&);

    Test::CustomBuffer<bool> opBoolBuffer(ICE_IN(Test::CustomBuffer<bool>), Test::CustomBuffer<bool>&,
                                          const Ice::Current&);

    Test::BufferStruct opBufferStruct(ICE_IN(Test::BufferStruct), const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

#endif
