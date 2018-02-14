// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Glacier2.application"]]
module Test
{

interface CallbackReceiver
{
    void callback();
}

interface Callback
{
    void initiateCallback(CallbackReceiver* proxy);
    void shutdown();
}

}
