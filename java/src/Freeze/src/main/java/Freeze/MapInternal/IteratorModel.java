// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

interface IteratorModel<K, V>
{
    String dbName();
    TraceLevels traceLevels();

    com.sleepycat.db.Cursor openCursor()
        throws com.sleepycat.db.DatabaseException;

    EntryI<K, V> firstEntry(com.sleepycat.db.Cursor cursor)
        throws com.sleepycat.db.DatabaseException;

    EntryI<K, V> nextEntry(com.sleepycat.db.Cursor cursor)
        throws com.sleepycat.db.DatabaseException;
}
