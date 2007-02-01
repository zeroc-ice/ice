// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public final class ConnectionInvalidException extends Ice.LocalException
{
    public ConnectionInvalidException()
    {
    }

    public ConnectionInvalidException(String reason)
    {
        this.reason = reason;
    }

    public String
    ice_name()
    {
        return "Ice::ConnectionInvalidException";
    }

    public String reason;
}
