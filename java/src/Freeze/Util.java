// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

public class Util
{
    public static DBEnvironment
    initialize(Ice.Communicator communicator, String name)
    {
	return new DBEnvironmentI(communicator, name);
    }
}

