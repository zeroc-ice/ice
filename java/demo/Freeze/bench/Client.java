// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class Client extends Ice.Application
{
    void
    IntIntMapTest(Freeze.Map<Integer, Integer> m, boolean fast)
    {
        //
        // Populate the database.
        //
        _watch.start();
        Freeze.Transaction tx = _connection.beginTransaction();
        if(fast)
        {
            for(int i = 0; i < _repetitions; ++i)
            {
                m.fastPut(i, i);
            }
        }
        else
        {
            for(int i = 0; i < _repetitions; ++i)
            {
                m.put(i, i);
            }
        }
        tx.commit();

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
            Integer n = m.get(i);
            test(n.intValue() == i);
        }
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

        if(m instanceof IndexedIntIntMap)
        {
            IndexedIntIntMap indexedM = (IndexedIntIntMap)m;
            //
            // Read each record using the index
            //
            _watch.start();
            for(int i = 0; i < _repetitions; ++i)
            {
                java.util.Iterator<java.util.Map.Entry<Integer, Integer>> p = indexedM.findByValue(i);
                test(p.hasNext());
                java.util.Map.Entry<Integer, Integer> e = p.next();
                test(e.getKey().intValue() == i);
                m.closeAllIterators();
            }
            total = _watch.stop();
            perRecord = total / _repetitions;

            System.out.println("\ttime for " + _repetitions + " indexed reads: " + total + "ms");
            System.out.println("\ttime per indexed read: " + perRecord + "ms");
        }

        //
        // Remove each record.
        //
        _watch.start();
        tx = _connection.beginTransaction();
        if(fast)
        {
            for(int i = 0; i < _repetitions; ++i)
            {
                test(m.fastRemove(i));
            }
        }
        else
        {
            for(int i = 0; i < _repetitions; ++i)
            {
                Integer n = m.remove(i);
                test(n.intValue() == i);
            }
        }
        tx.commit();
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
    }

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
            return new Integer((_max - _min) + 1).toString();
        }

        private int _min;
        private int _max;
        private int _current;
    }

    void
    generatedRead(Freeze.Map<Integer, Integer> m, int reads, Generator gen)
    {
        _watch.start();
        for(int i = 0; i < reads; ++i)
        {
            int key = gen.next();
            Integer n = m.get(key);
            test(n.intValue() == key);
        }
        double total = _watch.stop();
        double perRecord = total / reads;

        System.out.println("\ttime for " + reads + " reads of " + gen + " records: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

        if(m instanceof IndexedIntIntMap)
        {
            IndexedIntIntMap indexedM = (IndexedIntIntMap)m;
            _watch.start();
            for(int i = 0; i < reads; ++i)
            {
                int val = gen.next();
                java.util.Iterator<java.util.Map.Entry<Integer, Integer>> p = indexedM.findByValue(val);
                test(p.hasNext());
                java.util.Map.Entry<Integer, Integer> e = p.next();
                test(e.getKey().intValue() == val);
                m.closeAllIterators();
            }
            total = _watch.stop();
            perRecord = total / reads;

            System.out.println("\ttime for " + reads + " reverse (indexed) reads of " + gen + " records: " + total +
                               "ms");
            System.out.println("\ttime per reverse (indexed) read: " + perRecord + "ms");
        }
    }

    void
    IntIntMapReadTest(Freeze.Map<Integer, Integer> m)
    {
        //
        // Populate the database.
        //
        _watch.start();
        Freeze.Transaction tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            m.fastPut(i, i);
        }
        tx.commit();
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
            test(m.fastRemove(i));
        }
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");
*/

        m.close();
    }

    void
    Struct1Struct2MapTest(Freeze.Map<Struct1, Struct2> m)
    {
        //
        // Populate the database.
        //
        Struct1 s1 = new Struct1();
        Struct2 s2 = new Struct2();
        _watch.start();
        Freeze.Transaction tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            s2.s = new Integer(i).toString();
            s2.s1 = s1;
            m.fastPut(s1, s2);
        }
        tx.commit();
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
            Struct2 ns2 = m.get(s1);
            test(ns2.s.equals(new Integer(i).toString()));
        }
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

        //
        // Optional index test
        //

        if(m instanceof IndexedStruct1Struct2Map)
        {
            IndexedStruct1Struct2Map indexedM = (IndexedStruct1Struct2Map)m;
            _watch.start();
            for(int i = 0; i < _repetitions; ++i)
            {
                String s = (new Integer(i)).toString();
                java.util.Iterator<java.util.Map.Entry<Struct1, Struct2>> p = indexedM.findByS(s);
                test(p.hasNext());
                java.util.Map.Entry<Struct1, Struct2> e = p.next();
                test(e.getKey().l == i);
                m.closeAllIterators();
            }

            for(int i = 0; i < _repetitions; ++i)
            {
                s1.l = i;
                java.util.Iterator<java.util.Map.Entry<Struct1, Struct2>> p = indexedM.findByS1(s1);
                test(p.hasNext());
                java.util.Map.Entry<Struct1, Struct2> e = p.next();
                test(e.getKey().l == i);
                m.closeAllIterators();
            }
            total = _watch.stop();
            perRecord = total / (2 * _repetitions);

            System.out.println("\ttime for " + 2 * _repetitions + " indexed reads: " + total + "ms");
            System.out.println("\ttime per indexed read: " + perRecord + "ms");
        }

        //
        // Remove each record.
        //
        _watch.start();
        tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            test(m.fastRemove(s1));
        }
        tx.commit();
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

        m.close();
    }

    void
    Struct1Class1MapTest(Freeze.Map<Struct1, Class1> m)
    {
        //
        // Populate the database.
        //
        Struct1 s1 = new Struct1();
        Class1 c1 = new Class1();
        _watch.start();
        Freeze.Transaction tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            c1.s = new Integer(i).toString();
            m.fastPut(s1, c1);
        }
        tx.commit();
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
            Class1 nc1 = m.get(s1);
            test(nc1.s.equals(new Integer(i).toString()));
        }
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " reads: " + total + "ms");
        System.out.println("\ttime per read: " + perRecord + "ms");

        //
        // Optional index test
        //

        if(m instanceof IndexedStruct1Class1Map)
        {
            IndexedStruct1Class1Map indexedM = (IndexedStruct1Class1Map)m;
            _watch.start();
            for(int i = 0; i < _repetitions; ++i)
            {
                String s = (new Integer(i)).toString();
                java.util.Iterator<java.util.Map.Entry<Struct1, Class1>> p = indexedM.findByS(s);
                test(p.hasNext());
                java.util.Map.Entry<Struct1, Class1> e = p.next();
                test(e.getKey().l == i);
                m.closeAllIterators();
            }

            total = _watch.stop();
            perRecord = total /  _repetitions;

            System.out.println("\ttime for " +  _repetitions + " indexed reads: " + total + "ms");
            System.out.println("\ttime per indexed read: " + perRecord + "ms");
        }

        //
        // Remove each record.
        //
        _watch.start();
        tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            test(m.fastRemove(s1));
        }
        tx.commit();
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

        m.close();
    }

    void
    Struct1ObjectMapTest(Freeze.Map<Struct1, Ice.Object> m)
    {
        //
        // Populate the database.
        //
        Struct1 s1 = new Struct1();
        Class1 c1 = new Class1();
        Class2 c2 = new Class2();
        c2.rec = c2;
        c2.obj = c1;
        _watch.start();
        Freeze.Transaction tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            Ice.Object o;
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
        tx.commit();

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

            Ice.Object o = m.get(s1);

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
        tx = _connection.beginTransaction();
        for(int i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            test(m.fastRemove(s1));
        }
        tx.commit();
        total = _watch.stop();
        perRecord = total / _repetitions;

        System.out.println("\ttime for " + _repetitions + " removes: " + total + "ms");
        System.out.println("\ttime per remove: " + perRecord + "ms");

        m.close();
    }

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        _connection = Freeze.Util.createConnection(communicator(), _envName);

        System.out.println("IntIntMap (Collections API)");
        IntIntMapTest(new IntIntMap(_connection, "IntIntMap", true), false);

        System.out.println("IntIntMap (Fast API)");
        IntIntMapTest(new IntIntMap(_connection, "IntIntMap.fast", true), true);

        System.out.println("IntIntMap with index (Collections API)");
        IntIntMapTest(new IndexedIntIntMap(_connection, "IndexedIntIntMap", true), false);

        System.out.println("IntIntMap with index (Fast API)");
        IntIntMapTest(new IndexedIntIntMap(_connection, "IndexedIntIntMap.fast", true), true);

        System.out.println("Struct1Struct2Map");
        Struct1Struct2MapTest(new Struct1Struct2Map(_connection, "Struct1Struct2", true));

        System.out.println("Struct1Struct2Map with index");
        Struct1Struct2MapTest(new IndexedStruct1Struct2Map(_connection, "IndexedStruct1Struct2", true));

        System.out.println("Struct1Class1Map");
        Struct1Class1MapTest(new Struct1Class1Map(_connection, "Struct1Class1", true));

        System.out.println("Struct1Class1Map with index");
        Struct1Class1MapTest(new IndexedStruct1Class1Map(_connection, "IndexedStruct1Class1", true));

        System.out.println("Struct1ObjectMap");
        Struct1ObjectMapTest(new Struct1ObjectMap(_connection, "Struct1Object", true));

        System.out.println("IntIntMap (read test)");
        IntIntMapReadTest(new IntIntMap(_connection, "IntIntMap", true));

        System.out.println("IntIntMap with index (read test)");
        IntIntMapReadTest(new IndexedIntIntMap(_connection, "IndexedIntIntMap", true));

        _connection.close();

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

    Client(String envName)
    {
        _envName = envName;
    }

    static public void
    main(String[] args)
    {
        Client app = new Client("db");
        app.main("demo.Freeze.bench.Client", args);
    }

    private Freeze.Connection _connection;
    private int _repetitions = 10000;
    private StopWatch _watch = new StopWatch();
    private String _envName;
}
