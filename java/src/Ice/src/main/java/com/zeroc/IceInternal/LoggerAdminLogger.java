//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

interface LoggerAdminLogger extends com.zeroc.Ice.Logger
{
    com.zeroc.Ice.Object getFacet();
    void destroy();
}
