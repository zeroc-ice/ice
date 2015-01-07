// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

["objc:prefix:TestRetry"]
module Test
{

interface Retry
{
    void op(bool kill);
    void shutdown();
};

};
