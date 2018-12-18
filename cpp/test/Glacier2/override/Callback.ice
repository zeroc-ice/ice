// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

module Test
{

interface CallbackReceiver
{
    void callback(int token);

    void callbackWithPayload(Ice::ByteSeq payload);
}

interface Callback
{
    ["amd"] void initiateCallback(CallbackReceiver* proxy, int token);

    ["amd"] void initiateCallbackWithPayload(CallbackReceiver* proxy);

    void shutdown();
}

}
