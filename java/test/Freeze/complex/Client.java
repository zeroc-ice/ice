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
    final static String progName = "test.Freeze.complex.Client";

    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    private static int
    validate(DB db)
	throws DBException
    {
	Complex.ComplexDict m = new Complex.ComplexDict(db);

	try
	{
	    Parser parser = new Parser();
	
	    System.out.print("testing database expressions... ");
	    java.util.Iterator p = m.entrySet().iterator();
	    while (p.hasNext())
	    {
		java.util.Map.Entry e = (java.util.Map.Entry)p.next();

		Complex.Key key = (Complex.Key)e.getKey();

		//
		// Verify the stored record is correct.
		//
		test(key.result == ((Complex.Node)e.getValue()).calc(null));
	    
		//
		// Verify that the expression & result again.
		//
		Complex.Node root = parser.parse(key.expression);
		test(root.calc(null) == key.result);
	    }
	    System.out.println("ok");
	}
	catch(ParseError e)
	{
	    e.printStackTrace();
	    test(false);
	}
	
	return 0;
    }
    
    private static int
    populate(DB db)
	throws DBException
    {
	String[] expressions = 
	{
	    "2",
	    "10",
	    "2+(5*3)",
	    "5*(2+3)",
	    "10+(10+(20+(8*(2*(3*2+4+5+6)))))"
	};
	
	Complex.ComplexDict m = new Complex.ComplexDict(db);

	try
	{
	    Parser parser = new Parser();
	
	    System.out.print("populating the database... ");
	    for (int i = 0 ; i < expressions.length; ++i)
	    {
		Complex.Node root = parser.parse(expressions[i]);
		assert(root != null);
		Complex.Key k = new Complex.Key();
		k.expression = expressions[i];
		k.result = root.calc(null);
		m.put(k, root);
	    }
	    System.out.println("ok");
	}
	catch(ParseError e)
	{
	    e.printStackTrace();
	    test(false);
	}
	
	return 0;
    }
    
    static void
    usage(String name)
    {
	System.out.println("Usage: " + name + " [options] validate|populate");
	System.out.println("Options:");
	System.out.println("--dbdir           Location of the database directory.");
    }

    private static int
    run(String[] args, DB db)
	throws DBException
    {
	//
	// Register a factory for the node types.
	//
	Ice.Communicator communicator = db.getCommunicator();
	Ice.ObjectFactory factory = new Complex.ObjectFactoryI();
	communicator.addObjectFactory(factory, "::Complex::NumberNode");
	communicator.addObjectFactory(factory, "::Complex::AddNode");
	communicator.addObjectFactory(factory, "::Complex::MultiplyNode");
	
	if (args.length != 0 && args[0].equals("populate"))
	{
	    return populate(db);
	}
	if (args.length != 0 && args[0].equals("validate"))
	{
	    return validate(db);
	}
	usage(progName);

        return 0;
    }

    static public void
    main(String[] args)
    {
	int status;
	Ice.Communicator communicator = null;
	DBEnvironment dbEnv = null;
	DB db = null;
	String dbEnvDir = "db";

	try
	{
	    //
	    // Scan for --dbdir command line argument.
	    //
	    int i = 0;
	    while (i < args.length)
	    {
		if (args[i].equals("--dbdir"))
		{
		    if (i +1 >= args.length)
		    {
			usage(progName);
			System.exit(1);
		    }
		    
		    dbEnvDir = args[i+1];
		    dbEnvDir += "/";
		    dbEnvDir += "db";

		    //
		    // Consume arguments
		    //
		    String[] arr = new String[args.length - 2];
		    System.arraycopy(args, 0, arr, 0, i);
		    if (i < args.length - 2)
		    {
			System.arraycopy(args, i + 2, arr, i, args.length - i - 2);
		    }
		    args = arr;
		}
		else
		{
		    ++i;
		}
	    }

	    Ice.StringSeqHolder holder = new Ice.StringSeqHolder();
	    holder.value = args;
	    communicator = Ice.Util.initialize(holder);
	    args = holder.value;
	    dbEnv = Freeze.Util.initialize(communicator, dbEnvDir);
	    db = dbEnv.openDB("test", true);
	    status = run(args, db);
	}
	catch(Exception ex)
	{
	    ex.printStackTrace();
	    System.err.println(ex);
	    status = 1;
	}

	if (db != null)
	{
	    try
	    {
		db.close();
	    }
	    catch(DBException ex)
	    {
		System.err.println(progName + ": " + ex + ": " + ex.message);
		status = 1;
	    }
	    dbEnv = null;
	}

	if (dbEnv != null)
	{
	    try
	    {
		dbEnv.close();
	    }
	    catch(DBException ex)
	    {
		System.err.println(progName + ": " + ex + ": " + ex.message);
		status = 1;
	    }
	    catch(Ice.LocalException ex)
	    {
		System.err.println(progName + ": " + ex);
		status = 1;
	    }
	    catch(Exception ex)
	    {
		System.err.println(progName + ": unknown exception: " + ex);
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
