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
 
enum ThreadState
{
    ThreadIddle,
    ThreadInUse,
    ThreadInUseForIO,
};

local interface ThreadPoolThreadObserver extends ObjectObserver
{
    void stateChanged(ThreadState oldState, ThreadState newState);
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
 
enum ConnectionState
{
    ConnectionStateInitializing,
    ConnectionStateHolding,
    ConnectionStateActive,
    ConnectionStateClosing,
    ConnectionStateClosed
};

local interface ConnectionObserver extends ObjectObserver
{
    void stateChanged(ConnectionState oldState, ConnectionState newState);
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
    void setObserverUpdater(ObserverUpdater refresher);
    
    ConnectionObserver getConnectionObserver(ConnectionObserver old, Connection con);
    ObjectObserver getThreadObserver(ObjectObserver old, string parent, string id);
    ThreadPoolThreadObserver getThreadPoolThreadObserver(ThreadPoolThreadObserver old, string parent, string id);
    RequestObserver getInvocationObserver(RequestObserver old, Object* prx, string operation);
    RequestObserver getDispatchObserver(RequestObserver old, Object obj, Current c); 
};
    
};

