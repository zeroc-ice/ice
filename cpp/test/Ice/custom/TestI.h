//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : public virtual Test::TestIntf
{
public:

    virtual Test::DoubleSeq opDoubleArray(std::pair<const Ice::Double*, const Ice::Double*>,
                                          Test::DoubleSeq&,
                                          const Ice::Current&);

    virtual Test::BoolSeq opBoolArray(std::pair<const bool*, const bool*>,
                                      Test::BoolSeq&,
                                      const Ice::Current&);

    virtual Test::ByteList opByteArray(std::pair<const Ice::Byte*, const Ice::Byte*>,
                                       Test::ByteList&,
                                       const Ice::Current&);

    virtual Test::VariableList opVariableArray(std::pair<const Test::Variable*, const Test::Variable*>,
                                               Test::VariableList&,
                                               const Ice::Current&);

    virtual std::deque<bool> opBoolSeq(std::deque<bool>,
                                       std::deque<bool>&,
                                       const Ice::Current&);

    virtual std::list<bool> opBoolList(std::list<bool>,
                                       std::list<bool>&,
                                       const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeList(::Test::BoolDequeList,
                                                  ::Test::BoolDequeList&,
                                                  const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeListArray(::std::pair<const std::deque<bool>*, const std::deque<bool>*>,
                                                       ::Test::BoolDequeList&,
                                                       const ::Ice::Current&);

    virtual std::deque< ::Ice::Byte> opByteSeq(std::deque< ::Ice::Byte>,
                                               std::deque< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual std::list< ::Ice::Byte> opByteList(std::list< ::Ice::Byte>,
                                               std::list< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual MyByteSeq opMyByteSeq(MyByteSeq,
                                  MyByteSeq&,
                                  const Ice::Current&);

    virtual std::string opString(Util::string_view,
                                 std::string&,
                                 const Ice::Current&);

    virtual std::deque< ::std::string> opStringSeq(std::deque< ::std::string>,
                                                   std::deque< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::list< ::std::string> opStringList(std::list< ::std::string>,
                                                   std::list< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::deque< ::Test::Fixed> opFixedSeq(std::deque< ::Test::Fixed>,
                                                  std::deque< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::list< ::Test::Fixed> opFixedList(std::list< ::Test::Fixed>,
                                                  std::list< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::deque< ::Test::Variable> opVariableSeq(std::deque< ::Test::Variable>,
                                                        std::deque< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::list< ::Test::Variable> opVariableList(std::list< ::Test::Variable>,
                                                        std::list< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::deque< ::Test::StringStringDict> opStringStringDictSeq(std::deque< ::Test::StringStringDict>,
                                                                        std::deque< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::list< ::Test::StringStringDict> opStringStringDictList(std::list< ::Test::StringStringDict>,
                                                                        std::list< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::deque< ::Test::E> opESeq(std::deque< ::Test::E>,
                                          std::deque< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::list< ::Test::E> opEList(std::list< ::Test::E>,
                                          std::list< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::deque< ::Test::DPrxPtr> opDPrxSeq(std::deque< ::Test::DPrxPtr>,
                                                std::deque< ::Test::DPrxPtr>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::DPrxPtr> opDPrxList(std::list< ::Test::DPrxPtr>,
                                                std::list< ::Test::DPrxPtr>&,
                                                const Ice::Current&);

    virtual std::deque< ::Test::CPtr> opCSeq(std::deque< ::Test::CPtr>,
                                                std::deque< ::Test::CPtr>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::CPtr> opCList(std::list< ::Test::CPtr>,
                                                std::list< ::Test::CPtr>&,
                                                const Ice::Current&);

    virtual void opOutArrayByteSeq(Test::ByteSeq, Test::ByteSeq&, const Ice::Current&);

    virtual Test::IntStringDict opIntStringDict(Test::IntStringDict, Test::IntStringDict&,
                                                const Ice::Current&);

    virtual Test::CustomMap<Ice::Long, Ice::Long> opVarDict(Test::CustomMap<std::string, Ice::Int>,
                                                            Test::CustomMap<std::string, Ice::Int>&,
                                                            const Ice::Current&);

    virtual Test::CustomMap<Ice::Int, std::string> opCustomIntStringDict(
        std::map<Ice::Int, Util::string_view>, Test::CustomMap<Ice::Int, std::string>&, const Ice::Current&);

    Test::ShortBuffer opShortBuffer(Test::ShortBuffer, Test::ShortBuffer&, const Ice::Current&);

    Test::CustomBuffer<bool> opBoolBuffer(Test::CustomBuffer<bool>, Test::CustomBuffer<bool>&,
                                          const Ice::Current&);

    Test::BufferStruct opBufferStruct(Test::BufferStruct, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

#endif
