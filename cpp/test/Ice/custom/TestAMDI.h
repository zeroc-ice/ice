// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

class TestIntfI : public virtual Test::TestIntf
{
public:

    TestIntfI(const Ice::CommunicatorPtr&);

#ifdef ICE_CPP11_MAPPING

    void opDoubleArray_async(std::pair<const ::Ice::Double*, const ::Ice::Double*>,
                             std::function<void (const ::Test::DoubleSeq&, const ::Test::DoubleSeq&)>,
                             std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opBoolArray_async(std::pair<const bool*, const bool*>,
                           std::function<void (const ::Test::BoolSeq&, const ::Test::BoolSeq&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opByteArray_async(std::pair<const ::Ice::Byte*, const ::Ice::Byte*>,
                           std::function<void (const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                                               const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;

    void opVariableArray_async(std::pair<const ::Test::Variable*, const ::Test::Variable*>,
                               std::function<void (const ::Test::VariableList&, const ::Test::VariableList&)>,
                               std::function<void (std::exception_ptr)>, const Ice::Current&) override;

    void opBoolRange_async(std::pair< ::Test::BoolSeq::const_iterator,	::Test::BoolSeq::const_iterator>,
                           std::function<void (const ::Test::BoolSeq&, const ::Test::BoolSeq&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opByteRange_async(std::pair< ::Test::ByteList::const_iterator,  ::Test::ByteList::const_iterator>,
                           std::function<void (const std::pair< ::Test::ByteList::const_iterator,
                                               ::Test::ByteList::const_iterator>&,
                                               const std::pair< ::Test::ByteList::const_iterator,
                                               ::Test::ByteList::const_iterator>&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;

    void opVariableRange_async(std::pair< ::Test::VariableList::const_iterator,
                               ::Test::VariableList::const_iterator>,
                               std::function<void (const ::Test::VariableList&, const ::Test::VariableList&)>,
                               std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opByteRangeType_async(std::pair< ::Test::ByteList::const_iterator,  ::Test::ByteList::const_iterator>,
                               std::function<void (const ::Test::ByteList&, const ::Test::ByteList&)>,
                               std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opVariableRangeType_async(std::pair<std::deque< ::Test::Variable>::const_iterator,
                                   std::deque< ::Test::Variable>::const_iterator>,
                                   std::function<void (const ::Test::VariableList&,
                                                       const ::Test::VariableList&)>,
                                   std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opBoolSeq_async(std::deque<bool>,
                         std::function<void (const std::deque<bool>&, const std::deque<bool>&)>,
                         std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opBoolList_async(::Test::BoolList,
                          std::function<void (const ::Test::BoolList&, const ::Test::BoolList&)>,
                          std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opByteSeq_async(std::deque< ::Ice::Byte>,
                         std::function<void (const std::deque< ::Ice::Byte>&, const std::deque< ::Ice::Byte>&)>,
                         std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opByteList_async(::Test::ByteList,
                          std::function<void (const ::Test::ByteList&, const ::Test::ByteList&)>,
                          std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opMyByteSeq_async(MyByteSeq,
                           std::function<void (const MyByteSeq&, const MyByteSeq&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opString_async(Util::string_view,
                        std::function<void (const Util::string_view&, const Util::string_view&)>,
                        std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opStringSeq_async(std::deque<std::string>,
                           std::function<void (const std::deque<std::string>&, const std::deque<std::string>&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;

    void opStringList_async(::Test::StringList,
                            std::function<void (const ::Test::StringList&, const ::Test::StringList&)>,
                            std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opFixedSeq_async(std::deque< ::Test::Fixed>,
                          std::function<void (const std::deque< ::Test::Fixed>&,
                                              const std::deque< ::Test::Fixed>&)>,
                          std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opFixedList_async(::Test::FixedList,
                           std::function<void (const ::Test::FixedList&, const ::Test::FixedList&)>,
                           std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opVariableSeq_async(std::deque< ::Test::Variable>,
                             std::function<void (const std::deque< ::Test::Variable>&,
                                                 const std::deque< ::Test::Variable>&)>,
                             std::function<void (std::exception_ptr)>, const Ice::Current&) override;

    void opVariableList_async(::Test::VariableList,
                              std::function<void (const ::Test::VariableList&, const ::Test::VariableList&)>,
                              std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opStringStringDictSeq_async(std::deque< ::Test::StringStringDict>,
                                     std::function<void (const std::deque< ::Test::StringStringDict>&,
                                                         const std::deque< ::Test::StringStringDict>&)>,
                                     std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opStringStringDictList_async(::Test::StringStringDictList,
                                      std::function<void (const ::Test::StringStringDictList&,
                                                          const ::Test::StringStringDictList&)>,
                                      std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opESeq_async(std::deque< ::Test::E>,
                      std::function<void (const std::deque< ::Test::E>&, const std::deque< ::Test::E>&)>,
                      std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opEList_async(::Test::EList,
                       std::function<void (const ::Test::EList&, const ::Test::EList&)>,
                       std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opCPrxSeq_async(std::deque<std::shared_ptr<Ice::ObjectPrx>>,
                         std::function<void (const std::deque<std::shared_ptr<Ice::ObjectPrx>>&,
                                             const std::deque<std::shared_ptr<Ice::ObjectPrx>>&)>,
                         std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opCPrxList_async(::Test::CPrxList,
                          std::function<void (const ::Test::CPrxList&, const ::Test::CPrxList&)>,
                          std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opCSeq_async(std::deque<std::shared_ptr<Test::C>>,
                      std::function<void (const std::deque<std::shared_ptr<Test::C>>&,
                                          const std::deque<std::shared_ptr<Test::C>>&)>,
                      std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opCList_async(::Test::CList,
                       std::function<void (const ::Test::CList&, const ::Test::CList&)>,
                       std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opClassStruct_async(::Test::ClassStruct, ::Test::ClassStructSeq,
                             std::function<void (const ::Test::ClassStruct&,
                                                 const ::Test::ClassStruct&, const ::Test::ClassStructSeq&)>,
                             std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opOutArrayByteSeq_async(::Test::ByteSeq,
                                 std::function<void (const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)>,
                                 std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opOutRangeByteSeq_async(::Test::ByteSeq,
                                 std::function<void (const std::pair< ::Test::ByteSeq::const_iterator,
                                                     ::Test::ByteSeq::const_iterator>&)>,
                                 std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opIntStringDict_async(::Test::IntStringDict,
                                       std::function<void (const ::Test::IntStringDict&, const ::Test::IntStringDict&)>,
                               std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opVarDict_async(::Test::CustomMap<std::string, ::Ice::Int>,
                                 std::function<void (const ::Test::CustomMap< ::Ice::Long, ::Ice::Long>&,
                                                     const ::Test::CustomMap<std::string, ::Ice::Int>&)>,
                                 std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opCustomIntStringDict_async(std::map< ::Ice::Int, ::Util::string_view>,
                                             std::function<void (const std::map< ::Ice::Int, ::Util::string_view>&,
                                                                 const std::map< ::Ice::Int, ::Util::string_view>&)>,
                                             std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opShortBuffer_async(::Test::ShortBuffer,
                                     std::function<void (const ::Test::ShortBuffer&, const ::Test::ShortBuffer&)>,
                                     std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opBoolBuffer_async(::Test::CustomBuffer<bool>,
                                    std::function<void (const ::Test::CustomBuffer<bool>&,
                                                        const ::Test::CustomBuffer<bool>&)>,
                                    std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void opBufferStruct_async(::Test::BufferStruct,
                                      std::function<void (const ::Test::BufferStruct&)>,
                                      std::function<void (std::exception_ptr)>, const Ice::Current&) override;


    void shutdown_async(std::function<void ()>,
                                std::function<void (std::exception_ptr)>, const Ice::Current&) override;


#else
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
#endif

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
