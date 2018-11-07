// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEGRID_API", "cpp:doxygen:include:IceGrid/IceGrid.h", "objc:header-dir:objc", "objc:dll-export:ICEGRID_API", "python:pkgdir:IceGrid"]]
[["cpp:include:IceGrid/Config.h"]]

#include <IceGrid/Admin.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEGRID", "js:module:ice"]
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
