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

class TestIntfI : virtual public Test::TestIntf
{
public:

    TestIntfI(const Ice::CommunicatorPtr&);

    virtual Test::DoubleSeq opDoubleArray(const std::pair<const Ice::Double*, const Ice::Double*>&,
                                          Test::DoubleSeq&,
                                          const Ice::Current&);

    virtual Test::BoolSeq opBoolArray(const std::pair<const bool*, const bool*>&,
                                      Test::BoolSeq&,
                                      const Ice::Current&);

    virtual Test::ByteList opByteArray(const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                       Test::ByteList&,
                                       const Ice::Current&);

    virtual Test::VariableList opVariableArray(const std::pair<const Test::Variable*, const Test::Variable*>&,
                                               Test::VariableList&,
                                               const Ice::Current&);

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

    virtual Test::BoolSeq opBoolRangeType(const std::pair<const bool*, const bool*>&,
                                          Test::BoolSeq&,
                                          const Ice::Current&);

    virtual Test::ByteList opByteRangeType(const std::pair<Test::ByteList::const_iterator, 
                                                           Test::ByteList::const_iterator>&,
                                           Test::ByteList&,
                                           const Ice::Current&);

    virtual Test::VariableList 
    opVariableRangeType(const std::pair<std::deque<Test::Variable>::const_iterator,
                                        std::deque<Test::Variable>::const_iterator>&,
                        Test::VariableList&,
                        const Ice::Current&);

    virtual std::deque<bool> opBoolSeq(const std::deque<bool>&,
                                       std::deque<bool>&,
                                       const Ice::Current&);

    virtual std::list<bool> opBoolList(const std::list<bool>&,
                                       std::list<bool>&,
                                       const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeList(const ::Test::BoolDequeList&, 
                                                  ::Test::BoolDequeList&, 
                                                  const Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeListArray(const ::std::pair<const std::deque<bool>*, 
                                                                         const std::deque<bool>*>&, 
                                                       ::Test::BoolDequeList&, 
                                                       const ::Ice::Current&);

    virtual ::Test::BoolDequeList opBoolDequeListRange(const ::std::pair< ::Test::BoolDequeList::const_iterator,
                                                                          ::Test::BoolDequeList::const_iterator>&, 
                                                       ::Test::BoolDequeList&, 
                                                       const ::Ice::Current&);

    virtual std::deque< ::Ice::Byte> opByteSeq(const std::deque< ::Ice::Byte>&,
                                               std::deque< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual std::list< ::Ice::Byte> opByteList(const std::list< ::Ice::Byte>&,
                                               std::list< ::Ice::Byte>&,
                                               const Ice::Current&);

    virtual MyByteSeq opMyByteSeq(const MyByteSeq&,
                                  MyByteSeq&,
                                  const Ice::Current&);

    virtual std::string opString(const Util::string_view&,
                                 std::string&,
                                 const Ice::Current&);
                                 

    virtual std::deque< ::std::string> opStringSeq(const std::deque< ::std::string>&,
                                                   std::deque< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::list< ::std::string> opStringList(const std::list< ::std::string>&,
                                                   std::list< ::std::string>&,
                                                   const Ice::Current&);

    virtual std::deque< ::Test::Fixed> opFixedSeq(const std::deque< ::Test::Fixed>&,
                                                  std::deque< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::list< ::Test::Fixed> opFixedList(const std::list< ::Test::Fixed>&,
                                                  std::list< ::Test::Fixed>&,
                                                  const Ice::Current&);

    virtual std::deque< ::Test::Variable> opVariableSeq(const std::deque< ::Test::Variable>&,
                                                        std::deque< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::list< ::Test::Variable> opVariableList(const std::list< ::Test::Variable>&,
                                                        std::list< ::Test::Variable>&,
                                                        const Ice::Current&);

    virtual std::deque< ::Test::StringStringDict> opStringStringDictSeq(const std::deque< ::Test::StringStringDict>&,
                                                                        std::deque< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::list< ::Test::StringStringDict> opStringStringDictList(const std::list< ::Test::StringStringDict>&,
                                                                        std::list< ::Test::StringStringDict>&,
                                                                        const Ice::Current&);

    virtual std::deque< ::Test::E> opESeq(const std::deque< ::Test::E>&,
                                          std::deque< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::list< ::Test::E> opEList(const std::list< ::Test::E>&,
                                          std::list< ::Test::E>&,
                                          const Ice::Current&);

    virtual std::deque< ::Test::CPrx> opCPrxSeq(const std::deque< ::Test::CPrx>&,
                                                std::deque< ::Test::CPrx>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::CPrx> opCPrxList(const std::list< ::Test::CPrx>&,
                                                std::list< ::Test::CPrx>&,
                                                const Ice::Current&);

    virtual std::deque< ::Test::CPtr> opCSeq(const std::deque< ::Test::CPtr>&,
                                                std::deque< ::Test::CPtr>&,
                                                const Ice::Current&);

    virtual std::list< ::Test::CPtr> opCList(const std::list< ::Test::CPtr>&,
                                                std::list< ::Test::CPtr>&,
                                                const Ice::Current&);

    virtual ::Test::ClassStructPtr opClassStruct(const ::Test::ClassStructPtr&,
                                                 const ::Test::ClassStructSeq&,
                                                 ::Test::ClassStructPtr& out,
                                                 ::Test::ClassStructSeq& outSeq,
                                                 const Ice::Current&);
                                                 
                                                 
    virtual void opOutArrayByteSeq(const Test::ByteSeq&, Test::ByteSeq&, const Ice::Current&);
    
    virtual void opOutRangeByteSeq(const Test::ByteSeq&, Test::ByteSeq&, const Ice::Current&);


    virtual Test::IntStringDict opIntStringDict(const Test::IntStringDict&, Test::IntStringDict&, 
                                                const Ice::Current&);

    virtual Test::CustomMap<Ice::Long, Ice::Long> opVarDict(const Test::CustomMap<std::string, Ice::Int>&,
                                                            Test::CustomMap<std::string, Ice::Int>&,
                                                            const Ice::Current&);

    
    virtual Test::CustomMap<Ice::Int, std::string> opCustomIntStringDict(
        const std::map<Ice::Int, Util::string_view>&, Test::CustomMap<Ice::Int, std::string>&, const Ice::Current&);

    Test::ShortBuffer opShortBuffer(const Test::ShortBuffer&, Test::ShortBuffer&, const Ice::Current&);

    Test::CustomBuffer<bool> opBoolBuffer(const Test::CustomBuffer<bool>&, Test::CustomBuffer<bool>&, 
                                          const Ice::Current&);

    Test::BufferStruct opBufferStruct(const Test::BufferStruct&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
