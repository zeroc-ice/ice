// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

//
// LocationForward must be an unchecked exception, because it can
// be thrown from any servant operation
//
public class LocationForward extends RuntimeException
{
    public
    LocationForward(ObjectPrx prx)
    {
        _prx = prx;
    }

    public ObjectPrx _prx;
}
