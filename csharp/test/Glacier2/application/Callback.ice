//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Glacier2.sessionHelper"]]
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
