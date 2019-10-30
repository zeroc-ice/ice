//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if the type of an unmarshaled Slice class instance does
 * not match its expected type.
 * This can happen if client and server are compiled with mismatched Slice
 * definitions or if a class of the wrong type is passed as a parameter
 * or return value using dynamic invocation. This exception can also be
 * raised if IceStorm is used to send Slice class instances and
 * an operation is subscribed to the wrong topic.
 **/
public class UnexpectedObjectException extends MarshalException
{
    public UnexpectedObjectException()
    {
        super();
        this.type = "";
        this.expectedType = "";
    }

    public UnexpectedObjectException(Throwable cause)
    {
        super(cause);
        this.type = "";
        this.expectedType = "";
    }

    public UnexpectedObjectException(String reason, String type, String expectedType)
    {
        super(reason);
        this.type = type;
        this.expectedType = expectedType;
    }

    public UnexpectedObjectException(String reason, String type, String expectedType, Throwable cause)
    {
        super(reason, cause);
        this.type = type;
        this.expectedType = expectedType;
    }

    public String ice_id()
    {
        return "::Ice::UnexpectedObjectException";
    }

    /**
     * The Slice type ID of the class instance that was unmarshaled.
     **/
    public String type;

    /**
     * The Slice type ID that was expected by the receiving operation.
     **/
    public String expectedType;

    /** @hidden */
    public static final long serialVersionUID = -5786936875383180611L;
}
