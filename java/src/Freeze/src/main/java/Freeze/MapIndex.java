// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public interface MapIndex
{
    String name();

    void associate(String dbName, com.sleepycat.db.Database db, com.sleepycat.db.Transaction txn, boolean createDb)
        throws com.sleepycat.db.DatabaseException, java.io.FileNotFoundException;

    void init(MapIndex i);

    void close();
}
