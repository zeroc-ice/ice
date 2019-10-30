//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates file errors.
 **/
public class FileException extends SyscallException
{
    public FileException()
    {
        super();
        this.path = "";
    }

    public FileException(Throwable cause)
    {
        super(cause);
        this.path = "";
    }

    public FileException(int error, String path)
    {
        super(error);
        this.path = path;
    }

    public FileException(int error, String path, Throwable cause)
    {
        super(error, cause);
        this.path = path;
    }

    public String ice_id()
    {
        return "::Ice::FileException";
    }

    /**
     * The path of the file responsible for the error.
     **/
    public String path;

    /** @hidden */
    public static final long serialVersionUID = 8755315548941623583L;
}
