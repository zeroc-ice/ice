// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** A logger that works in tandem with a "Logger" admin facet. */
interface LoggerAdminLogger extends Logger {
    Object getFacet();

    void detach();
}
