// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Glacier2/Session.ice>

module Test
{

interface Backend
{
    void check();
    void shutdown();
}

enum StateCode { Initial, Running, Finished }

struct TestToken
{
    bool expectedResult;
    string description;
    StateCode code;
    short config;
    short caseIndex;
    string testReference;
}

/**
 *
 * The test controller interface permits coordination between the test
 * server and the test client. Prior to each call the client makes on
 * various backend references, it calls step on the controller. The
 * controller will manage the configuration of the system and return a
 * flag indicating whether the next call is meant to succeed or not.
 *
 **/
interface TestController
{
    void step(Glacier2::Session* currentSession, TestToken currentState, out TestToken newState);
    void shutdown();
}

interface TestSession extends Glacier2::Session
{
    void shutdown();
}

}
