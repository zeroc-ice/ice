// Copyright (c) ZeroC, Inc.
    
#pragma once
    
#include "Glacier2/Session.ice"
    
module Test
{
    interface Backend
    {
        void check();
        void shutdown();
    }
<<<<<<< Updated upstream
    enum StateCode { Initial, Running, Finished }
=======
        
    enum StateCode { Initial, Running, Finished }
        
>>>>>>> Stashed changes
    struct TestToken
    {
        bool expectedResult;
        string description;
        StateCode code;
        short config;
        short caseIndex;
        string testReference;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface TestSession extends Glacier2::Session
    {
        void shutdown();
    }
}
