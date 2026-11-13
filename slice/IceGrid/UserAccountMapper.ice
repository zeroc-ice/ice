// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    /// The exception that is thrown when a user account for a given session identifier can't be found.
    exception UserAccountNotFoundException
    {
    }

    /// Maps user strings in server descriptors to actual user account names. You can configure the user account mapper
    /// of an IceGrid node with the property `IceGrid.Node.UserAccountMapper`.
    interface UserAccountMapper
    {
        /// Gets the name of the user account for the given user. This is used by IceGrid nodes to figure out the user
        /// account to use to run servers.
        /// @param user The value of the server descriptor's `user` attribute. When this attribute is not defined, and
        /// the server's activation mode is `session`, the default value for `user` is the session identifier.
        /// @return The user account name.
        /// @throws UserAccountNotFoundException Thrown when no user account is found for the given user.
        string getUserAccount(string user)
            throws UserAccountNotFoundException;
    }
}
