// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <TestI.h>

Test::DoubleSeq
TestIntfI::opDoubleArray(ICE_IN(std::pair<const Ice::Double*, const Ice::Double*>) inSeq,
                         Test::DoubleSeq& outSeq,
                         const Ice::Current&)
{
    Test::DoubleSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::BoolSeq
TestIntfI::opBoolArray(ICE_IN(std::pair<const bool*, const bool*>) inSeq,
                       Test::BoolSeq& outSeq,
                       const Ice::Current&)
{
    Test::BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteArray(ICE_IN(std::pair<const Ice::Byte*, const Ice::Byte*>) inSeq,
                       Test::ByteList& outSeq,
                       const Ice::Current&)
{
    Test::ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableArray(ICE_IN(std::pair<const Test::Variable*, const Test::Variable*>) inSeq,
                           Test::VariableList& outSeq,
                           const Ice::Current&)
{
    Test::VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

#ifdef ICE_CPP11_MAPPING

Test::BoolSeq
TestIntfI::opBoolRange(Test::BoolSeq inSeq, Test::BoolSeq& outSeq, const Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteRange(Test::ByteList inSeq, Test::ByteList& outSeq, const Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableRange(Test::VariableList inSeq, Test::VariableList& outSeq, const Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteRangeType(Test::ByteList inSeq, Test::ByteList& outSeq, const Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableRangeType(Test::VariableList inSeq, Test::VariableList& outSeq, const Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}

#else

Test::BoolSeq
TestIntfI::opBoolRange(const std::pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& inSeq,
                       Test::BoolSeq& outSeq,
                       const Ice::Current&)
{
    Test::BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteRange(const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                       Test::ByteList& outSeq,
                       const Ice::Current&)
{
    Test::ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableRange(
    const std::pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator>& inSeq,
    Test::VariableList& outSeq,
    const Ice::Current&)
{
    Test::VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteRangeType(const std::pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& inSeq,
                           Test::ByteList& outSeq,
                           const Ice::Current&)
{
    Test::ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableRangeType(
    const std::pair<std::deque<Test::Variable>::const_iterator, std::deque<Test::Variable>::const_iterator>& inSeq,
    Test::VariableList& outSeq,
    const Ice::Current&)
{
    for(std::deque<Test::Variable>::const_iterator p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    return outSeq;
}

#endif

std::deque<bool>
TestIntfI::opBoolSeq(ICE_IN(std::deque<bool>) inSeq,
                     std::deque<bool>& outSeq,
                     const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list<bool>
TestIntfI::opBoolList(ICE_IN(std::list<bool>) inSeq,
                      std::list<bool>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

::Test::BoolDequeList
TestIntfI::opBoolDequeList(ICE_IN(::Test::BoolDequeList) inSeq, ::Test::BoolDequeList& outSeq, const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

::Test::BoolDequeList
TestIntfI::opBoolDequeListArray(ICE_IN(::std::pair<const std::deque<bool>*, const std::deque<bool>*>) inSeq,
                                ::Test::BoolDequeList& outSeq,
                                const ::Ice::Current&)
{
    for(const std::deque<bool>* p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    return outSeq;
}

#ifdef ICE_CPP11_MAPPING
Test::BoolDequeList
TestIntfI::opBoolDequeListRange(Test::BoolDequeList inSeq, Test::BoolDequeList& outSeq, const ::Ice::Current&)
{
    outSeq.swap(inSeq);
    return outSeq;
}
#else
::Test::BoolDequeList
TestIntfI::opBoolDequeListRange(const ::std::pair< ::Test::BoolDequeList::const_iterator,
                                                   ::Test::BoolDequeList::const_iterator>& inSeq,
                                ::Test::BoolDequeList& outSeq,
                                const ::Ice::Current&)
{
    for(::Test::BoolDequeList::const_iterator p = inSeq.first; p != inSeq.second; ++p)
    {
        outSeq.push_back(*p);
    }
    return outSeq;
}
#endif

std::deque< ::Ice::Byte>
TestIntfI::opByteSeq(ICE_IN(std::deque< ::Ice::Byte>) inSeq,
                     std::deque< ::Ice::Byte>& outSeq,
                     const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Ice::Byte>
TestIntfI::opByteList(ICE_IN(std::list< ::Ice::Byte>) inSeq,
                      std::list< ::Ice::Byte>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

MyByteSeq
TestIntfI::opMyByteSeq(ICE_IN(MyByteSeq) inSeq,
                       MyByteSeq& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::string
TestIntfI::opString(ICE_IN(Util::string_view) inString,
                    std::string& outString,
                    const Ice::Current&)
{
    outString = inString.to_string();
    return outString;
}

std::deque< ::std::string>
TestIntfI::opStringSeq(ICE_IN(std::deque< ::std::string>) inSeq,
                       std::deque< ::std::string>& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::std::string>
TestIntfI::opStringList(ICE_IN(std::list< ::std::string>) inSeq,
                        std::list< ::std::string>& outSeq,
                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Fixed>
TestIntfI::opFixedSeq(ICE_IN(std::deque< ::Test::Fixed>) inSeq,
                      std::deque< ::Test::Fixed>& outSeq,
                      const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Fixed>
TestIntfI::opFixedList(ICE_IN(std::list< ::Test::Fixed>) inSeq,
                       std::list< ::Test::Fixed>& outSeq,
                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Variable>
TestIntfI::opVariableSeq(ICE_IN(std::deque< ::Test::Variable>) inSeq,
                         std::deque< ::Test::Variable>& outSeq,
                         const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Variable>
TestIntfI::opVariableList(ICE_IN(std::list< ::Test::Variable>) inSeq,
                          std::list< ::Test::Variable>& outSeq,
                          const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::StringStringDict>
TestIntfI::opStringStringDictSeq(ICE_IN(std::deque< ::Test::StringStringDict>) inSeq,
                                       std::deque< ::Test::StringStringDict>& outSeq,
                                       const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::StringStringDict>
TestIntfI::opStringStringDictList(ICE_IN(std::list< ::Test::StringStringDict>) inSeq,
                                        std::list< ::Test::StringStringDict>& outSeq,
                                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::E>
TestIntfI::opESeq(ICE_IN(std::deque< ::Test::E>) inSeq,
                        std::deque< ::Test::E>& outSeq,
                        const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::E>
TestIntfI::opEList(ICE_IN(std::list< ::Test::E>) inSeq,
                         std::list< ::Test::E>& outSeq,
                         const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::DPrxPtr>
TestIntfI::opDPrxSeq(ICE_IN(std::deque< ::Test::DPrxPtr>) inSeq,
                           std::deque< ::Test::DPrxPtr>& outSeq,
                           const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::DPrxPtr>
TestIntfI::opDPrxList(ICE_IN(std::list< ::Test::DPrxPtr>) inSeq,
                            std::list< ::Test::DPrxPtr>& outSeq,
                            const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::CPtr>
TestIntfI::opCSeq(ICE_IN(std::deque< ::Test::CPtr>) inSeq,
                  std::deque< ::Test::CPtr>& outSeq,
                  const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::CPtr>
TestIntfI::opCList(ICE_IN(std::list< ::Test::CPtr>) inSeq,
                   std::list< ::Test::CPtr>& outSeq,
                   const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

#ifdef ICE_CPP11_MAPPING
Test::ClassStruct
TestIntfI::opClassStruct(Test::ClassStruct inS,
                         Test::ClassStructSeq inSeq,
                         Test::ClassStruct& out,
                         Test::ClassStructSeq& outSeq,
                         const Ice::Current&)
{
    outSeq = std::move(inSeq);
    out = inS;
    return inS;
}

#else
::Test::ClassStructPtr
TestIntfI::opClassStruct(const ::Test::ClassStructPtr& inS,
                         const ::Test::ClassStructSeq& inSeq,
                         ::Test::ClassStructPtr& out,
                         ::Test::ClassStructSeq& outSeq,
                         const Ice::Current&)
{
    outSeq = inSeq;
    out = inS;
    return inS;
}
#endif

void
TestIntfI::opOutArrayByteSeq(ICE_IN(Test::ByteSeq) data, Test::ByteSeq& copy, const Ice::Current&)
{
    copy = data;
}

void
TestIntfI::opOutRangeByteSeq(ICE_IN(Test::ByteSeq) data, Test::ByteSeq& copy, const Ice::Current&)
{
    copy = data;
}

Test::IntStringDict
TestIntfI::opIntStringDict(ICE_IN(Test::IntStringDict) data, Test::IntStringDict& copy, const Ice::Current&)
{
    copy = data;
    return data;
}

Test::CustomMap<Ice::Long, Ice::Long>
TestIntfI::opVarDict(ICE_IN(Test::CustomMap<std::string, Ice::Int>) data,
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
    ICE_IN(std::map<Ice::Int, Util::string_view>) data,
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
TestIntfI::opShortBuffer(ICE_IN(Test::ShortBuffer) inS, Test::ShortBuffer& outS, const Ice::Current&)
{
    outS = inS;
    return outS;
}

Test::CustomBuffer<bool>
TestIntfI::opBoolBuffer(ICE_IN(Test::CustomBuffer<bool>) inS, Test::CustomBuffer<bool>& outS, const Ice::Current&)
{
    outS = inS;
    return outS;
}

Test::BufferStruct
TestIntfI::opBufferStruct(ICE_IN(Test::BufferStruct) bs, const Ice::Current&)
{
    return bs;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
