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

    public static Evictor
    createEvictor(Ice.ObjectAdapter adapter, String envName, String filename, 
		  ServantInitializer initializer, Index[] indices, boolean createDb)
    {
	return new EvictorI(adapter, envName, filename, initializer, indices, createDb);
    } 

    public static Evictor
    createEvictor(Ice.ObjectAdapter adapter, String envName, 
		  com.sleepycat.db.DbEnv dbEnv, String filename, 
		  ServantInitializer initializer, Index[] indices, boolean createDb)
    {
	return new EvictorI(adapter, envName, dbEnv, filename, initializer, indices, createDb);
    } 

    public static Connection
    createConnection(Ice.Communicator communicator, String envName)
    {
	return new ConnectionI(communicator, envName);
    } 

    public static Connection
    createConnection(Ice.Communicator communicator, String envName, com.sleepycat.db.DbEnv dbEnv)
    {
	return new ConnectionI(communicator, envName, dbEnv);
    } 

}

