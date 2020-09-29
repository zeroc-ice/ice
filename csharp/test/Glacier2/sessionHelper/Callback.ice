//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Glacier2::Test::SessionHelper
{

exception CallbackException
{
    double someValue;
    string someString;
}

interface CallbackReceiver
{
    void callback();

    void callbackEx();
}

interface Callback
{
    void initiateCallback(CallbackReceiver* proxy);

    void initiateCallbackEx(CallbackReceiver* proxy);

    void shutdown();
}

}
