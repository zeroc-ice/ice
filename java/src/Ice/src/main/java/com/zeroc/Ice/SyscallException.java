//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if a system error occurred in the server
 * or client process. There are many possible causes for such a system
 * exception. For details on the cause, {@link SyscallException#error}
 * should be inspected.
 **/
public class SyscallException extends LocalException
{
    public SyscallException()
    {
        this.error = 0;
    }

    public SyscallException(Throwable cause)
    {
        super(cause);
        this.error = 0;
    }

    public SyscallException(int error)
    {
        this.error = error;
    }

    public SyscallException(int error, Throwable cause)
    {
        super(cause);
        this.error = error;
    }

    public String ice_id()
    {
        return "::Ice::SyscallException";
    }

    /**
     * The error number describing the system exception. For C++ and
     * Unix, this is equivalent to <code>errno</code>. For C++
     * and Windows, this is the value returned by
     * <code>GetLastError()</code> or
     * <code>WSAGetLastError()</code>.
     **/
    public int error;

    /** @hidden */
    public static final long serialVersionUID = -2440066513892919497L;
}
