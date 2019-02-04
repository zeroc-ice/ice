//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

interface LoggerAdminLogger extends Ice.Logger
{
    Ice.Object getFacet();
    void destroy();
}
