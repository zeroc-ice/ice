// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

class CallbackReceiver
{
    void callback();
};

class Callback
{
    void initiateCallback(CallbackReceiver* proxy);
    void shutdown();
};

#endif
