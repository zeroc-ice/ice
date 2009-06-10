// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

/**
 * Indicates that a connection is not an SSL connection.
 **/
public final class ConnectionInvalidException extends Ice.LocalException
{
    /**
     * Creates an instance with a <code>null</code> <code>reason</code> field.
     **/
    public ConnectionInvalidException()
    {
    }

    /**
     * Creates an instance with the specified <code>reason</code>field.
     *
     * @param reason The reason why the connection is considered invalid.
     **/
    public ConnectionInvalidException(String reason)
    {
        this.reason = reason;
    }

    /**
     * Returns the name of this exception.
     *
     * @return Returns <code>"Ice::ConnectionInvalidException"</code>.
     **/
    public String
    ice_name()
    {
        return "Ice::ConnectionInvalidException";
    }

    /**
     * The reason why the connection is considered invalid.
     **/
    public String reason;
}
