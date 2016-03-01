// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * An administrative interface for process management. Managed servers must
 * implement this interface.
 *
 * <p class="Note">A servant implementing this interface is a potential target
 * for denial-of-service attacks, therefore proper security precautions
 * should be taken. For example, the servant can use a UUID to make its
 * identity harder to guess, and be registered in an object adapter with
 * a secured endpoint.
 *
 **/
interface Process
{
    /**
     *
     * Initiate a graceful shut-down.
     *
     * @see Communicator#shutdown
     *
     **/
    void shutdown();


    /**
     *
     * Write a message on the process' stdout or stderr.
     *
     * @param message The message.
     *
     * @param fd 1 for stdout, 2 for stderr.
     *
     **/
    void writeMessage(string message, int fd);
};

};

