// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

interface LoggerAdminLogger extends Logger {
    Object getFacet();

    void destroy();
}
