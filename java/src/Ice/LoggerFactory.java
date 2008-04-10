// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * @deprecated LoggerFactory is deprecated, use Ice.LoggerPlugin instead.
 **/
public interface LoggerFactory
{
    Logger create(Communicator communicator, String[] args);
}
