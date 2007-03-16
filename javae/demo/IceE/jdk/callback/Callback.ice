// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

module Demo {

interface CallbackReceiver
{
    void callback();
};

interface CallbackSender
{
    void initiateCallback(CallbackReceiver* proxy);
    void shutdown();
};

};

#endif
