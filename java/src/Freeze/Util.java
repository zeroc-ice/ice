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
    createEvictor(Ice.Communicator communicator, String envName, String dbName, boolean createDb)
    {
	return new EvictorI(communicator, envName, dbName, createDb);
    } 

    public static Evictor
    createEvictor(Ice.Communicator communicator, String envName, 
		  com.sleepycat.db.DbEnv dbEnv, String dbName, boolean createDb)
    {
	return new EvictorI(communicator, envName, dbEnv, dbName, createDb);
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

