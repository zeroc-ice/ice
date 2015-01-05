// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Metrics.ice>
module Demo
{

interface Hello
{
    idempotent void sayHello(int delay);
    void shutdown();
};

class MyMetrics extends IceMX::Metrics
{
    string foo;
};

};

