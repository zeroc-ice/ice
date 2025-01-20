// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

interface LoggerAdminLogger extends Logger {
    com.zeroc.Ice.Object getFacet();

    void destroy();
}
