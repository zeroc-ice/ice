//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

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
}

}
