// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
