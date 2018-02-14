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

#include <TestAMD.h>

class TestIntfI : virtual public Test::TestIntf
{
public:

    TestIntfI(const Ice::CommunicatorPtr&);

    virtual void opDoubleArray_async(const Test::AMD_TestIntf_opDoubleArrayPtr&,
                                     const std::pair<const Ice::Double*, const Ice::Double*>&,
                                     const Ice::Current&);

    virtual void opBoolArray_async(const Test::AMD_TestIntf_opBoolArrayPtr&,
                                   const std::pair<const bool*, const bool*>&,
                                   const Ice::Current&);

    virtual void opByteArray_async(const Test::AMD_TestIntf_opByteArrayPtr&,
                                   const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                                   const Ice::Current&);

    virtual void opVariableArray_async(const Test::AMD_TestIntf_opVariableArrayPtr&,
                                       const std::pair<const Test::Variable*, const Test::Variable*>&,
                                       const Ice::Current&);

    virtual void opBoolRange_async(const Test::AMD_TestIntf_opBoolRangePtr&,
                                   const std::pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>&,
                                   const Ice::Current&);

    virtual void opByteRange_async(const Test::AMD_TestIntf_opByteRangePtr&,
                                   const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>&,
                                   const Ice::Current&);

    virtual void opVariableRange_async(const Test::AMD_TestIntf_opVariableRangePtr&,
                                       const std::pair<Test::VariableList::const_iterator, 
                                                       Test::VariableList::const_iterator>&,
                                       const Ice::Current&);

    virtual void opByteRangeType_async(const Test::AMD_TestIntf_opByteRangeTypePtr&,
                                       const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>&,
                                       const Ice::Current&);

    virtual void opVariableRangeType_async(const Test::AMD_TestIntf_opVariableRangeTypePtr&,
                                           const std::pair<std::deque<Test::Variable>::const_iterator, 
                                                           std::deque<Test::Variable>::const_iterator>&,
                                           const Ice::Current&);

    virtual void opBoolSeq_async(const Test::AMD_TestIntf_opBoolSeqPtr&,
                                 const std::deque<bool>&,
                                 const Ice::Current&);

    virtual void opBoolList_async(const Test::AMD_TestIntf_opBoolListPtr&,
                                  const Test::BoolList&,
                                  const Ice::Current&);

    virtual void opByteSeq_async(const Test::AMD_TestIntf_opByteSeqPtr&,
                                 const std::deque<Ice::Byte>&,
                                 const Ice::Current&);

    virtual void opByteList_async(const Test::AMD_TestIntf_opByteListPtr&,
                                  const Test::ByteList&,
                                  const Ice::Current&);

    virtual void opMyByteSeq_async(const Test::AMD_TestIntf_opMyByteSeqPtr&,
                                   const MyByteSeq&,
                                   const Ice::Current&);

    virtual void opString_async(const Test::AMD_TestIntf_opStringPtr&,
                                const Util::string_view&,
                                const Ice::Current&);

    virtual void opStringSeq_async(const Test::AMD_TestIntf_opStringSeqPtr&,
                                   const std::deque<std::string>&,
                                   const Ice::Current&);

    virtual void opStringList_async(const Test::AMD_TestIntf_opStringListPtr&,
                                    const Test::StringList&,
                                    const Ice::Current&);

    virtual void opFixedSeq_async(const Test::AMD_TestIntf_opFixedSeqPtr&,
                                  const std::deque<Test::Fixed>&,
                                  const Ice::Current&);

    virtual void opFixedList_async(const Test::AMD_TestIntf_opFixedListPtr&,
                                   const Test::FixedList&,
                                   const Ice::Current&);

    virtual void opVariableSeq_async(const Test::AMD_TestIntf_opVariableSeqPtr&,
                                     const std::deque<Test::Variable>&,
                                     const Ice::Current&);

    virtual void opVariableList_async(const Test::AMD_TestIntf_opVariableListPtr&,
                                      const Test::VariableList&,
                                      const Ice::Current&);

    virtual void opStringStringDictSeq_async(const Test::AMD_TestIntf_opStringStringDictSeqPtr&,
                                             const std::deque<Test::StringStringDict>&,
                                             const Ice::Current&);

    virtual void opStringStringDictList_async(const Test::AMD_TestIntf_opStringStringDictListPtr&,
                                              const Test::StringStringDictList&,
                                              const Ice::Current&);

    virtual void opESeq_async(const Test::AMD_TestIntf_opESeqPtr&,
                              const std::deque<Test::E>&,
                              const Ice::Current&);

    virtual void opEList_async(const Test::AMD_TestIntf_opEListPtr&,
                               const Test::EList&,
                               const Ice::Current&);

    virtual void opCPrxSeq_async(const Test::AMD_TestIntf_opCPrxSeqPtr&,
                                 const std::deque<Test::CPrx>&,
                                 const Ice::Current&);

    virtual void opCPrxList_async(const Test::AMD_TestIntf_opCPrxListPtr&,
                                  const Test::CPrxList&,
                                  const Ice::Current&);

    virtual void opCSeq_async(const Test::AMD_TestIntf_opCSeqPtr&,
                              const std::deque<Test::CPtr>&,
                              const Ice::Current&);

    virtual void opCList_async(const Test::AMD_TestIntf_opCListPtr&,
                               const Test::CList&,
                               const Ice::Current&);

    virtual void opClassStruct_async(const ::Test::AMD_TestIntf_opClassStructPtr&,
                                     const ::Test::ClassStructPtr&,
                                     const ::Test::ClassStructSeq&,
                                     const ::Ice::Current&);
                                     
                                     
    virtual void opOutArrayByteSeq_async(const ::Test::AMD_TestIntf_opOutArrayByteSeqPtr&,
                                         const ::Test::ByteSeq&, 
                                         const ::Ice::Current&);
                                         

    virtual void opOutRangeByteSeq_async(const ::Test::AMD_TestIntf_opOutRangeByteSeqPtr&,
                                         const ::Test::ByteSeq&,
                                         const ::Ice::Current&);

    virtual void opIntStringDict_async(const ::Test::AMD_TestIntf_opIntStringDictPtr&,
                                       const ::Test::IntStringDict&,
                                       const ::Ice::Current&);
    
    virtual void opVarDict_async(const ::Test::AMD_TestIntf_opVarDictPtr&,
                                 const ::Test::CustomMap<std::string, Ice::Int>&,                          
                                 const ::Ice::Current&);

    virtual void opCustomIntStringDict_async(const ::Test::AMD_TestIntf_opCustomIntStringDictPtr&,
                                             const std::map<Ice::Int, Util::string_view>&,
                                             const Ice::Current&);
    
    virtual void opShortBuffer_async(const ::Test::AMD_TestIntf_opShortBufferPtr&,
                                     const Test::ShortBuffer&, const Ice::Current&);

    virtual void opBoolBuffer_async(const ::Test::AMD_TestIntf_opBoolBufferPtr&,
                                    const Test::CustomBuffer<bool>&, const Ice::Current&);

    virtual void opBufferStruct_async(const ::Test::AMD_TestIntf_opBufferStructPtr&, const Test::BufferStruct&,
                                      const Ice::Current&);

    virtual void shutdown_async(const Test::AMD_TestIntf_shutdownPtr&,
                                const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
