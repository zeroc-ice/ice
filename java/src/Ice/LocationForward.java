// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
