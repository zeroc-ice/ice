//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestControllerI.h>
#include <TestHelper.h>
#include <vector>
#include <string>

using namespace Ice;
using namespace Test;
using namespace std;

//
// TODO: More test cases.
//

TestControllerI::TestControllerI(const string& endpoint)
{
    TestConfiguration current;
    current.description = "No filters at all";
    current.cases.push_back(TestCase("foo/bar:" + endpoint, true));

    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Category filter";
    current.cases.push_back(TestCase("foo/barA:" + endpoint, true));
    current.cases.push_back(TestCase("bar/fooA:" + endpoint, false));
    current.cases.push_back(TestCase("\"a cat with spaces/fooX\":" + endpoint, true));
    current.categoryFiltersAccept.push_back("foo");
    current.categoryFiltersAccept.push_back("a cat with spaces");
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Adapter id filter";
    current.cases.push_back(TestCase("fooB @ bar", true));
    current.cases.push_back(TestCase("bazB @ baz", false));
    current.adapterIdFiltersAccept.push_back("bar");
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Object id filter";
    current.cases.push_back(TestCase("foo/barC:" + endpoint, true));
    current.cases.push_back(TestCase("bar/fooC:" + endpoint, false));
    Identity id;
    id.category = "foo";
    id.name = "barC";
    current.objectIdFiltersAccept.push_back(id);
    _configurations.push_back(current);
};

void
TestControllerI::step(shared_ptr<Glacier2::SessionPrx> currentSession, TestToken currentState, TestToken& newState,
                      const Ice::Current&)
{
    switch(currentState.code)
    {
        case Test::StateCode::Finished:
        {
            assert(false);
            break;
        }

        case Test::StateCode::Running:
        {
            TestConfiguration& config = _configurations[static_cast<size_t>(currentState.config)];
            assert(!config.description.empty());

            bool found = false;
            SessionTuple session;
            for(const auto& p : _sessions)
            {
                if(targetEqualTo(p.session, currentSession))
                {
                    session = p;
                    found = true;
                    break;
                }
            }

            test(found);

            //
            // New sessions force configuration step.
            //
            bool reconfigure = !session.configured;

            //
            // We start with the previous known state.
            //
            newState = currentState;

            ++newState.caseIndex;
            if(!(newState.caseIndex < (long)config.cases.size()))
            {
                //
                // We are out of test cases for this configuration. Move to
                // the next configuration.
                //
                ++newState.config;
                if(!(newState.config < (long)_configurations.size()))
                {
                    newState.code = Test::StateCode::Finished;
                    newState.expectedResult = false;
                    newState.description = "No more tests";
                    newState.testReference = "";
                    newState.config = -1;
                    newState.caseIndex = -1;
                    return;
                }

                //
                // New test configuration!
                //
                config = _configurations[static_cast<size_t>(newState.config)];

                newState.description = config.description;
                newState.caseIndex = 0;
                reconfigure = true;
            }
            newState.expectedResult = config.cases[static_cast<size_t>(newState.caseIndex)].expectedResult;
            newState.testReference = config.cases[static_cast<size_t>(newState.caseIndex)].proxy;

            if(reconfigure)
            {
                auto categories = session.sessionControl->categories();
                categories->add(config.categoryFiltersAccept);

                auto adapterIds = session.sessionControl->adapterIds();
                adapterIds->add(config.adapterIdFiltersAccept);

                auto ids = session.sessionControl->identities();
                ids->add(config.objectIdFiltersAccept);
                session.configured = true;
            }
            break;
        }

        default:
        {
            newState.code = Test::StateCode::Running;
            newState.config = 0;
            newState.caseIndex = 0;
            newState.testReference = "";
            newState.description = "Initial running state";
            newState.expectedResult = false;
            break;
        }
    }
}

void
TestControllerI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
TestControllerI::addSession(SessionTuple&& s)
{
    _sessions.emplace_back(move(s));
}

void
TestControllerI::notifyDestroy(const shared_ptr<Glacier2::SessionControlPrx>& control)
{
    for(auto i = _sessions.begin(); i != _sessions.end(); ++i)
    {
        if(targetEqualTo(i->sessionControl, control))
        {
            _sessions.erase(i);
            break;
        }
    }
}
