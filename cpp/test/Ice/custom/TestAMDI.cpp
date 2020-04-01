//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <TestAMDI.h>

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
TestIntfI::opDPrxSeqAsync(std::deque<std::shared_ptr<Test::DPrx>> in,
                          std::function<void(const std::deque<std::shared_ptr<Test::DPrx>>&,
                                             const std::deque<std::shared_ptr<Test::DPrx>>&)> response,
                          std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(in, in);
}

void
TestIntfI::opDPrxListAsync(Test::DPrxList in,
                           std::function<void(const Test::DPrxList&, const Test::DPrxList&)> response,
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
TestIntfI::opOutArrayByteSeqAsync(Test::ByteSeq in,
                                  std::function<void(const std::pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                  std::function<void(std::exception_ptr)>, const Ice::Current&)
{
    response(std::make_pair(in.data(), in.data() + in.size()));
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
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}
