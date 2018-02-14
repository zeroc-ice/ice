// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Creates a UserExceptionReader for extracting a user exception
 * from an input stream.
 *
 * @see InputStream
 **/
public interface UserExceptionReaderFactory
{
    /**
     * Creates and throws a UserExceptionReader instance.
     *
     * @param typeId The Slice type ID of the user exception to be instantiated.
     **/
    void createAndThrow(String typeId)
        throws UserExceptionReader;
}
