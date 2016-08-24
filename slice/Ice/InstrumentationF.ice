// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/JavaCompat.ice>

#ifndef JAVA_COMPAT
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

["objc:prefix:ICEINSTRUMENTATION"]
module Instrumentation
{

local interface Observer;
local interface CommunicatorObserver;

};

};
