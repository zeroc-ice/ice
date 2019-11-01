//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

#include <Glacier2/Session.h>
#include <Test.h>
#include <vector>
#include <string>

struct SessionTuple
{
    std::shared_ptr<Glacier2::SessionPrx> session;
    std::shared_ptr<Glacier2::SessionControlPrx> sessionControl;
    bool configured = false;

    SessionTuple() = default;
    SessionTuple(std::shared_ptr<Glacier2::SessionPrx> s, std::shared_ptr<Glacier2::SessionControlPrx> control):
        session(move(s)),
        sessionControl(move(control)),
        configured(false)
    {
    }

    SessionTuple(const SessionTuple&) = delete;
    SessionTuple& operator=(const SessionTuple&) = default;
    SessionTuple(SessionTuple&&) = default;
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
class TestControllerI final : public Test::TestController
{
public:
    TestControllerI(const std::string&);

    void step(std::shared_ptr<Glacier2::SessionPrx> currentSession, Test::TestToken currentState,
              Test::TestToken& newState, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

    //
    // Internal methods.
    //
    void addSession(SessionTuple&&);

    void notifyDestroy(const std::shared_ptr<Glacier2::SessionControlPrx>&);

private:
    std::vector<SessionTuple> _sessions;
    std::vector<TestConfiguration> _configurations;

};

#endif
