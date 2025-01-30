// Copyright (c) ZeroC, Inc.

#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

#include "Glacier2/Session.h"
#include "Test.h"
#include <string>
#include <vector>

struct SessionTuple
{
    std::optional<Glacier2::SessionPrx> session;
    std::optional<Glacier2::SessionControlPrx> sessionControl;
    bool configured = false;

    SessionTuple() = default;
    SessionTuple(std::optional<Glacier2::SessionPrx> s, std::optional<Glacier2::SessionControlPrx> control)
        : session(std::move(s)),
          sessionControl(std::move(control))
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

    TestCase(std::string s, const bool b) : proxy(std::move(s)), expectedResult(b) {}
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

    void step(
        std::optional<Glacier2::SessionPrx> currentSession,
        Test::TestToken currentState,
        Test::TestToken& newState,
        const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

    //
    // Internal methods.
    //
    void addSession(SessionTuple&&);

    void notifyDestroy(const std::optional<Glacier2::SessionControlPrx>&);

private:
    std::mutex _mutex;
    std::vector<SessionTuple> _sessions;
    std::vector<TestConfiguration> _configurations;
};

#endif
