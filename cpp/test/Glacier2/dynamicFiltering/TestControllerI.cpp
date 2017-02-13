// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestControllerI.h>
#include <TestCommon.h>
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
TestControllerI::step(const Glacier2::SessionPrx& currentSession, const TestToken& currentState, TestToken& newState,
                      const Ice::Current&)
{
    switch(currentState.code)
    {
        case Test::Finished:
        {
            assert("TestController::step() shouldn't have been called with a state of Finished" == 0);
            break;
        }

        case Test::Running:
        {
            TestConfiguration& config = _configurations[currentState.config];
            assert(!config.description.empty());


            bool found = false;
            SessionTuple session;
            for(vector<SessionTuple>::const_iterator i = _sessions.begin(); i != _sessions.end() && !found; ++i)
            {
                if(i->session == currentSession)
                {
                    session = *i;
                    found = true;
                }
            }

            assert(found);

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
                    newState.code = Test::Finished;
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
                config = _configurations[newState.config];

                newState.description = config.description;
                newState.caseIndex = 0;
                reconfigure = true;
            }
            newState.expectedResult = config.cases[newState.caseIndex].expectedResult;
            newState.testReference = config.cases[newState.caseIndex].proxy;

            if(reconfigure)
            {
                Glacier2::StringSetPrx categories = session.sessionControl->categories();
                categories->add(config.categoryFiltersAccept);

                Glacier2::StringSetPrx adapterIds = session.sessionControl->adapterIds();
                adapterIds->add(config.adapterIdFiltersAccept);

                Glacier2::IdentitySetPrx ids = session.sessionControl->identities();
                ids->add(config.objectIdFiltersAccept);
                session.configured = true;
            }
            break;
        }

        default:
        {
            newState.code = Running;
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
TestControllerI::addSession(const SessionTuple& s)
{
    _sessions.push_back(s);
}

void
TestControllerI::notifyDestroy(const Glacier2::SessionControlPrx& control)
{
    for(vector<SessionTuple>::iterator i = _sessions.begin(); i != _sessions.end(); ++i)
    {
        if(i->sessionControl == control)
        {
            _sessions.erase(i);
            break;
        }
    }
}
