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

public class LocationForward extends Exception
{
    public
    LocationForward(ObjectPrx prx)
    {
        _prx = prx;
    }

    public ObjectPrx _prx;
}
