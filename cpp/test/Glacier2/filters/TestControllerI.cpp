// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestControllerI.h>
#include <vector>
#include <string>

using namespace Ice;
using namespace Test;
using namespace std;

TestControllerI::TestControllerI()
{
    TestConfiguration current;
    current.description = "No filters at all";
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", true));

    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Category filter (accept)";
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("bar/foo:default -h 127.0.0.1 -p 12012", false));
    current.categoryFiltersAccept.push_back("foo");
    _configurations.push_back(current);
 
    current = TestConfiguration();
    current.description = "Category filter (reject)";
    current.cases.push_back(TestCase("bar/foo:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", false));
    current.categoryFiltersReject.push_back("foo");
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Adapter id filter (accept)";
    current.cases.push_back(TestCase("foo @ bar", true));
    current.cases.push_back(TestCase("baz @ baz", false));
    current.adapterIdFiltersAccept.push_back("bar");
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Adapter id filter (reject)";
    current.cases.push_back(TestCase("foo @ bar", true));
    current.cases.push_back(TestCase("baz @ baz", false));
    current.adapterIdFiltersReject.push_back("baz");
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Object id filter (accept)";
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("bar/foo:default -h 127.0.0.1 -p 12012", false));
    Identity id;
    id.category = "foo";
    id.name = "bar";
    current.objectIdFiltersAccept.push_back(id);
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Object id filter (accept)";
    current.cases.push_back(TestCase("bar/foo:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", false));
    current.objectIdFiltersReject.push_back(id);
    _configurations.push_back(current);

    //
    // TODO: 
    // Exhaustive combination of accept override flags.
    //
    current = TestConfiguration();
    current.description = "Category filter (accept override == true)";
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("bar/foo:default -h 127.0.0.1 -p 12012", false));
    current.categoryFiltersReject.push_back("foo");
    current.categoryFiltersReject.push_back("bar");
    current.categoryFiltersAccept.push_back("foo");
    current.categoryFilterAcceptOverride = true;
    _configurations.push_back(current);

    current = TestConfiguration();
    current.description = "Mixed filters (category + adapter id)";
    current.cases.push_back(TestCase("foo/bar:default -h 127.0.0.1 -p 12012", false));
    current.cases.push_back(TestCase("bar/tide:default -h 127.0.0.1 -p 12012", true));
    current.cases.push_back(TestCase("cat/manx @ baz", false));
    current.cases.push_back(TestCase("bar/siamese @ other", true));
    current.categoryFiltersAccept.push_back("bar");
    current.adapterIdFiltersReject.push_back("baz");
    _configurations.push_back(current);
};

void
TestControllerI::step(const Glacier2::SessionPrx& currentSession, const TestToken& currentState, TestToken& newState,
		      const Ice::Current& current)
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
	    bool reconfigure = session.configured;

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
		Glacier2::StringFilterPrx catFilter = session.sessionControl->categoryFilter();
		catFilter->setAccept(config.categoryFiltersAccept);
		catFilter->setReject(config.categoryFiltersReject);
		catFilter->setAcceptOverride(config.categoryFilterAcceptOverride);

		Glacier2::StringFilterPrx adapterFilter = session.sessionControl->adapterIdFilter();
		adapterFilter->setAccept(config.adapterIdFiltersAccept);
		adapterFilter->setReject(config.adapterIdFiltersReject);
		adapterFilter->setAcceptOverride(config.adapterIdAcceptOverride);
			
		Glacier2::IdFilterPrx idFilter = session.sessionControl->objectIdFilter();
		idFilter->setAccept(config.objectIdFiltersAccept);
		idFilter->setReject(config.objectIdFiltersReject);
		idFilter->setAcceptOverride(config.objectIdAcceptOverride);
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
