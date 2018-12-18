// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Glacier2/Session.h>
#include <Test.h>
#include <vector>
#include <string>

struct SessionTuple
{
    Glacier2::SessionPrx session;
    Glacier2::SessionControlPrx sessionControl;
    bool configured;

    SessionTuple() {}
    SessionTuple(Glacier2::SessionPrx s, Glacier2::SessionControlPrx control):
        session(s),
        sessionControl(control),
        configured(false)
    {}

    SessionTuple&
    operator=(const SessionTuple& rhs)
    {
        if(this == &rhs)
        {
            return *this;
        }

        session = rhs.session;
        sessionControl = rhs.sessionControl;
        configured = rhs.configured;
        return *this;
    }
};

/*
 * The test controller manipulates the router's filter tables for this session.
 */

struct TestCase
{
    std::string proxy;
    bool expectedResult;

    TestCase(const std::string& s, const bool b) : proxy(s), expectedResult(b) {}
};

struct TestConfiguration
{
    std::string description;
    std::vector<TestCase> cases;
    std::vector<std::string> categoryFiltersAccept;
    std::vector<std::string> adapterIdFiltersAccept;
    std::vector<Ice::Identity> objectIdFiltersAccept;
};

//
// The test controller acts like a test server of sorts. It manages the
// configuration of the test's session and provides the client with test
// cases and expected outcomes.
//
class TestControllerI : public Test::TestController
{
public:
    TestControllerI(const std::string&);
    //
    // Slice to C++ mapping.
    //
    void step(const Glacier2::SessionPrx& currentSession, const Test::TestToken& currentState,
              Test::TestToken& newState, const Ice::Current&);

    void shutdown(const Ice::Current&);

    //
    // Internal methods.
    //
    void addSession(const SessionTuple&);

    void notifyDestroy(const Glacier2::SessionControlPrx&);

private:
    std::vector<SessionTuple> _sessions;
    std::vector<TestConfiguration> _configurations;

};

typedef IceUtil::Handle<TestControllerI> TestControllerIPtr;

#endif
