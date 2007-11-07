// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_FILE_PARSER_ICE
#define ICE_GRID_FILE_PARSER_ICE

#include <IceGrid/Admin.ice>

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
};
    
/**
 *
 * <tt>icegridadmin</tt> provides a [FileParser]
 * object to transform XML files into [ApplicationDescriptor]
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
     * @returns The application descriptor.
     *
     * @throws ParseException Raised if an error occurred during parsing.
     *
     **/

    idempotent ApplicationDescriptor parse(string xmlFile, Admin* adminProxy)
        throws ParseException;
};

};

#endif
