// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <Ice/ConnectionF.ice>
#include <Ice/Current.ice>

module Ice
{

local interface ObjectObserver
{
   void attach();
   void detach();
};
 
enum ObserverThreadState
{
    ThreadIdle,
    ThreadInUse,
    ThreadInUseForIO,
};

local interface ThreadPoolThreadObserver extends ObjectObserver
{
    void stateChanged(ObserverThreadState oldState, ObserverThreadState newState);
};
 
local interface RequestObserver extends ObjectObserver
{
   void reponseOK();
   void reponseUserException();
   void reponseSystemException();
   void failure();
   void marshalTime(long time);
   void unmarshalTime(long time);
   void callTime(long time);
};
 
enum ObserverConnectionState
{
    ObserverConnectionStateInitializing,
    ObserverConnectionStateHolding,
    ObserverConnectionStateActive,
    ObserverConnectionStateClosing,
    ObserverConnectionStateClosed
};

local interface ConnectionObserver extends ObjectObserver
{
    void stateChanged(ObserverConnectionState oldState, ObserverConnectionState newState);
    void sentBytes(int num, long duration);
    void receivedBytes(int num, long duration);
};

local interface ObserverUpdater
{
    void updateConnectionObservers();
    void updateThreadObservers();
    void updateThreadPoolThreadObservers();
};

local interface ObserverResolver
{
    void setObserverUpdater(ObserverUpdater updater);
    
    ConnectionObserver getConnectionObserver(Connection con, ConnectionObserver old);
    ObjectObserver getThreadObserver(string parent, string id, ObjectObserver old);
    ThreadPoolThreadObserver getThreadPoolThreadObserver(string parent, string id, ThreadPoolThreadObserver old);

    RequestObserver getInvocationObserver(Object* prx, string operation);
    RequestObserver getDispatchObserver(Object obj, Current c); 
};
    
};

