// **********************************************************************
//
// Copyright (c) 2003
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

package Freeze;

public class Util
{
    public static DBEnvironment
    initialize(Ice.Communicator communicator, String name)
    {
	return new DBEnvironmentI(communicator, name, false);
    }

    public static DBEnvironment
    initializeWithTxn(Ice.Communicator communicator, String name)
    {
	return new DBEnvironmentI(communicator, name, true);
    }
}

