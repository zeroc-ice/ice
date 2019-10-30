//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if no suitable value factory was found during
 * unmarshaling of a Slice class instance.
 *
 * @see ValueFactory
 * @see Communicator#getValueFactoryManager
 * @see ValueFactoryManager#add
 * @see ValueFactoryManager#find
 **/
public class NoValueFactoryException extends MarshalException
{
    public NoValueFactoryException()
    {
        super();
        this.type = "";
    }

    public NoValueFactoryException(Throwable cause)
    {
        super(cause);
        this.type = "";
    }

    public NoValueFactoryException(String reason, String type)
    {
        super(reason);
        this.type = type;
    }

    public NoValueFactoryException(String reason, String type, Throwable cause)
    {
        super(reason, cause);
        this.type = type;
    }

    public String ice_id()
    {
        return "::Ice::NoValueFactoryException";
    }

    /**
     * The Slice type ID of the class instance for which no
     * no factory could be found.
     **/
    public String type;

    /** @hidden */
    public static final long serialVersionUID = -4888152001471748622L;
}
