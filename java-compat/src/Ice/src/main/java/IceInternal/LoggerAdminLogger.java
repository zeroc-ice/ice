// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

interface LoggerAdminLogger extends Ice.Logger
{
    Ice.Object getFacet();
    void destroy();
}
