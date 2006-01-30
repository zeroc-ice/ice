// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <deque>
#include <list>
#include <MyByteSeq.h>
#include <TestI.h>
#include <TestCommon.h>

TestIntfI::TestIntfI(const Ice::CommunicatorPtr& communicator)
    : _communicator(communicator)
{
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

void
TestIntfI::shutdown(const Ice::Current& current)
{
    _communicator->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}
