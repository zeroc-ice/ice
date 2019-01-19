//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceGrid/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICEGRID_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:IceGrid"]]

#include <IceGrid/Admin.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * This exception is raised if an error occurs during parsing.
 *
 **/
exception ParseException
{
    /** The reason for the failure. */
    string reason;
}

/**
 *
 * <code>icegridadmin</code> provides a {@link FileParser}
 * object to transform XML files into {@link ApplicationDescriptor}
 * objects.
 *
 **/
interface FileParser
{

    /**
     *
     * Parse a file.
     *
     * @param xmlFile Full pathname to the file.
     *
     * @param adminProxy An Admin proxy, used only to retrieve default
     * templates when needed. May be null.
     *
     * @return The application descriptor.
     *
     * @throws ParseException Raised if an error occurred during parsing.
     *
     **/

    idempotent ApplicationDescriptor parse(string xmlFile, Admin* adminProxy)
        throws ParseException;
}

}
