// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Indicates the status of operation dispatch.
 *
 * @see DispatchInterceptor
 **/
public enum DispatchStatus implements java.io.Serializable
{
    /**
     * Indicates that an operation was dispatched synchronously and successfully.
     **/
    DispatchOK,

    /**
     * Indicates that an operation was dispatched synchronously and raised a user exception.
     **/
    DispatchUserException,

    /**
     * Indicates that an operation was dispatched asynchronously.
     **/
    DispatchAsync;

    public static final long serialVersionUID = 0L;
}
