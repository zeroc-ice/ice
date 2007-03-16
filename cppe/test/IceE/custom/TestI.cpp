// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <TestCommon.h>

TestIntfI::TestIntfI(const Ice::CommunicatorPtr& communicator)
    : _communicator(communicator)
{
}

Test::BoolSeq
TestIntfI::opBoolArray(const std::pair<const bool*, const bool*>& inSeq,
		       Test::BoolSeq& outSeq,
		       const Ice::Current& current)
{
    Test::BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::ByteList
TestIntfI::opByteArray(const std::pair<const Ice::Byte*, const Ice::Byte*>& inSeq,
		       Test::ByteList& outSeq,
		       const Ice::Current& current)
{
    Test::ByteList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

Test::VariableList
TestIntfI::opVariableArray(const std::pair<const Test::Variable*, const Test::Variable*>& inSeq,
		           Test::VariableList& outSeq,
		           const Ice::Current& current)
{
    Test::VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

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

Test::BoolSeq
TestIntfI::opBoolRangeType(const std::pair<const bool*, const bool*>& inSeq,
		           Test::BoolSeq& outSeq,
		           const Ice::Current&)
{
    Test::BoolSeq(inSeq.first, inSeq.second).swap(outSeq);
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
    Test::VariableList(inSeq.first, inSeq.second).swap(outSeq);
    return outSeq;
}

std::deque<bool>
TestIntfI::opBoolSeq(const std::deque<bool>& inSeq,
		     std::deque<bool>& outSeq,
		     const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list<bool>
TestIntfI::opBoolList(const std::list<bool>& inSeq,
		      std::list<bool>& outSeq,
		      const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Ice::Byte>
TestIntfI::opByteSeq(const std::deque< ::Ice::Byte>& inSeq,
		     std::deque< ::Ice::Byte>& outSeq,
		     const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Ice::Byte>
TestIntfI::opByteList(const std::list< ::Ice::Byte>& inSeq,
		      std::list< ::Ice::Byte>& outSeq,
		      const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

MyByteSeq
TestIntfI::opMyByteSeq(const MyByteSeq& inSeq,
		       MyByteSeq& outSeq,
		       const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::std::string>
TestIntfI::opStringSeq(const std::deque< ::std::string>& inSeq,
		       std::deque< ::std::string>& outSeq,
		       const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::std::string>
TestIntfI::opStringList(const std::list< ::std::string>& inSeq,
		        std::list< ::std::string>& outSeq,
		        const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Fixed>
TestIntfI::opFixedSeq(const std::deque< ::Test::Fixed>& inSeq,
		      std::deque< ::Test::Fixed>& outSeq,
		      const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Fixed>
TestIntfI::opFixedList(const std::list< ::Test::Fixed>& inSeq,
		       std::list< ::Test::Fixed>& outSeq,
		       const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::Variable>
TestIntfI::opVariableSeq(const std::deque< ::Test::Variable>& inSeq,
		         std::deque< ::Test::Variable>& outSeq,
		         const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::Variable>
TestIntfI::opVariableList(const std::list< ::Test::Variable>& inSeq,
			  std::list< ::Test::Variable>& outSeq,
			  const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::StringStringDict>
TestIntfI::opStringStringDictSeq(const std::deque< ::Test::StringStringDict>& inSeq,
                                       std::deque< ::Test::StringStringDict>& outSeq,
                                       const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::StringStringDict>
TestIntfI::opStringStringDictList(const std::list< ::Test::StringStringDict>& inSeq,
                                        std::list< ::Test::StringStringDict>& outSeq,
                                        const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::E>
TestIntfI::opESeq(const std::deque< ::Test::E>& inSeq,
                        std::deque< ::Test::E>& outSeq,
                        const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::E>
TestIntfI::opEList(const std::list< ::Test::E>& inSeq,
                         std::list< ::Test::E>& outSeq,
                         const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::deque< ::Test::CPrx>
TestIntfI::opCPrxSeq(const std::deque< ::Test::CPrx>& inSeq,
                           std::deque< ::Test::CPrx>& outSeq,
                           const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

std::list< ::Test::CPrx>
TestIntfI::opCPrxList(const std::list< ::Test::CPrx>& inSeq,
                            std::list< ::Test::CPrx>& outSeq,
                            const Ice::Current& current)
{
    outSeq = inSeq;
    return inSeq;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    _communicator->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}
