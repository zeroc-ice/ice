//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

interface LoggerAdminLogger extends Logger {
    com.zeroc.Ice.Object getFacet();

    void destroy();
}
