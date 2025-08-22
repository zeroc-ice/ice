// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Admin.ice"

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    /// The exception that is thrown when an error occurs during the parsing of an IceGrid XML file.
    exception ParseException
    {
        /// The reason for the failure.
        string reason;
    }

    /// `icegridadmin` provides a `FileParser` object to transform XML files into {@link ApplicationDescriptor} objects.
    interface FileParser
    {
        /// Parses a file.
        /// @param xmlFile The full path to the file.
        /// @param adminProxy An {@link Admin} proxy, used only to retrieve default templates when needed. May be null.
        /// @return The application descriptor.
        /// @throws ParseException Thrown when an error occurs during parsing.
        idempotent ApplicationDescriptor parse(string xmlFile, Admin* adminProxy)
            throws ParseException;
    }
}
