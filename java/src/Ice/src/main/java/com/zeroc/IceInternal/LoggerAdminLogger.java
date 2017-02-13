// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

interface LoggerAdminLogger extends com.zeroc.Ice.Logger
{
    com.zeroc.Ice.Object getFacet();
    void destroy();
}
