// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

import Freeze.*;

public class Client
{
    static String alphabet = "abcdefghijklmnopqrstuvwxyz";

    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    static byte[]
    constructKey(char key)
    {
	byte[] k = new byte[1];
	k[0] = (byte)key;

	return k;
    }

    static void
    addValue(DB db, char key, int value)
	throws DBException
    {
	byte[] k = constructKey(key);

	byte[] v = new byte[4];
	v[0] = (byte)(value & 0xff);
	v[1] = (byte)((value << 8) & 0xff);
	v[2] = (byte)((value << 16)& 0xff);
	v[3] = (byte)((value << 24) & 0xff);
	
	db.put(k, v);
    }

    static void
    readValue(DB db, KeyHolder k, ValueHolder v, Ice.ByteHolder key, Ice.IntHolder value)
    {
	key.value = k.value[0];
	value.value = v.value[0] | (v.value[1] << 8) | (v.value[2] << 16) | (v.value[3] << 24);
    }

    private static void
    populateDB(DB db)
	throws DBException
    {
	for (int j = 0; j < alphabet.length(); ++j)
	{
	    addValue(db, alphabet.charAt(j), j);
	}
    }

    static int
    find(String s, char b)
    {
	for (int i = 0; i < s.length(); ++i)
	{
	    if (s.charAt(i) == b)
	    {
		return i;
	    }
	}
	return -1;
    }

    private static int
    run(String[] args, DBEnvironment dbEnv)
	throws DBException
    {
	DB db = dbEnv.openDB("test", true);

	//
	// Populate the database with the alphabet.
	//
	populateDB(db);

	byte[] akey;

	KeyHolder k = new KeyHolder();
	ValueHolder v = new ValueHolder();

	Ice.ByteHolder key = new Ice.ByteHolder();
	Ice.IntHolder value = new Ice.IntHolder();

	DBCursor cursor, clone;
	int j;

	System.out.print("testing populate... ");
	cursor = db.getCursor();
	j = 0;
	try
	{
	    do
	    {
		cursor.curr(k, v);
		readValue(db, k, v, key, value);
		test(key.value == alphabet.charAt(j) && value.value == j);
		++j;
	    }
	    while (cursor.next());
	}
	catch(DBNotFoundException e)
	{
	    test(false);
	}
	cursor.close();
	System.out.println("ok");

	System.out.print("testing contains... ");
	j = 0;
	try
	{
	    for (j = 0; j < alphabet.length(); ++j)
	    {
		akey = constructKey(alphabet.charAt(j));
		test(db.contains(akey));
	    }
	}
	catch(DBException e)
	{
	    test(false);
	}
	akey = constructKey('0');
	test(!db.contains(akey));
	System.out.println("ok");

	System.out.print("testing DB::getCursorAKey... ");
	akey = constructKey('n');
	j = find(alphabet, 'n');
	cursor = db.getCursorAtKey(akey);
	try
	{
	    do
	    {
		cursor.curr(k, v);
		readValue(db, k, v, key, value);
		test(key.value == alphabet.charAt(j) && value.value == j);
		++j;
	    }
	    while (cursor.next());
	}
	catch(DBNotFoundException e)
	{
	    test(false);
	}
	cursor.close();
	System.out.println("ok");

	System.out.print("testing remove... ");
	cursor = db.getCursor();
	j = 0;

	try
	{
	    do
	    {
		cursor.curr(k, v);
		readValue(db, k, v, key, value);
		test(key.value == alphabet.charAt(j) && value.value == j);
		cursor.del();
		++j;
		if (key.value == 'c')
		{
		    break;
		}
	    }
	    while (cursor.next());
	}
	catch(DBNotFoundException e)
	{
	    test(false);
	}
	cursor.close();

	cursor = db.getCursor();
	j = find(alphabet, 'd');
	try
	{
	    do
	    {
		cursor.curr(k, v);
		readValue(db, k, v, key, value);
		test(key.value == alphabet.charAt(j) && value.value == j);
		++j;
	    }
	    while (cursor.next());
	}
	catch(DBNotFoundException e)
	{
	}
	cursor.close();
	System.out.println("ok");

	//
	// Get a cursor for the deleted element - this should fail.
	//
	System.out.print("testing DB::getCursorAtKey (again)... ");
	try
	{
	    akey = constructKey('a');
	    cursor = db.getCursorAtKey(akey);
	    test(false);
	}
	catch(DBNotFoundException e)
	{
	    // Ignore
	}
	System.out.println("ok");

	System.out.print("testing clone... ");
	cursor = db.getCursor();
	clone = cursor._clone();

	//
	// Verify both cursors point at 'd'.
	//
	cursor.curr(k, v);
	readValue(db, k, v, key, value);
	test(key.value == 'd' && value.value == 3);

	clone.curr(k, v);
	readValue(db, k, v, key, value);
	test(key.value == 'd' && value.value == 3);

	cursor.close();
	clone.close();

	//
	// Create cursor that points at 'n'
	//
	akey = constructKey('n');
	cursor = db.getCursorAtKey(akey);
	clone = cursor._clone();

	//
	// Verify both cursors point at 'n'.
	//
	cursor.curr(k, v);
	readValue(db, k, v, key, value);
	test(key.value == 'n' && value.value == 13);

	clone.curr(k, v);
	readValue(db, k, v, key, value);
	test(key.value == 'n' && value.value == 13);

	cursor.close();
	clone.close();

	//
	// Create cursor that points at 'n'.
	//
	akey = constructKey('n');
	cursor = db.getCursorAtKey(akey);

	cursor.curr(k, v);
	cursor.next();
	readValue(db, k, v, key, value);
	test(key.value == 'n' && value.value == 13);

	clone = cursor._clone();

	//
	// Verify cloned cursors are independent.
	//
	cursor.curr(k, v);
	cursor.next();

	readValue(db, k, v, key, value);
	test(key.value == 'o' && value.value == 14);

	cursor.curr(k, v);
	cursor.next();
	readValue(db, k, v, key, value);
	test(key.value == 'p' && value.value == 15);

	clone.curr(k, v);
	clone.next();
	readValue(db, k, v, key, value);
	test(key.value == 'o' && value.value == 14);

	cursor.close();
	clone.close();
    
	System.out.println("ok");

	db.close();
	
        return 0;
    }

    static public void
    main(String[] args)
    {
	int status;
	Ice.Communicator communicator = null;
	DBEnvironment dbEnv = null;
	String dbEnvDir = "db";

	try
	{
	    Ice.StringSeqHolder holder = new Ice.StringSeqHolder();
	    holder.value = args;
	    communicator = Ice.Util.initialize(holder);
	    args = holder.value;
	    if (args.length > 1)
	    {
		dbEnvDir = args[1];
		dbEnvDir += "/";
		dbEnvDir += "db";
	    }
	    dbEnv = Freeze.Util.initialize(communicator, dbEnvDir);
	    status = run(args, dbEnv);
	}
	catch(Exception ex)
	{
	    ex.printStackTrace();
	    System.err.println(ex);
	    status = 1;
	}

	if (dbEnv != null)
	{
	    try
	    {
		dbEnv.close();
	    }
	    catch(DBException ex)
	    {
		System.err.println(args[0] + ": " + ex + ": " + ex.message);
		status = 1;
	    }
	    catch(Ice.LocalException ex)
	    {
		System.err.println(args[0] + ": " + ex);
		status = 1;
	    }
	    catch(Exception ex)
	    {
		System.err.println(args[0] + ": unknown exception: " + ex);
		status = 1;
	    }
	    dbEnv = null;
	}

	if (communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(Exception ex)
	    {
		System.err.println(ex);
		status = 1;
	    }
	}

	System.exit(status);
    }
}
