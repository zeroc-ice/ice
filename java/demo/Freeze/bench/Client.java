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

class TestApp extends Ice.Application
{
    TestApp(String envName)
    {
	_envName = envName;
    }

    void
    IntIntMapTest(boolean fast)
    {
	IntIntMap m;

	if(fast)
	{
	    m = new IntIntMap(communicator(), _envName, "IntIntMap.fast", true);
	}
	else
	{
	    m = new IntIntMap(communicator(), _envName, "IntIntMap", true);
	}

	//
	// Populate the database.
	//
	_watch.start();
	if(fast)
	{
	    for(int i = 0; i < _repetitions; ++i)
	    {
		m.fastPut(new Integer(i), new Integer(i));
	    }
	}
	else
	{
	    for(int i = 0; i < _repetitions; ++i)
	    {
		m.put(new Integer(i), new Integer(i));
	    }
	}

	double total = _watch.stop();
        double perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " " + ((fast) ? "fast " : "") + "writes: " + total + "ms");
        System.out.println("\ttime per write: " + perRecord + "ms");

	//
	// Read each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    Integer n = (Integer)m.get(new Integer(i));
	    test(n.intValue() == i);
	}
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

	//
	// Remove each record.
	//
	_watch.start();
	if(fast)
	{
	    for(int i = 0; i < _repetitions; ++i)
	    {
		test(m.fastRemove(new Integer(i)));
	    }
	}
	else
	{
	    for(int i = 0; i < _repetitions; ++i)
	    {
		Integer n = (Integer)m.remove(new Integer(i));
		test(n.intValue() == i);
	    }
	}
	total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " " + ((fast) ? "fast " : "") + "removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

	m.close();
    }

    interface Generator
    {
	int next();
	String toString();
    }

    static class RandomGenerator implements Generator
    {
	RandomGenerator(int seed, int max)
	{
	    _r = new java.util.Random(0);
	    _max = max;
	}

	public int
	next()
	{
	    return _r.nextInt(_max);
	}

	public String
	toString()
	{
	    return "random(" + _max + ")";
	}

	private java.util.Random _r;
	private int _max;
    };

    static class SequentialGenerator implements Generator
    {
	SequentialGenerator(int min, int max)
	{
	    _min = min;
	    _max = max;
	    _current = _min;
	}

	public int
	next()
	{
	    int n = _current;
	    ++_current;
	    if(_current > _max)
	    {
		_current = _min;
	    }
	    return n;
	}

	public String
	toString()
	{
	    return new Integer((_max - _min)+1).toString();
	}

	private int _min;
	private int _max;
	private int _current;
    }

    void
    generatedRead(IntIntMap m, int reads, Generator gen)
    {
	_watch.start();
	for(int i = 0; i < reads; ++i)
	{
	    int key = gen.next();
	    Integer n = (Integer)m.get(new Integer(key));
	    test(n.intValue() == key);
	}
        double total = _watch.stop();
        double perRecord = total / reads;

        System.out.println("\ttime for " + reads + " reads of " + gen + " records: " +
			   total + "ms");
	System.out.println("\ttime per read: " + perRecord + "ms");
    }

    void
    IntIntMapReadTest()
    {
	IntIntMap m = new IntIntMap(communicator(), _envName, "IntIntMap", true);

	//
	// Populate the database.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    m.fastPut(new Integer(i), new Integer(i));
	}

	double total = _watch.stop();
        double perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " writes: " + total + "ms");
        System.out.println("\ttime per write: " + perRecord + "ms");

	//
	// Do some read tests.
	//
	generatedRead(m, _repetitions, new SequentialGenerator(1000, 1000));
	generatedRead(m, _repetitions, new SequentialGenerator(2000, 2009));
	generatedRead(m, _repetitions, new SequentialGenerator(3000, 3099));
	generatedRead(m, _repetitions, new SequentialGenerator(4000, 4999));

	//
	// Do a random read test.
	//
	generatedRead(m, _repetitions, new RandomGenerator(0, 10000));

	//
	// Remove each record.
	//
/*
 *      For this test I don't want to remove the records because I
 *      want to examine the cache stats for the database.
 *
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    test(m.fastRemove(new Integer(i)));
	}
	total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");
*/

	m.close();
    }

    void
    Struct1Struct2MapTest()
    {
	Struct1Struct2Map m = new Struct1Struct2Map(communicator(), _envName, "Struct1Struct2", true);

	//
	// Populate the database.
	//
	Struct1 s1 = new Struct1();
	Struct2 s2 = new Struct2();
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    s2.s = new Integer(i).toString();
	    m.fastPut(s1, s2);
	}

	double total = _watch.stop();
        double perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " writes: " + total + "ms");
        System.out.println("\ttime per write: " + perRecord + "ms");

	//
	// Read each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    Struct2 ns2 = (Struct2)m.get(s1);
	    test(ns2.s.equals(new Integer(i).toString()));
	}
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

	//
	// Remove each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    test(m.fastRemove(s1));
	}
	total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

	m.close();
    }

    void
    Struct1Class1MapTest()
    {
	Struct1Class1Map m = new Struct1Class1Map(communicator(), _envName, "Struct1Class1", true);

	//
	// Populate the database.
	//
	Struct1 s1 = new Struct1();
	Class1 c1 = new Class1();
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    c1.s = new Integer(i).toString();
	    m.fastPut(s1, c1);
	}

	double total = _watch.stop();
        double perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " writes: " + total + "ms");
        System.out.println("\ttime per write: " + perRecord + "ms");

	//
	// Read each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    Class1 nc1 = (Class1)m.get(s1);
	    test(nc1.s.equals(new Integer(i).toString()));
	}
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

	//
	// Remove each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    test(m.fastRemove(s1));
	}
	total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

	m.close();
    }

    void
    Struct1ObjectMapTest()
    {
	Struct1ObjectMap m = new Struct1ObjectMap(communicator(), _envName, "Struct1Object", true);

	//
	// Populate the database.
	//
	Struct1 s1 = new Struct1();
	Class1 c1 = new Class1();
	Class2 c2 = new Class2();
	c2.rec = c2;
	c2.obj = c1;
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    Object o;
	    if((i % 2) == 0)
	    {
		o = c2;
	    }
	    else
	    {
		o = c1;
	    }
	    c1.s = new Integer(i).toString();

	    m.fastPut(s1, o);
	}

	double total = _watch.stop();
        double perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " writes: " + total + "ms");
        System.out.println("\ttime per write: " + perRecord + "ms");

	//
	// Read each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;

	    Object o = m.get(s1);

	    Class1 nc1;
	    if((i % 2) == 0)
	    {
		Class2 nc2 = (Class2)o;
		test(nc2.rec == nc2);
		nc1 = (Class1)nc2.obj;
	    }
	    else
	    {
		nc1 = (Class1)o;
	    }

	    test(nc1.s.equals(new Integer(i).toString()));
	}
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

	//
	// Remove each record.
	//
	_watch.start();
	for(int i = 0; i < _repetitions; ++i)
	{
	    s1.l = i;
	    test(m.fastRemove(s1));
	}
	total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

	m.close();
    }
    
    public int
    run(String[] args)
    {
	System.out.println("IntIntMap (Collections API)");
	IntIntMapTest(false);

	System.out.println("IntIntMap (Fast API)");
	IntIntMapTest(true);

	System.out.println("Struct1Struct2Map");
	Struct1Struct2MapTest();

	System.out.println("Struct1Class1Map");
	Struct1Class1MapTest();

	MyFactory factory = new MyFactory();
	factory.install(communicator());

	System.out.println("Struct1ObjectMap");
	Struct1ObjectMapTest();

	System.out.println("IntIntMap (read test)");
	IntIntMapReadTest();

	return 0;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static private class MyFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
	public Ice.Object
	create(String type)
	{
	    if(type.equals("::Class1"))
	    {
		return new Class1();
	    }
	    else if(type.equals("::Class2"))
	    {
		return new Class2();
	    }
	    throw new RuntimeException();
	}
	
	public void
	destroy()
	{
	}

	public void
	install(Ice.Communicator communicator)
	{
	    communicator.addObjectFactory(this, "::Class1");
	    communicator.addObjectFactory(this, "::Class2");
	}
    }

    private int _repetitions = 10000;
    private StopWatch _watch = new StopWatch();
    private String _envName;
}

public class Client
{
    static public void
    main(String[] args)
    {
	TestApp app = new TestApp("db");
	app.main("test.Freeze.bench.Client", args, "config");
    }
}
