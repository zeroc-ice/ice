// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

import Freeze.*;

class CharIntMap extends Freeze.Map
{
    public byte[]
    encodeKey(Object o, Ice.Communicator communicator)
    {
	if(!(o instanceof Character))
	{
	    throw new RuntimeException();
	}
	java.io.StringWriter sw = new java.io.StringWriter();
	java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	pw.print("<data>");
	Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
	ostream.writeByte("Key", (byte) ((Character)o).charValue());
	pw.print("</data>");
	pw.flush();

	return sw.toString().getBytes();
    }

    public Object
    decodeKey(byte[] b, Ice.Communicator communicator)
    {
	java.io.StringReader sr = new java.io.StringReader(new String(b));
	Ice.Stream istream = new IceXML.StreamI(communicator, sr);
	return new Character((char) istream.readByte("Key"));
    }

    public byte[]
    encodeValue(Object o, Ice.Communicator communicator)
    {
	if(!(o instanceof Integer))
	{
	    throw new RuntimeException();
	}

	java.io.StringWriter sw = new java.io.StringWriter();
	java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	pw.print("<data>");
	Ice.Stream ostream = new IceXML.StreamI(communicator, pw);
	ostream.writeInt("Value", ((Integer)o).intValue());
	pw.print("</data>");
	pw.flush();

	return sw.toString().getBytes();
    }

    public Object
    decodeValue(byte[] b, Ice.Communicator communicator)
    {
	java.io.StringReader sr = new java.io.StringReader(new String(b));
	Ice.Stream istream = new IceXML.StreamI(communicator, sr);
	return new Integer(istream.readInt("Value"));
    }

    CharIntMap(Freeze.DB db)
    {
	super(db);
    }
}

public class Client
{
    static String alphabet = "abcdefghijklmnopqrstuvwxyz";

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static void
    populateDB(CharIntMap m)
	throws DBException
    {
	for(int j = 0; j < alphabet.length(); ++j)
	{
	    m.put(new Character(alphabet.charAt(j)), new Integer(j));
	}
    }

    private static int
    run(String[] args, DBEnvironment dbEnv)
	throws DBException
    {
	DB db = dbEnv.openDB("test", true);

	CharIntMap m = new CharIntMap(db);

	//
	// First clear the database.
	//
	m.clear();

	//
	// Re-populate.
	//
	populateDB(m);
	test(m.size() == 26);

	//
	// Populate the database with the alphabet.
	//
	populateDB(m);

	int j;

	System.out.print("testing populate... ");
	for(j = 0; j < alphabet.length(); ++j)
	{
	    Object value = m.get(new Character(alphabet.charAt(j)));
	    test(value != null);
	}
	test(m.get(new Character('0')) == null);
	for(j = 0; j < alphabet.length(); ++j)
	{
	    test(m.containsKey(new Character(alphabet.charAt(j))));
	}
	test(!m.containsKey(new Character('0')));
	for(j = 0; j < alphabet.length(); ++j)
	{
	    test(m.containsValue(new Integer(j)));
	}
	test(!m.containsValue(new Integer(-1)));
	test(m.size() == alphabet.length());
	test(!m.isEmpty());
	System.out.println("ok");

	System.out.print("testing erase... ");
	m.remove(new Character('a'));
	m.remove(new Character('b'));
	m.remove(new Character('c'));
	for(j = 3; j < alphabet.length(); ++j)
	{
	    Object value = m.get(new Character(alphabet.charAt(j)));
	    test(value != null);
	}
	test(m.get(new Character('a')) == null);
	test(m.get(new Character('b')) == null);
	test(m.get(new Character('c')) == null);
	System.out.println("ok");
	
	//
	// Re-populate.
	//
	populateDB(m);

	System.out.print("testing keySet... ");
	java.util.Set keys = m.keySet();
	test(keys.size() == alphabet.length());
	test(!keys.isEmpty());
	java.util.Iterator p = keys.iterator();
	while(p.hasNext())
	{
	    Object o = p.next();
	    test(keys.contains(o));

	    Character c = (Character)o;
	    test(m.containsKey(c));
	}
	System.out.println("ok");

	System.out.print("testing values... ");
	java.util.Collection values = m.values();
	test(values.size() == alphabet.length());
	test(!values.isEmpty());
	p = values.iterator();
	while(p.hasNext())
	{
	    Object o = p.next();
	    test(values.contains(o));

	    Integer i = (Integer)o;
	    test(m.containsValue(i));
	}
	System.out.println("ok");

	System.out.print("testing entrySet... ");
	java.util.Set entrySet = m.entrySet();
	test(entrySet.size() == alphabet.length());
	test(!entrySet.isEmpty());
	p = entrySet.iterator();
	while(p.hasNext())
	{
	    Object o = p.next();
	    test(entrySet.contains(o));

	    java.util.Map.Entry e = (java.util.Map.Entry)o;
	    test(m.containsKey(e.getKey()));
	    test(m.containsValue(e.getValue()));
	}
	System.out.println("ok");

	System.out.print("testing iterator.remove... ");

	test(m.size() == 26);
	test(m.get(new Character('b')) != null);
	test(m.get(new Character('n')) != null);
	test(m.get(new Character('z')) != null);

	p = entrySet.iterator();
	while(p.hasNext())
	{
	    Object o = p.next();
	    java.util.Map.Entry e = (java.util.Map.Entry)o;
	    Character c = (Character)e.getKey();
	    char v = c.charValue();
	    if(v == 'b' || v == 'n' || v == 'z')
	    {
		p.remove();
	    }
	}

	test(m.size() == 23);
	test(m.get(new Character('b')) == null);
	test(m.get(new Character('n')) == null);
	test(m.get(new Character('z')) == null);

	//
	// Re-populate.
	//
	populateDB(m);

	test(m.size() == 26);

	p = entrySet.iterator();
	while(p.hasNext())
	{
	    Object o = p.next();
	    java.util.Map.Entry e = (java.util.Map.Entry)o;
	    char v = ((Character)e.getKey()).charValue();
	    if(v == 'a' || v == 'b' || v == 'c')
	    {
		p.remove();
	    }
	}

	test(m.size() == 23);
	test(m.get(new Character('a')) == null);
	test(m.get(new Character('b')) == null);
	test(m.get(new Character('c')) == null);
	System.out.println("ok");

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
	    if(args.length > 0)
	    {
		dbEnvDir = args[0];
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

	if(dbEnv != null)
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

	if(communicator != null)
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
