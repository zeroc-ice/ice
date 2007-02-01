// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <TestAMDI.h>

TestIntfI::TestIntfI(const Ice::CommunicatorPtr& communicator)
    : _communicator(communicator)
{
}

void
TestIntfI::opDoubleArray_async(const Test::AMD_TestIntf_opDoubleArrayPtr& opDoubleArrayCB,
                               const std::pair<const Ice::Double*, const Ice::Double*>& inSeq,
                               const Ice::Current& current)
{
    Test::DoubleSeq outSeq(inSeq.first, inSeq.second);
    opDoubleArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolArray_async(const Test::AMD_TestIntf_opBoolArrayPtr& opBoolArrayCB,
                             const std::pair<const bool*, const bool*>& inSeq,
                             const Ice::Current& current)
{
    Test::BoolSeq outSeq(inSeq.first, inSeq.second);
    opBoolArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteArray_async(const Test::AMD_TestIntf_opByteArrayPtr& opByteArrayCB,
                             const std::pair<const Ice::Byte*, const Ice::Byte*>& inSeq,
                             const Ice::Current& current)
{
    opByteArrayCB->ice_response(inSeq, inSeq);
}

void
TestIntfI::opVariableArray_async(const Test::AMD_TestIntf_opVariableArrayPtr& opVariableArrayCB,
                                 const std::pair<const Test::Variable*, const Test::Variable*>& inSeq,
                                 const Ice::Current& current)
{
    Test::VariableList outSeq(inSeq.first, inSeq.second);
    opVariableArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolRange_async(const Test::AMD_TestIntf_opBoolRangePtr& opBoolRangeCB,
                             const std::pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& inSeq,
                             const Ice::Current& current)
{
    Test::BoolSeq outSeq(inSeq.first, inSeq.second);
    opBoolRangeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteRange_async(const Test::AMD_TestIntf_opByteRangePtr& opByteRangeCB,
                             const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                             const Ice::Current& current)
{
    opByteRangeCB->ice_response(inSeq, inSeq);
}

void
TestIntfI::opVariableRange_async(const Test::AMD_TestIntf_opVariableRangePtr& opVariableRangeCB,
                                 const std::pair<Test::VariableList::const_iterator, 
                                                 Test::VariableList::const_iterator>& inSeq,
                                 const Ice::Current& current)
{
    Test::VariableList outSeq(inSeq.first, inSeq.second);
    opVariableRangeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolRangeType_async(const Test::AMD_TestIntf_opBoolRangeTypePtr& opBoolRangeTypeCB,
                                 const std::pair<const bool*, const bool*>& inSeq,
                                 const Ice::Current& current)
{
    Test::BoolSeq outSeq(inSeq.first, inSeq.second);
    opBoolRangeTypeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteRangeType_async(const Test::AMD_TestIntf_opByteRangeTypePtr& opByteRangeTypeCB,
                                 const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                                 const Ice::Current& current)
{
    Test::ByteList outSeq(inSeq.first, inSeq.second);
    opByteRangeTypeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableRangeType_async(const Test::AMD_TestIntf_opVariableRangeTypePtr& opVariableRangeTypeCB,
                                     const std::pair<std::deque<Test::Variable>::const_iterator, 
                                                     std::deque<Test::Variable>::const_iterator>& inSeq,
                                     const Ice::Current& current)
{
    Test::VariableList outSeq;
    for(std::deque<Test::Variable>::const_iterator p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    opVariableRangeTypeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolSeq_async(const Test::AMD_TestIntf_opBoolSeqPtr& opBoolSeqCB,
                           const std::deque<bool>& inSeq,
                           const Ice::Current& current)
{
    std::deque<bool> outSeq(inSeq);
    opBoolSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolList_async(const Test::AMD_TestIntf_opBoolListPtr& opBoolListCB,
                            const Test::BoolList& inSeq,
                            const Ice::Current& current)
{
    Test::BoolList outSeq(inSeq);
    opBoolListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteSeq_async(const Test::AMD_TestIntf_opByteSeqPtr& opByteSeqCB,
                           const std::deque<Ice::Byte>& inSeq,
                           const Ice::Current& current)
{
    std::deque<Ice::Byte> outSeq(inSeq);
    opByteSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteList_async(const Test::AMD_TestIntf_opByteListPtr& opByteListCB,
                            const Test::ByteList& inSeq,
                            const Ice::Current& current)
{
    Test::ByteList outSeq(inSeq);
    opByteListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opMyByteSeq_async(const Test::AMD_TestIntf_opMyByteSeqPtr& opMyByteSeqCB,
                             const MyByteSeq& inSeq,
                             const Ice::Current& current)
{
    MyByteSeq outSeq(inSeq);
    opMyByteSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringSeq_async(const Test::AMD_TestIntf_opStringSeqPtr& opStringSeqCB,
                             const std::deque<std::string>& inSeq,
                             const Ice::Current& current)
{
    std::deque<std::string> outSeq(inSeq);
    opStringSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringList_async(const Test::AMD_TestIntf_opStringListPtr& opStringListCB,
                              const Test::StringList& inSeq,
                              const Ice::Current& current)
{
    Test::StringList outSeq(inSeq);
    opStringListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opFixedSeq_async(const Test::AMD_TestIntf_opFixedSeqPtr& opFixedSeqCB,
                            const std::deque<Test::Fixed>& inSeq,
                            const Ice::Current& current)
{
    std::deque<Test::Fixed> outSeq(inSeq);
    opFixedSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opFixedList_async(const Test::AMD_TestIntf_opFixedListPtr& opFixedListCB,
                             const Test::FixedList& inSeq,
                             const Ice::Current& current)
{
    Test::FixedList outSeq(inSeq);
    opFixedListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableSeq_async(const Test::AMD_TestIntf_opVariableSeqPtr& opVariableSeqCB,
                               const std::deque<Test::Variable>& inSeq,
                               const Ice::Current& current)
{
    std::deque<Test::Variable> outSeq(inSeq);
    opVariableSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableList_async(const Test::AMD_TestIntf_opVariableListPtr& opVariableListCB,
                                const Test::VariableList& inSeq,
                                const Ice::Current& current)
{
    Test::VariableList outSeq(inSeq);
    opVariableListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringStringDictSeq_async(const Test::AMD_TestIntf_opStringStringDictSeqPtr& opStringStringDictSeqCB,
                                       const std::deque<Test::StringStringDict>& inSeq,
                                       const Ice::Current& current)
{
    std::deque<Test::StringStringDict> outSeq(inSeq);
    opStringStringDictSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringStringDictList_async(const Test::AMD_TestIntf_opStringStringDictListPtr& opStringStringDictListCB,
                                        const Test::StringStringDictList& inSeq,
                                        const Ice::Current& current)
{
    Test::StringStringDictList outSeq(inSeq);
    opStringStringDictListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opESeq_async(const Test::AMD_TestIntf_opESeqPtr& opESeqCB,
                        const std::deque<Test::E>& inSeq,
                        const Ice::Current& current)
{
    std::deque<Test::E> outSeq(inSeq);
    opESeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opEList_async(const Test::AMD_TestIntf_opEListPtr& opEListCB,
                         const Test::EList& inSeq,
                         const Ice::Current& current)
{
    Test::EList outSeq(inSeq);
    opEListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCPrxSeq_async(const Test::AMD_TestIntf_opCPrxSeqPtr& opCPrxSeqCB,
                           const std::deque<Test::CPrx>& inSeq,
                           const Ice::Current& current)
{
    std::deque<Test::CPrx> outSeq(inSeq);
    opCPrxSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCPrxList_async(const Test::AMD_TestIntf_opCPrxListPtr& opCPrxListCB,
                            const Test::CPrxList& inSeq,
                            const Ice::Current& current)
{
    Test::CPrxList outSeq(inSeq);
    opCPrxListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCSeq_async(const Test::AMD_TestIntf_opCSeqPtr& opCSeqCB,
                         const std::deque<Test::CPtr>& inSeq,
                         const Ice::Current& current)
{
    std::deque<Test::CPtr> outSeq(inSeq);
    opCSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCList_async(const Test::AMD_TestIntf_opCListPtr& opCListCB,
                          const Test::CList& inSeq,
                          const Ice::Current& current)
{
    Test::CList outSeq(inSeq);
    opCListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opClassStruct_async(const ::Test::AMD_TestIntf_opClassStructPtr& cb,
                               const ::Test::ClassStructPtr& inS,
                               const ::Test::ClassStructSeq& inSeq,
                               const ::Ice::Current&)
{
    cb->ice_response(inS, inS, inSeq);
}

void
TestIntfI::shutdown_async(const Test::AMD_TestIntf_shutdownPtr& shutdownCB,
                          const Ice::Current& current)
{
    _communicator->shutdown();
    shutdownCB->ice_response();
}
