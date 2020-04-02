//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <TestI.h>

Test::DoubleSeq
TestIntfI::opDoubleArray(std::pair<const Ice::Double*, const Ice::Double*> inSeq,
                         Test::DoubleSeq& outSeq,
                         const Ice::Current&)
{
    Test::DoubleSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::BoolSeq
TestIntfI::opBoolArray(std::pair<const bool*, const bool*> inSeq,
                       Test::BoolSeq& outSeq,
                       const Ice::Current&)
{
    Test::BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteArray(std::pair<const Ice::Byte*, const Ice::Byte*> inSeq,
                       Test::ByteList& outSeq,
                       const Ice::Current&)
{
    Test::ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableArray(std::pair<const Test::Variable*, const Test::Variable*> inSeq,
                           Test::VariableList& outSeq,
                           const Ice::Current&)
{
    Test::VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

std::deque<bool>
TestIntfI::opBoolSeq(std::deque<bool> inSeq,
                     std::deque<bool>& outSeq,
                     const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list<bool>
TestIntfI::opBoolList(std::list<bool> inSeq,
                      std::list<bool>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

::Test::BoolDequeList
TestIntfI::opBoolDequeList(::Test::BoolDequeList inSeq, ::Test::BoolDequeList& outSeq, const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

::Test::BoolDequeList
TestIntfI::opBoolDequeListArray(::std::pair<const std::deque<bool>*, const std::deque<bool>*> inSeq,
                                ::Test::BoolDequeList& outSeq,
                                const ::Ice::Current&)
{
    for(const std::deque<bool>* p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    return outSeq;
}

std::deque< ::Ice::Byte>
TestIntfI::opByteSeq(std::deque< ::Ice::Byte> inSeq,
                     std::deque< ::Ice::Byte>& outSeq,
                     const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Ice::Byte>
TestIntfI::opByteList(std::list< ::Ice::Byte> inSeq,
                      std::list< ::Ice::Byte>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

MyByteSeq
TestIntfI::opMyByteSeq(MyByteSeq inSeq,
                       MyByteSeq& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::string
TestIntfI::opString(Util::string_view inString,
                    std::string& outString,
                    const Ice::Current&)
{
    outString = inString.to_string();
    return outString;
}

std::deque< ::std::string>
TestIntfI::opStringSeq(std::deque< ::std::string> inSeq,
                       std::deque< ::std::string>& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::std::string>
TestIntfI::opStringList(std::list< ::std::string> inSeq,
                        std::list< ::std::string>& outSeq,
                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Fixed>
TestIntfI::opFixedSeq(std::deque< ::Test::Fixed> inSeq,
                      std::deque< ::Test::Fixed>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Fixed>
TestIntfI::opFixedList(std::list< ::Test::Fixed> inSeq,
                       std::list< ::Test::Fixed>& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Variable>
TestIntfI::opVariableSeq(std::deque< ::Test::Variable> inSeq,
                         std::deque< ::Test::Variable>& outSeq,
                         const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Variable>
TestIntfI::opVariableList(std::list< ::Test::Variable> inSeq,
                          std::list< ::Test::Variable>& outSeq,
                          const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::StringStringDict>
TestIntfI::opStringStringDictSeq(std::deque< ::Test::StringStringDict> inSeq,
                                       std::deque< ::Test::StringStringDict>& outSeq,
                                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::StringStringDict>
TestIntfI::opStringStringDictList(std::list< ::Test::StringStringDict> inSeq,
                                        std::list< ::Test::StringStringDict>& outSeq,
                                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::E>
TestIntfI::opESeq(std::deque< ::Test::E> inSeq,
                        std::deque< ::Test::E>& outSeq,
                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::E>
TestIntfI::opEList(std::list< ::Test::E> inSeq,
                         std::list< ::Test::E>& outSeq,
                         const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::DPrxPtr>
TestIntfI::opDPrxSeq(std::deque< ::Test::DPrxPtr> inSeq,
                           std::deque< ::Test::DPrxPtr>& outSeq,
                           const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::DPrxPtr>
TestIntfI::opDPrxList(std::list< ::Test::DPrxPtr> inSeq,
                            std::list< ::Test::DPrxPtr>& outSeq,
                            const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::CPtr>
TestIntfI::opCSeq(std::deque< ::Test::CPtr> inSeq,
                  std::deque< ::Test::CPtr>& outSeq,
                  const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::CPtr>
TestIntfI::opCList(std::list< ::Test::CPtr> inSeq,
                   std::list< ::Test::CPtr>& outSeq,
                   const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

void
TestIntfI::opOutArrayByteSeq(Test::ByteSeq data, Test::ByteSeq& copy, const Ice::Current&)
{
    copy = data;
}

Test::IntStringDict
TestIntfI::opIntStringDict(Test::IntStringDict data, Test::IntStringDict& copy, const Ice::Current&)
{
    copy = data;
    return data;
}

Test::CustomMap<Ice::Long, Ice::Long>
TestIntfI::opVarDict(Test::CustomMap<std::string, Ice::Int> data,
                     Test::CustomMap<std::string, Ice::Int>& copy, const Ice::Current&)
{
    copy = data;

    Test::CustomMap<Ice::Long, Ice::Long> result;
    for(Ice::Long i = 0; i < 1000; ++i)
    {
        result[i] = i*i;
    }
    return result;
}

Test::CustomMap<Ice::Int, std::string>
TestIntfI::opCustomIntStringDict(
    std::map<Ice::Int, Util::string_view> data,
    Test::CustomMap<Ice::Int, std::string>& copy,
    const Ice::Current&)
{
    copy.clear();

    for(std::map<Ice::Int, Util::string_view>::const_iterator p = data.begin();
        p != data.end(); ++p)
    {
        copy[p->first] = p->second.to_string();
    }

    return copy;
}

Test::ShortBuffer
TestIntfI::opShortBuffer(Test::ShortBuffer inS, Test::ShortBuffer& outS, const Ice::Current&)
{
    outS = inS;
    return outS;
}

Test::CustomBuffer<bool>
TestIntfI::opBoolBuffer(Test::CustomBuffer<bool> inS, Test::CustomBuffer<bool>& outS, const Ice::Current&)
{
    outS = inS;
    return outS;
}

Test::BufferStruct
TestIntfI::opBufferStruct(Test::BufferStruct bs, const Ice::Current&)
{
    return bs;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
