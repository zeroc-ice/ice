// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface UserExceptionFactory
{
    void createAndThrow(String type)
        throws UserException;

    void destroy();
}
