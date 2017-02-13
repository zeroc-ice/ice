// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#ifdef ICE_CPP11_MAPPING

void
TestIntfI::opDoubleArrayAsync(std::pair<const Ice::Double*, const Ice::Double*> in,
                              std::function<void(const Test::DoubleSeq&, const Test::DoubleSeq&)> response,
                              std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    Test::DoubleSeq out(in.first, in.second);
    response(out, out);
}

void
TestIntfI::opBoolArrayAsync(std::pair<const bool*, const bool*> in,
                            std::function<void(const Test::BoolSeq&, const Test::BoolSeq&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    Test::BoolSeq out(in.first, in.second);
    response(out, out);
}

void
TestIntfI::opByteArrayAsync(std::pair<const Ice::Byte*, const Ice::Byte*> in,
                            std::function<void(const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                                const std::pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableArrayAsync(std::pair<const Test::Variable*, const Test::Variable*> in,
                                std::function<void(const Test::VariableList&, const Test::VariableList&)> response,
                                std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    Test::VariableList out(in.first, in.second);
    response(out, out);
}

void
TestIntfI::opBoolRangeAsync(Test::BoolSeq in,
                            std::function<void(const Test::BoolSeq&, const Test::BoolSeq&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opByteRangeAsync(Test::ByteList in,
                            std::function<void(const Test::ByteList&, const Test::ByteList&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableRangeAsync(Test::VariableList in,
                                std::function<void(const Test::VariableList&, const Test::VariableList&)> response,
                                std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}


void
TestIntfI::opByteRangeTypeAsync(Test::ByteList in,
                                std::function<void(const Test::ByteList&, const Test::ByteList&)> response,
                                std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableRangeTypeAsync(Test::VariableList in,
                                    std::function<void(const Test::VariableList&,
                                                        const Test::VariableList&)> response,
                                    std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opBoolSeqAsync(std::deque<bool> in,
                          std::function<void(const std::deque<bool>&, const std::deque<bool>&)> response,
                          std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}


void
TestIntfI::opBoolListAsync(Test::BoolList in,
                           std::function<void(const Test::BoolList&, const Test::BoolList&)> response,
                           std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opByteSeqAsync(std::deque<Ice::Byte> in,
                          std::function<void(const std::deque<Ice::Byte>&, const std::deque<Ice::Byte>&)> response,
                          std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opByteListAsync(Test::ByteList in,
                           std::function<void(const Test::ByteList&, const Test::ByteList&)> response,
                           std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opMyByteSeqAsync(MyByteSeq in,
                            std::function<void(const MyByteSeq&, const MyByteSeq&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opStringAsync(Util::string_view in,
                         std::function<void(const Util::string_view&, const Util::string_view&)> response,
                         std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opStringSeqAsync(std::deque<std::string> in,
                            std::function<void(const std::deque<std::string>&, const std::deque<std::string>&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opStringListAsync(Test::StringList in,
                             std::function<void(const Test::StringList&, const Test::StringList&)> response,
                             std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opFixedSeqAsync(std::deque<Test::Fixed> in,
                           std::function<void(const std::deque<Test::Fixed>&,
                                               const std::deque<Test::Fixed>&)> response,
                           std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opFixedListAsync(Test::FixedList in,
                            std::function<void(const Test::FixedList&, const Test::FixedList&)> response,
                            std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}


void
TestIntfI::opVariableSeqAsync(std::deque<Test::Variable> in,
                              std::function<void(const std::deque<Test::Variable>&,
                                                  const std::deque<Test::Variable>&)> response,
                              std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opVariableListAsync(Test::VariableList in,
                               std::function<void(const Test::VariableList&, const Test::VariableList&)> response,
                               std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opStringStringDictSeqAsync(std::deque<Test::StringStringDict> in,
                                      std::function<void(const std::deque<Test::StringStringDict>&,
                                                          const std::deque<Test::StringStringDict>&)> response,
                                      std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opStringStringDictListAsync(Test::StringStringDictList in,
                                       std::function<void(const Test::StringStringDictList&,
                                                           const Test::StringStringDictList&)> response,
                                       std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opESeqAsync(std::deque<Test::E> in,
                       std::function<void(const std::deque<Test::E>&, const std::deque<Test::E>&)> response,
                       std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opEListAsync(Test::EList in,
                        std::function<void(const Test::EList&, const Test::EList&)> response,
                        std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opCPrxSeqAsync(std::deque<std::shared_ptr<Ice::ObjectPrx>> in,
                          std::function<void(const std::deque<std::shared_ptr<Ice::ObjectPrx>>&,
                                              const std::deque<std::shared_ptr<Ice::ObjectPrx>>&)> response,
                          std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opCPrxListAsync(Test::CPrxList in,
                           std::function<void(const Test::CPrxList&, const Test::CPrxList&)> response,
                           std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opCSeqAsync(std::deque<std::shared_ptr<Test::C>> in,
                       std::function<void(const std::deque<std::shared_ptr<Test::C>>&,
                                           const std::deque<std::shared_ptr<Test::C>>&)> response,
                       std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opCListAsync(Test::CList in,
                        std::function<void(const Test::CList&, const Test::CList&)> response,
                        std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opClassStructAsync(Test::ClassStruct in1, Test::ClassStructSeq in2,
                              std::function<void(const Test::ClassStruct&,
                                                  const Test::ClassStruct&, const Test::ClassStructSeq&)> response,
                              std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in1, in1, in2);
}

void
TestIntfI::opOutArrayByteSeqAsync(Test::ByteSeq in,
                                  std::function<void(const std::pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                  std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(std::make_pair(in.data(), in.data() + in.size()));
}

void
TestIntfI::opOutRangeByteSeqAsync(Test::ByteSeq in,
                                  std::function<void(const Test::ByteSeq&)> response,
                                  std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in);
}

void
TestIntfI::opIntStringDictAsync(Test::IntStringDict in,
                                std::function<void(const Test::IntStringDict&, const Test::IntStringDict&)> response,
                                std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opVarDictAsync(Test::CustomMap<std::string, Ice::Int> in,
                          std::function<void(const Test::CustomMap<Ice::Long, Ice::Long>&,
                                              const Test::CustomMap<std::string, Ice::Int>&)> response,
                          std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    Test::CustomMap<Ice::Long, Ice::Long> result;
    for(Ice::Long i = 0; i < 1000; ++i)
    {
        result[i] = i*i;
    }
    response(result, in);
}

void
TestIntfI::opCustomIntStringDictAsync(std::map<Ice::Int, ::Util::string_view> in,
                                      std::function<void(const std::map<Ice::Int, ::Util::string_view>&,
                                                          const std::map<Ice::Int, ::Util::string_view>&)> response,
                                      std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opShortBufferAsync(Test::ShortBuffer in,
                              std::function<void(const Test::ShortBuffer&, const Test::ShortBuffer&)> response,
                              std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}


void
TestIntfI::opBoolBufferAsync(Test::CustomBuffer<bool> in,
                             std::function<void(const Test::CustomBuffer<bool>&,
                                                 const Test::CustomBuffer<bool>&)> response,
                             std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opBufferStructAsync(Test::BufferStruct in,
                               std::function<void(const Test::BufferStruct&)> response,
                               std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in);
}


void
TestIntfI::shutdownAsync(std::function<void()> response,
                         std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    _communicator->shutdown();
    response();
}

#else

void
TestIntfI::opDoubleArray_async(const Test::AMD_TestIntf_opDoubleArrayPtr& opDoubleArrayCB,
                               const std::pair<const Ice::Double*, const Ice::Double*>& inSeq,
                               const Ice::Current&)
{
    Test::DoubleSeq outSeq(inSeq.first, inSeq.second);
    opDoubleArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolArray_async(const Test::AMD_TestIntf_opBoolArrayPtr& opBoolArrayCB,
                             const std::pair<const bool*, const bool*>& inSeq,
                             const Ice::Current&)
{
    Test::BoolSeq outSeq(inSeq.first, inSeq.second);
    opBoolArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteArray_async(const Test::AMD_TestIntf_opByteArrayPtr& opByteArrayCB,
                             const std::pair<const Ice::Byte*, const Ice::Byte*>& inSeq,
                             const Ice::Current&)
{
    opByteArrayCB->ice_response(inSeq, inSeq);
}

void
TestIntfI::opVariableArray_async(const Test::AMD_TestIntf_opVariableArrayPtr& opVariableArrayCB,
                                 const std::pair<const Test::Variable*, const Test::Variable*>& inSeq,
                                 const Ice::Current&)
{
    Test::VariableList outSeq(inSeq.first, inSeq.second);
    opVariableArrayCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolRange_async(const Test::AMD_TestIntf_opBoolRangePtr& opBoolRangeCB,
                             const std::pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& inSeq,
                             const Ice::Current&)
{
    Test::BoolSeq outSeq(inSeq.first, inSeq.second);
    opBoolRangeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteRange_async(const Test::AMD_TestIntf_opByteRangePtr& opByteRangeCB,
                             const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                             const Ice::Current&)
{
    opByteRangeCB->ice_response(inSeq, inSeq);
}

void
TestIntfI::opVariableRange_async(const Test::AMD_TestIntf_opVariableRangePtr& opVariableRangeCB,
                                 const std::pair<Test::VariableList::const_iterator,
                                 Test::VariableList::const_iterator>& inSeq,
                                 const Ice::Current&)
{
    Test::VariableList outSeq(inSeq.first, inSeq.second);
    opVariableRangeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteRangeType_async(const Test::AMD_TestIntf_opByteRangeTypePtr& opByteRangeTypeCB,
                                 const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                                 const Ice::Current&)
{
    Test::ByteList outSeq(inSeq.first, inSeq.second);
    opByteRangeTypeCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableRangeType_async(const Test::AMD_TestIntf_opVariableRangeTypePtr& opVariableRangeTypeCB,
                                     const std::pair<std::deque<Test::Variable>::const_iterator,
                                     std::deque<Test::Variable>::const_iterator>& inSeq,
                                     const Ice::Current&)
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
                           const Ice::Current&)
{
    std::deque<bool> outSeq(inSeq);
    opBoolSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opBoolList_async(const Test::AMD_TestIntf_opBoolListPtr& opBoolListCB,
                            const Test::BoolList& inSeq,
                            const Ice::Current&)
{
    Test::BoolList outSeq(inSeq);
    opBoolListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteSeq_async(const Test::AMD_TestIntf_opByteSeqPtr& opByteSeqCB,
                           const std::deque<Ice::Byte>& inSeq,
                           const Ice::Current&)
{
    std::deque<Ice::Byte> outSeq(inSeq);
    opByteSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opByteList_async(const Test::AMD_TestIntf_opByteListPtr& opByteListCB,
                            const Test::ByteList& inSeq,
                            const Ice::Current&)
{
    Test::ByteList outSeq(inSeq);
    opByteListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opMyByteSeq_async(const Test::AMD_TestIntf_opMyByteSeqPtr& opMyByteSeqCB,
                             const MyByteSeq& inSeq,
                             const Ice::Current&)
{
    MyByteSeq outSeq(inSeq);
    opMyByteSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opString_async(const Test::AMD_TestIntf_opStringPtr& opStringCB,
                          const Util::string_view& inString,
                          const Ice::Current&)
{
    opStringCB->ice_response(inString, inString);
}

void
TestIntfI::opStringSeq_async(const Test::AMD_TestIntf_opStringSeqPtr& opStringSeqCB,
                             const std::deque<std::string>& inSeq,
                             const Ice::Current&)
{
    std::deque<std::string> outSeq(inSeq);
    opStringSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringList_async(const Test::AMD_TestIntf_opStringListPtr& opStringListCB,
                              const Test::StringList& inSeq,
                              const Ice::Current&)
{
    Test::StringList outSeq(inSeq);
    opStringListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opFixedSeq_async(const Test::AMD_TestIntf_opFixedSeqPtr& opFixedSeqCB,
                            const std::deque<Test::Fixed>& inSeq,
                            const Ice::Current&)
{
    std::deque<Test::Fixed> outSeq(inSeq);
    opFixedSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opFixedList_async(const Test::AMD_TestIntf_opFixedListPtr& opFixedListCB,
                             const Test::FixedList& inSeq,
                             const Ice::Current&)
{
    Test::FixedList outSeq(inSeq);
    opFixedListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableSeq_async(const Test::AMD_TestIntf_opVariableSeqPtr& opVariableSeqCB,
                               const std::deque<Test::Variable>& inSeq,
                               const Ice::Current&)
{
    std::deque<Test::Variable> outSeq(inSeq);
    opVariableSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opVariableList_async(const Test::AMD_TestIntf_opVariableListPtr& opVariableListCB,
                                const Test::VariableList& inSeq,
                                const Ice::Current&)
{
    Test::VariableList outSeq(inSeq);
    opVariableListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringStringDictSeq_async(const Test::AMD_TestIntf_opStringStringDictSeqPtr& opStringStringDictSeqCB,
                                       const std::deque<Test::StringStringDict>& inSeq,
                                       const Ice::Current&)
{
    std::deque<Test::StringStringDict> outSeq(inSeq);
    opStringStringDictSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opStringStringDictList_async(const Test::AMD_TestIntf_opStringStringDictListPtr& opStringStringDictListCB,
                                        const Test::StringStringDictList& inSeq,
                                        const Ice::Current&)
{
    Test::StringStringDictList outSeq(inSeq);
    opStringStringDictListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opESeq_async(const Test::AMD_TestIntf_opESeqPtr& opESeqCB,
                        const std::deque<Test::E>& inSeq,
                        const Ice::Current&)
{
    std::deque<Test::E> outSeq(inSeq);
    opESeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opEList_async(const Test::AMD_TestIntf_opEListPtr& opEListCB,
                         const Test::EList& inSeq,
                         const Ice::Current&)
{
    Test::EList outSeq(inSeq);
    opEListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCPrxSeq_async(const Test::AMD_TestIntf_opCPrxSeqPtr& opCPrxSeqCB,
                           const std::deque<Test::CPrx>& inSeq,
                           const Ice::Current&)
{
    std::deque<Test::CPrx> outSeq(inSeq);
    opCPrxSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCPrxList_async(const Test::AMD_TestIntf_opCPrxListPtr& opCPrxListCB,
                            const Test::CPrxList& inSeq,
                            const Ice::Current&)
{
    Test::CPrxList outSeq(inSeq);
    opCPrxListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCSeq_async(const Test::AMD_TestIntf_opCSeqPtr& opCSeqCB,
                        const std::deque<Test::CPtr>& inSeq,
                        const Ice::Current&)
{
    std::deque<Test::CPtr> outSeq(inSeq);
    opCSeqCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opCList_async(const Test::AMD_TestIntf_opCListPtr& opCListCB,
                         const Test::CList& inSeq,
                         const Ice::Current&)
{
    Test::CList outSeq(inSeq);
    opCListCB->ice_response(outSeq, outSeq);
}

void
TestIntfI::opClassStruct_async(const Test::AMD_TestIntf_opClassStructPtr& cb,
                               const Test::ClassStructPtr& inS,
                               const Test::ClassStructSeq& inSeq,
                               const Ice::Current&)
{
    cb->ice_response(inS, inS, inSeq);
}


void
TestIntfI::opOutArrayByteSeq_async(const Test::AMD_TestIntf_opOutArrayByteSeqPtr& cb,
                                   const Test::ByteSeq& inS,
                                   const Ice::Current&)
{
    cb->ice_response(std::pair<const Ice::Byte*,
                     const Ice::Byte*>(&inS[0], &inS[0] + inS.size()));
}

void
TestIntfI::opOutRangeByteSeq_async(const Test::AMD_TestIntf_opOutRangeByteSeqPtr& cb,
                                   const Test::ByteSeq& inS,
                                   const Ice::Current&)
{
    cb->ice_response(std::pair<Test::ByteSeq::const_iterator,
                     Test::ByteSeq::const_iterator>(inS.begin(), inS.end()));
}

void
TestIntfI::opIntStringDict_async(const Test::AMD_TestIntf_opIntStringDictPtr& cb,
                                 const Test::IntStringDict& inDict,
                                 const Ice::Current&)
{
    cb->ice_response(inDict, inDict);
}

void
TestIntfI::opVarDict_async(const Test::AMD_TestIntf_opVarDictPtr& cb,
                           const Test::CustomMap<std::string, Ice::Int>& inDict,
                           const Ice::Current&)
{
    Test::CustomMap<Ice::Long, Ice::Long> result;
    for(Ice::Long i = 0; i < 1000; ++i)
    {
        result[i] = i*i;
    }
    cb->ice_response(result, inDict);
}

void
TestIntfI::opCustomIntStringDict_async(const Test::AMD_TestIntf_opCustomIntStringDictPtr& cb,
                                       const std::map<Ice::Int, Util::string_view>& data,
                                       const Ice::Current&)
{
    cb->ice_response(data, data);
}

void
TestIntfI::opShortBuffer_async(const Test::AMD_TestIntf_opShortBufferPtr& cb, const Test::ShortBuffer& inS,
                               const Ice::Current&)
{
    cb->ice_response(inS, inS);
}

void
TestIntfI::opBoolBuffer_async(const Test::AMD_TestIntf_opBoolBufferPtr& cb, const Test::CustomBuffer<bool>& inS,
                              const Ice::Current&)
{
    cb->ice_response(inS, inS);
}

void
TestIntfI::opBufferStruct_async(const Test::AMD_TestIntf_opBufferStructPtr& cb,
                                const Test::BufferStruct& s, const Ice::Current&)
{
    cb->ice_response(s);
}

void
TestIntfI::shutdown_async(const Test::AMD_TestIntf_shutdownPtr& shutdownCB,
                          const Ice::Current&)
{
    _communicator->shutdown();
    shutdownCB->ice_response();
}
#endif
