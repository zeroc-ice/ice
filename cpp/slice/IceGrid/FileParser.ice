// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
 * This exception is raised when a file-parsing fails
 *
 **/
exception ParseException
{
    string reason;
};
    
/**
 *
 * icegridadmin provides a FileParser object to transform XML files
 * into ApplicationDescriptor objects (structs).
 *
 **/

interface FileParser
{

    /**
     *
     * Parse a file
     *
     * @param file Full pathname to the file.
     *
     * @param adminProxy An Admin proxy, used only to retrieve default templates when needed. 
     *  May be null.
     *
     * @returns The application descriptor.
     *
     * @throws ParseException Raised if an error occured during parsing.
     *
     **/

    idempotent ApplicationDescriptor parse(string xmlFile, Admin* adminProxy)
	throws ParseException;
};

};

#endif
