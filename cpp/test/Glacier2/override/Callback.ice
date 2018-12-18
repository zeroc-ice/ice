// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
