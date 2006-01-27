// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : virtual public Test::TestIntf
{
public:

    TestIntfI(const Ice::CommunicatorPtr&);

    virtual std::deque<bool> opBoolSeq(const std::deque<bool>&,
				       std::deque<bool>&,
				       const Ice::Current&);

    virtual std::list<bool> opBoolList(const std::list<bool>&,
				       std::list<bool>&,
				       const Ice::Current&);

    virtual std::deque< ::Ice::Byte> opByteSeq(const std::deque< ::Ice::Byte>&,
					       std::deque< ::Ice::Byte>&,
					       const Ice::Current&);

    virtual std::list< ::Ice::Byte> opByteList(const std::list< ::Ice::Byte>&,
					       std::list< ::Ice::Byte>&,
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

    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
