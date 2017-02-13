// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.dbmap;

import Freeze.*;

public class Client extends test.Util.Application
{
    static class ReadThread extends Thread
    {
        @Override
        public void
        run()
        {
            try
            {
                for(int i = 0; i < 10; ++i)
                {
                    for(;;)
                    {
                        Transaction tx = _connection.beginTransaction();

                        try
                        {
                            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p = _map.entrySet().iterator();
                            while(p.hasNext())
                            {
                                java.util.Map.Entry<Byte, Integer> e = p.next();
                                byte v = e.getKey().byteValue();
                                test(e.getValue().equals(new Integer(v - (byte)'a')));
                            }

                            break;
                        }
                        catch(DeadlockException ex)
                        {
                            // System.err.print("r");
                            //
                            // Try again
                            //
                        }
                        finally
                        {
                            tx.rollback();
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                System.err.println(ex);
            }
            finally
            {
                ((Freeze.Map<Byte, Integer>)_map).close();
                _connection.close();
            }
        }

        ReadThread(Ice.Communicator communicator, String envName, String dbName)
        {
            _connection = Freeze.Util.createConnection(communicator, envName);
            _map = new ByteIntMap(_connection, dbName, true);
        }

        private Freeze.Connection _connection;
        private java.util.Map<Byte, Integer> _map;
    }

    static class WriteThread extends Thread
    {
        @Override
        public void
        run()
        {
            try
            {
                for(int i = 0; i < 4; ++i)
                {
                    for(;;)
                    {
                        Transaction tx = _connection.beginTransaction();

                        try
                        {
                            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p = _map.entrySet().iterator();
                            while(p.hasNext())
                            {
                                java.util.Map.Entry<Byte, Integer> e = p.next();
                                int v = e.getValue().intValue() + 1;
                                e.setValue(v);
                                p.remove();
                            }

                            tx.commit();
                            tx = null;

                            break;
                        }
                        catch(DeadlockException ex)
                        {
                            // System.err.print("w");
                            //
                            // Try again
                            //
                        }
                        finally
                        {
                            if(tx != null)
                            {
                                tx.rollback();
                            }
                        }
                    }
                    populateDB(_connection, _map);
                }
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
                System.err.println(ex);
            }
            finally
            {
                ((Freeze.Map<Byte, Integer>)_map).close();
                _connection.close();
            }
        }

        WriteThread(Ice.Communicator communicator, String envName, String dbName)
        {
            _connection = Freeze.Util.createConnection(communicator, envName);
            _map = new ByteIntMap(_connection, dbName, true);
        }

        private Freeze.Connection _connection;
        private java.util.Map<Byte, Integer> _map;
    }

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
    populateDB(Freeze.Connection connection, java.util.Map<Byte, Integer> m)
        throws DatabaseException
    {
        int length = alphabet.length();

        for(;;)
        {

            try
            {
                Transaction tx = connection.beginTransaction();
                for(int j = 0; j < length; ++j)
                {
                    m.put((byte)alphabet.charAt(j), j);
                }
                tx.commit();
                break; // for(;;)
            }
            catch(Freeze.DeadlockException dx)
            {
                length = length / 2;
                // System.err.print("t");
                //
                // Try again
                //
            }
            finally
            {
                if(connection.currentTransaction() != null)
                {
                    connection.currentTransaction().rollback();
                }
            }
        }
    }

    private static int
    run(String[] args, Ice.Communicator communicator, String envName, String dbName)
        throws DatabaseException
    {
        Freeze.Connection connection = Freeze.Util.createConnection(communicator, envName);

        //
        // Open/close db within transaction
        //
        {
            Transaction tx = connection.beginTransaction();
            ByteIntMap m = new ByteIntMap(connection, dbName, true);

            m.put((byte)'a', 1);
            m.close();
            tx.rollback();
        }

        java.util.Map<Byte, Integer> m = new ByteIntMap(connection, dbName, true);

        //
        // Populate the database with the alphabet.
        //
        populateDB(connection, m);

        int j;

        System.out.print("testing populate... ");
        System.out.flush();
        for(j = 0; j < alphabet.length(); ++j)
        {
            Integer value = m.get((byte)alphabet.charAt(j));
            test(value != null);
        }
        test(m.get((byte)'0') == null);
        for(j = 0; j < alphabet.length(); ++j)
        {
            test(m.containsKey((byte)alphabet.charAt(j)));
        }
        test(!m.containsKey((byte)'0'));
        for(j = 0; j < alphabet.length(); ++j)
        {
            test(m.containsValue(j));
        }
        test(!m.containsValue(-1));
        test(m.size() == alphabet.length());
        test(!m.isEmpty());
        System.out.println("ok");

        System.out.print("testing erase... ");
        System.out.flush();
        m.remove((byte)'a');
        m.remove((byte)'b');
        m.remove((byte)'c');
        for(j = 3; j < alphabet.length(); ++j)
        {
            Integer value = m.get((byte)alphabet.charAt(j));
            test(value != null);
        }
        test(m.get((byte)'a') == null);
        test(m.get((byte)'b') == null);
        test(m.get((byte)'c') == null);
        test(((ByteIntMap)m).fastRemove((byte)'d') == true);
        test(((ByteIntMap)m).fastRemove((byte)'d') == false);
        System.out.println("ok");

        //
        // Re-populate.
        //
        populateDB(connection, m);

        {
            System.out.print("testing keySet... ");
            System.out.flush();
            java.util.Set<Byte> keys = m.keySet();
            test(keys.size() == alphabet.length());
            test(!keys.isEmpty());
            java.util.Iterator<Byte> p = keys.iterator();
            while(p.hasNext())
            {
                Byte b = p.next();
                test(keys.contains(b));
                test(m.containsKey(b));
            }

            //
            // The iterator should have already been closed when we reached the last entry.
            //
            int count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 0);

            try
            {
                keys.remove((byte)'a');
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - no transaction.
            }

            count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 1); // Opened by keys.remove()

            Transaction tx = connection.beginTransaction();
            
            test(keys.size() == alphabet.length());
            test(keys.remove((byte)'a') == true);

            // Verify that size is transactional
            test(keys.size() == alphabet.length() - 1);
            test(keys.remove((byte)'a') == false);
            tx.commit();
            test(m.containsKey((byte)'a') == false);

            System.out.println("ok");
        }

        //
        // Re-populate.
        //
        populateDB(connection, m);

        {
            System.out.print("testing values... ");
            System.out.flush();
            java.util.Collection<Integer> values = m.values();
            test(values.size() == alphabet.length());
            test(!values.isEmpty());
            java.util.Iterator<Integer> p = values.iterator();
            while(p.hasNext())
            {
                Integer i = p.next();
                test(values.contains(i));
                test(m.containsValue(i));
            }

            //
            // The iterator should have already been closed when we reached the last entry.
            //
            int count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 0);

            try
            {
                values.remove(0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - no transaction.
            }

            count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 1); // Opened by keys.remove()

            Transaction tx = connection.beginTransaction();
            test(values.remove(0) == true);
            test(values.remove(0) == false);
            tx.commit();
            test(m.containsKey((byte)'a') == false);

            System.out.println("ok");
        }

        //
        // Re-populate.
        //
        populateDB(connection, m);

        {
            System.out.print("testing entrySet... ");
            System.out.flush();
            java.util.Set<java.util.Map.Entry<Byte, Integer>> entrySet = m.entrySet();
            test(entrySet.size() == alphabet.length());
            test(!entrySet.isEmpty());
            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p = entrySet.iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry<Byte, Integer> e = p.next();
                test(entrySet.contains(e));
                test(m.containsKey(e.getKey()));
                test(m.containsValue(e.getValue()));
            }

            //
            // The iterator should have already been closed when we reached the last entry.
            //
            int count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 0);
            System.out.println("ok");
        }

        {
            System.out.print("testing unsorted map... ");
            System.out.flush();

            NavigableMap<Byte, Integer> nm = (NavigableMap<Byte, Integer>)m;
            final byte firstByte = (byte)alphabet.charAt(0);
            final byte lastByte = (byte)alphabet.charAt(alphabet.length() - 1);
            final int length = alphabet.length();

            //
            // Keys
            //

            Byte key;

            key = nm.firstKey();
            test(key != null);
            test(key.byteValue() == firstByte);

            key = nm.lastKey();
            test(key != null);
            test(key.byteValue() == lastByte);

            try
            {
                nm.ceilingKey((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.floorKey((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.higherKey((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.lowerKey((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            m.clear();
            test(m.size() == 0);
            try
            {
                nm.firstKey();
                test(false);
            }
            catch(java.util.NoSuchElementException ex)
            {
                // Expected.
            }
            try
            {
                nm.lastKey();
                test(false);
            }
            catch(java.util.NoSuchElementException ex)
            {
                // Expected.
            }

            populateDB(connection, m);

            //
            // Entries
            //

            java.util.Map.Entry<Byte, Integer> e;

            e = nm.firstEntry();
            test(e != null);
            test(e.getKey().byteValue() == firstByte);
            test(e.getValue().intValue() == 0);

            e = nm.lastEntry();
            test(e != null);
            test(e.getKey().byteValue() == lastByte);
            test(e.getValue().intValue() == length - 1);

            try
            {
                nm.ceilingEntry((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.floorEntry((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.higherEntry((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                nm.lowerEntry((byte)0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            e = nm.pollFirstEntry();
            test(e != null);
            test(e.getKey().byteValue() == firstByte);
            test(e.getValue().intValue() == 0);
            test(!nm.containsKey(firstByte));

            e = nm.pollLastEntry();
            test(e != null);
            test(e.getKey().byteValue() == lastByte);
            test(e.getValue().intValue() == length - 1);
            test(!nm.containsKey(lastByte));

            ByteIntMap typedM = (ByteIntMap)m;

            try
            {
                typedM.headMapForValue(0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                typedM.tailMapForValue(0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                typedM.subMapForValue(0, 0);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            try
            {
                typedM.mapForValue();
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected - a comparator is required.
            }

            m.clear();
            test(nm.firstEntry() == null);
            test(nm.lastEntry() == null);
            populateDB(connection, m);

            System.out.println("ok");
        }

        {
            System.out.print("testing for loop... ");
            System.out.flush();
            for(java.util.Map.Entry<Byte, Integer> e : m.entrySet())
            {
                test(m.containsKey(e.getKey()));
                test(m.containsValue(e.getValue()));
            }

            //
            // The iterator should have already been closed when we reached the last entry.
            //
            int count = ((Freeze.Map<Byte, Integer>)m).closeAllIterators();
            test(count == 0);

            System.out.println("ok");
        }

        {
            System.out.print("testing iterator.remove... ");
            System.out.flush();

            Freeze.Map<Byte, Integer> fm = (Freeze.Map<Byte, Integer>)m;
            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p;
            Transaction tx;
            int count;

            test(m.size() == 26);
            test(m.get((byte)'b') != null);
            test(m.get((byte)'n') != null);
            test(m.get((byte)'z') != null);

            //
            // Verify that remove fails without a transaction.
            //
            p = m.entrySet().iterator();
            test(p.hasNext());
            p.next();
            try
            {
                p.remove();
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected.
            }

            count = fm.closeAllIterators();
            test(count == 1);

            tx = connection.beginTransaction();
            p = m.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry<Byte, Integer> e = p.next();
                Byte b = e.getKey();
                byte v = b.byteValue();
                if(v == (byte)'b' || v == (byte)'n' || v == (byte)'z')
                {
                    p.remove();
                    try
                    {
                        p.remove();
                    }
                    catch(IllegalStateException ex)
                    {
                        // Expected.
                    }
                }
            }
            tx.commit();
            count = fm.closeAllIterators(); // Committing the transaction should close the iterator.
            test(count == 0);

            test(m.size() == 23);
            test(m.get((byte)'b') == null);
            test(m.get((byte)'n') == null);
            test(m.get((byte)'z') == null);

            //
            // Re-populate.
            //
            populateDB(connection, m);

            test(m.size() == 26);

            tx = connection.beginTransaction();
            p = m.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry<Byte, Integer> e = p.next();
                byte v = e.getKey().byteValue();
                if(v == (byte)'a' || v == (byte)'b' || v == (byte)'c')
                {
                    p.remove();
                }
            }
            tx.commit();
            count = fm.closeAllIterators(); // Committing the transaction should close the iterator.
            test(count == 0);

            test(m.size() == 23);
            test(m.get((byte)'a') == null);
            test(m.get((byte)'b') == null);
            test(m.get((byte)'c') == null);
            System.out.println("ok");
        }

        {
            System.out.print("testing entry.setValue... ");
            System.out.flush();

            //
            // Re-populate.
            //
            populateDB(connection, m);

            Freeze.Map<Byte, Integer> fm = (Freeze.Map<Byte, Integer>)m;
            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p;
            Transaction tx;
            java.util.Map.Entry<Byte, Integer> e;

            //
            // Verify that setValue on an iterator fails without a transaction.
            //
            p = m.entrySet().iterator();
            test(p.hasNext());
            try
            {
                e = p.next();
                e.setValue(0);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected.
            }

            tx = connection.beginTransaction();
            p = m.entrySet().iterator();
            while(p.hasNext())
            {
                e = p.next();
                byte v = e.getKey().byteValue();
                if(v == (byte)'b' || v == (byte)'n' || v == (byte)'z')
                {
                    e.setValue(v + 100);
                }
            }
            tx.commit();
            test(m.size() == 26);
            test(m.get((byte)'b') != null);
            test(m.get((byte)'n') != null);
            test(m.get((byte)'z') != null);

            p = m.entrySet().iterator();
            while(p.hasNext())
            {
                e = p.next();
                byte v = e.getKey().byteValue();
                if(v == (byte)'b' || v == (byte)'n' || v == (byte)'z')
                {
                    test(e.getValue().equals(new Integer(v + 100)));
                }
                else
                {
                    test(e.getValue().equals(new Integer(v - (byte)'a')));
                }
            }

            //
            // No transaction is necessary for entries obtained without an iterator.
            //
            e = fm.firstEntry();
            test(e != null);
            e.setValue(-1);
            test(e.getValue().intValue() == -1);
            e = fm.firstEntry();
            test(e != null);
            test(e.getValue().intValue() == -1);

            System.out.println("ok");
        }

        {
            System.out.print("testing clear... ");
            System.out.flush();
            test(m.size() > 0);
            m.clear();
            test(m.size() == 0);
            System.out.println("ok");
        }

        {
            System.out.print("testing index... ");
            System.out.flush();

            //
            // Re-populate.
            //
            populateDB(connection, m);

            ByteIntMap typedM = (ByteIntMap)m;

            java.util.Map.Entry<Byte, Integer> e;
            java.util.Iterator<java.util.Map.Entry<Byte, Integer>> p;
            Transaction tx;

            int length = alphabet.length();

            for(int k = 0; k < length; ++k)
            {
                p = typedM.findByValue(k);
                test(p.hasNext());
                e = p.next();
                test(e.getKey().byteValue() == (byte)alphabet.charAt(k));
                test(!p.hasNext());
            }

            //
            // Change the value associated with key 21 to 17.
            //
            m.put((byte)alphabet.charAt(21), 17);

            //
            // Verify that the index no longer has an entry for value 21.
            //
            p = typedM.findByValue(21);
            test(!p.hasNext());

            //
            // Verify that the iterator returns two entries for value 17.
            //
            p = typedM.findByValue(17);
            test(p.hasNext());
            e = p.next();
            byte v = e.getKey().byteValue();
            test(v == (byte)alphabet.charAt(17) || v == (byte)alphabet.charAt(21));

            test(p.hasNext());
            e = p.next();
            v = e.getKey().byteValue();
            test(v == (byte)alphabet.charAt(17) || v == (byte)alphabet.charAt(21));

            test(!p.hasNext()); // Iterator defaults to returning only exact matches.
            test(typedM.valueCount(17) == 2);

            //
            // Cannot remove without a transaction.
            //
            p = typedM.findByValue(17);
            test(p.hasNext());
            p.next();
            try
            {
                p.remove();
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected.
            }

            tx = connection.beginTransaction();
            p = typedM.findByValue(17);
            test(p.hasNext());
            p.next();
            p.remove();
            test(p.hasNext());
            e = p.next();
            v = e.getKey().byteValue();
            test(v == (byte)alphabet.charAt(17) || v == (byte)alphabet.charAt(21));
            test(!p.hasNext());
            tx.commit();

            int count = typedM.closeAllIterators();
            test(count == 0); // Committing the transaction also closes the iterators.

            test(typedM.valueCount(17) == 1);

            p = typedM.findByValue(17);
            test(p.hasNext());
            e = p.next();

            //
            // Cannot set a value on an index iterator.
            //
            try
            {
                e.setValue(18);
                test(false);
            }
            catch(UnsupportedOperationException ex)
            {
                // Expected.
            }

            v = e.getKey().byteValue();
            test(v == (byte)alphabet.charAt(17) || v == (byte)alphabet.charAt(21));
            test(typedM.valueCount(17) == 1);

            m.put((byte)alphabet.charAt(21), 17);

            //
            // Non-exact match
            //
            p = typedM.findByValue(21);
            test(!p.hasNext());

            test(typedM.valueCount(21) == 0);

            p = typedM.findByValue(21, false);
            test(!p.hasNext());

            p = typedM.findByValue(22, false);
            int previous = 21;
            count = 0;
            while(p.hasNext())
            {
                e = p.next();

                int val = e.getValue().intValue();

                test(val > previous);
                previous = val;
                count++;
            }
            test(count == 4);

            System.out.println("ok");
        }

        ((Freeze.Map<Byte, Integer>)m).closeAllIterators();

        {
            System.out.print("testing concurrent access... ");
            System.out.flush();

            m.clear();
            populateDB(connection, m);

            java.util.List<Thread> l = new java.util.ArrayList<Thread>();

            //
            // Create each thread.
            //
            for(int i = 0; i < 5; ++i)
            {
                l.add(new ReadThread(communicator, envName, dbName));
                l.add(new WriteThread(communicator, envName, dbName));
            }

            //
            // Start each thread.
            //
            for(Thread t : l)
            {
                t.start();
            }

            //
            // Wait for each thread to terminate.
            //
            for(Thread t : l)
            {
                while(t.isAlive())
                {
                    try
                    {
                        t.join();
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
            }

            System.out.println("ok");
        }

        System.out.print("testing index creation... ");
        System.out.flush();

        {
            IntIdentityMap iim = new IntIdentityMap(connection, "intIdentity", true);

            Ice.Identity odd = new Ice.Identity();
            odd.name = "foo";
            odd.category = "odd";

            Ice.Identity even = new Ice.Identity();
            even.name = "bar";
            even.category = "even";

            Transaction tx = connection.beginTransaction();
            for(int i = 0; i < 1000; i++)
            {
                if(i % 2 == 0)
                {
                    iim.fastPut(i, even);
                }
                else
                {
                    iim.fastPut(i, odd);
                }
            }
            tx.commit();
            iim.closeDb();
        }

        {
            //
            // Need true to create the index
            //
            IntIdentityMapWithIndex iim = new IntIdentityMapWithIndex(connection, "intIdentity", true);

            test(iim.categoryCount("even") == 500);
            test(iim.categoryCount("odd") == 500);

            int count = 0;
            java.util.Iterator<java.util.Map.Entry<Integer, Ice.Identity>> p = iim.findByCategory("even");
            while(p.hasNext())
            {
                java.util.Map.Entry<Integer, Ice.Identity> e = p.next();
                int k = e.getKey().intValue();
                test(k % 2 == 0);
                ++count;
            }
            test(count == 500);

            count = 0;
            p = iim.findByCategory("odd");
            while(p.hasNext())
            {
                java.util.Map.Entry<Integer, Ice.Identity> e = p.next();
                int k = e.getKey().intValue();
                test(k % 2 == 1);
                ++count;
            }
            test(count == 500);

            iim.destroy();
        }
        System.out.println("ok");

        //
        // Sorting
        //

        final java.util.Comparator<Integer> less = new java.util.Comparator<Integer>()
        {
            @Override
            public int compare(Integer i1, Integer i2)
            {
                if(i1 == i2)
                {
                    return 0;
                }
                else if(i1 == null)
                {
                    return -i2.compareTo(i1);
                }
                else
                {
                    return i1.compareTo(i2);
                }
            }
        };

        java.util.Comparator<String> greater = new java.util.Comparator<String>()
        {
            @Override
            public int compare(String s1, String s2)
            {
                if(s1 == s2)
                {
                    return 0;
                }
                else if(s1 == null)
                {
                    return s2.compareTo(s1);
                }
                else
                {
                    return -s1.compareTo(s2);
                }
            }
        };

        SortedMap.IndexComparators indexComparators = new SortedMap.IndexComparators();
        indexComparators.categoryComparator = greater;
        java.util.Random rand = new java.util.Random();

        {
            SortedMap sm = new SortedMap(connection, "sortedMap", true, less, indexComparators);

            Transaction tx = connection.beginTransaction();
            for(int i = 0; i < 500; i++)
            {
                int k = rand.nextInt(1000);
                Ice.Identity id = new Ice.Identity("foo", String.valueOf(alphabet.charAt(k % 26)));
                sm.fastPut(k, id);
            }
            tx.commit();
            sm.close();
        }

        {
            SortedMap sm = new SortedMap(connection, "sortedMap", true, less, indexComparators);
            NavigableMap<Integer, Ice.Identity> sub = null;

            System.out.print("testing sorting with primary key... ");
            System.out.flush();

            testSortedMap(sm, true);

            {
                final Integer first = sm.firstKey();

                //
                // fastRemove
                //
                sub = sm.headMap(first, true);
              
                Ice.Identity id = sub.get(first);
                test(sub.fastRemove(first) == true);
                test(sub.fastRemove(first) == false);
                test(sm.containsKey(first) == false);
                sm.put(first, id);
                test(sm.containsKey(first) == true);
            }

            System.out.println("ok");

            //
            // Category index
            //

            {
                System.out.print("testing sorting with secondary key... ");
                System.out.flush();

                NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isub = null;

                isub = sm.mapForCategory();
            
                {
                    Transaction tx = connection.beginTransaction();
                    
                    Ice.Identity id = sm.get(sm.firstKey());
                    int sz = isub.get(id.category).size();
                    test(sz > 0);
                    

                    test(sm.fastRemove(sm.firstKey()) == true); 
                    
                    
                    if(sz == 1)
                    {
                        test(isub.get(id.category) == null);
                    }
                    else
                    { 
                        // System.out.println("Check size within tx");
                        test(isub.get(id.category).size() == sz -1);
                    }
                    tx.rollback();
                    test(isub.get(id.category).size() == sz);
                }

                final String first = isub.firstKey();
                final String last = isub.lastKey();

                //
                // Head map
                //
                isub = sm.headMapForCategory(last);
                test(greater.compare(isub.lastKey(), last) < 0);
                isub = sm.headMapForCategory(last, true);
                test(greater.compare(isub.lastKey(), last) == 0);
                isub = sm.headMapForCategory(first);
                test(isub.firstEntry() == null); // map is empty
                isub = sm.headMapForCategory(first, true);
                test(greater.compare(isub.firstKey(), first) == 0);
                test(greater.compare(isub.lastKey(), first) == 0);

                sm.headMapForCategory(first, true).headMap(first, true);
                sm.headMapForCategory(first, true).headMap(first);
                sm.headMapForCategory(first).headMap(first);
                try
                {
                    sm.headMapForCategory(first).headMap(first, true);
                    test(false);
                }
                catch(IllegalArgumentException ex)
                {
                    // Expected.
                }

                {
                    String category = null;
                    while(true)
                    {
                        int k = rand.nextInt(1000);
                        category = String.valueOf(alphabet.charAt(k % 26));
                        isub = sm.headMapForCategory(category);
                        if(isub.firstEntry() != null) // Make sure submap isn't empty.
                        {
                            break;
                        }
                    }

                    testSecondaryKey(isub, null, false, category, false);

                    try
                    {
                        isub.tailMap(category);
                        test(false);
                    }
                    catch(IllegalArgumentException ex)
                    {
                        // Expected.
                    }
                }

                {
                    String category = null;
                    while(true)
                    {
                        int k = rand.nextInt(1000);
                        category = String.valueOf(alphabet.charAt(k % 26));
                        isub = sm.headMapForCategory(category, true);
                        if(isub.firstEntry() != null) // Make sure submap isn't empty.
                        {
                            break;
                        }
                    }

                    testSecondaryKey(isub, null, false, category, true);

                    try
                    {
                        String invalid = String.valueOf(category.charAt(0) + 1);
                        isub.tailMap(invalid);
                        test(false);
                    }
                    catch(IllegalArgumentException ex)
                    {
                        // Expected.
                    }
                }

                //
                // Tail map
                //
                isub = sm.tailMapForCategory(first);
                test(greater.compare(isub.firstKey(), first) == 0);
                isub = sm.tailMapForCategory(first, false);
                test(greater.compare(isub.firstKey(), first) > 0);
                isub = sm.tailMapForCategory(last);
                test(greater.compare(isub.firstKey(), last) == 0);
                test(greater.compare(isub.lastKey(), last) == 0);
                isub = sm.tailMapForCategory(last, false);
                test(isub.firstEntry() == null); // map is empty

                sm.tailMapForCategory(last).tailMap(last);
                sm.tailMapForCategory(last).tailMap(last, false);
                try
                {
                    sm.tailMapForCategory(last, false).tailMap(last);
                    test(false);
                }
                catch(IllegalArgumentException ex)
                {
                    // Expected.
                }

                {
                    String category = null;
                    while(true)
                    {
                        int k = rand.nextInt(1000);
                        category = String.valueOf(alphabet.charAt(k % 26));
                        isub = sm.tailMapForCategory(category);
                        if(isub.firstEntry() != null) // Make sure submap isn't empty.
                        {
                            break;
                        }
                    }

                    testSecondaryKey(isub, category, true, null, false);

                    try
                    {
                        String invalid = String.valueOf((char)(category.charAt(0) + 1));
                        isub.headMap(invalid);
                        test(false);
                    }
                    catch(IllegalArgumentException ex)
                    {
                        // Expected.
                    }
                }

                {
                    String category = null;
                    while(true)
                    {
                        int k = rand.nextInt(1000);
                        category = String.valueOf(alphabet.charAt(k % 26));
                        isub = sm.tailMapForCategory(category, false);
                        if(isub.firstEntry() != null) // Make sure submap isn't empty.
                        {
                            break;
                        }
                    }

                    testSecondaryKey(isub, category, false, null, false);

                    try
                    {
                        isub.headMap(category);
                        test(false);
                    }
                    catch(IllegalArgumentException ex)
                    {
                        // Expected.
                    }
                }

                //
                // Sub map
                //
                isub = sm.subMapForCategory(first, last);
                test(greater.compare(isub.firstKey(), first) == 0);
                test(greater.compare(isub.lastKey(), last) < 0);
                isub = sm.subMapForCategory(first, false, last, false);
                test(greater.compare(isub.firstKey(), first) > 0);
                isub = sm.subMapForCategory(first, true, last, true);
                test(greater.compare(isub.firstKey(), first) == 0);
                test(greater.compare(isub.lastKey(), last) == 0);

                try
                {
                    sm.subMapForCategory(first, false, first, false);
                    test(false);
                }
                catch(IllegalArgumentException ex)
                {
                    // Expected.
                }

                NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isubsub = null;

                isubsub = isub.subMap(first, true, last, true);
                test(greater.compare(isubsub.firstKey(), first) == 0);
                test(greater.compare(isubsub.lastKey(), last) == 0);

                isubsub = isub.subMap(first, false, last, false);
                test(greater.compare(isubsub.firstKey(), first) > 0);
                test(greater.compare(isubsub.lastKey(), last) < 0);

                try
                {
                    isubsub.subMap(first, true, last, false);
                    test(false);
                }
                catch(IllegalArgumentException ex)
                {
                    // Expected.
                }

                try
                {
                    isubsub.subMap(first, false, last, true);
                    test(false);
                }
                catch(IllegalArgumentException ex)
                {
                    // Expected.
                }

                {
                    final boolean fromInclusive[] = { false, false, true, true };
                    final boolean toInclusive[] = { false, true, false, true };
                    for(int i = 0; i < 4; ++i)
                    {
                        String from = null, to = null;
                        while(true)
                        {
                            int f = rand.nextInt(1000) % 26;
                            int t = rand.nextInt(1000) % 26;
                            int f1 = Math.max(f, t);
                            int t1 = Math.min(f, t);
                            if(f1 - t1 < 10)
                            {
                                continue;
                            }
                            from = String.valueOf(alphabet.charAt(f1 % 26));
                            to = String.valueOf(alphabet.charAt(t1 % 26));
                            isub = sm.subMapForCategory(from, fromInclusive[i], to, toInclusive[i]);
                            if(isub.firstEntry() != null) // Make sure submap isn't empty.
                            {
                                break;
                            }
                        }

                        testSecondaryKey(isub, from, fromInclusive[i], to, toInclusive[i]);
                    }
                }

                //
                // fastRemove
                //
                isub = sm.headMapForCategory(first, true);
                try
                {
                    isub.fastRemove(first);
                    test(false);
                }
                catch(UnsupportedOperationException ex)
                {
                    // Expected.
                }

                System.out.println("ok");
            }

            sm.close();
        }

        {
            SortedMap sm = new SortedMap(connection, "sortedMap", true, less, indexComparators);

            System.out.print("testing descending map... ");
            System.out.flush();

            {
                NavigableMap<Integer, Ice.Identity> dmap = sm.descendingMap();
               
                testSortedMap(dmap, false);
                testSortedMap(dmap.descendingMap(), true); // Ascending submap.
              
            }

            int finc, tinc; // Inclusive flags

            for(tinc = 0; tinc < 2; ++tinc)
            {
                while(true)
                {
                    NavigableMap<Integer, Ice.Identity> sub = sm.headMap(rand.nextInt(1000), tinc == 0);
                    if(sub.firstEntry() == null)
                    {
                        continue;
                    }
                    NavigableMap<Integer, Ice.Identity> dmap = sub.descendingMap();
                    test(dmap.firstKey().equals(sub.lastKey()));
                    test(dmap.lastKey().equals(sub.firstKey()));
                    break;
                }
            }

            for(finc = 0; finc < 2; ++finc)
            {
                while(true)
                {
                    NavigableMap<Integer, Ice.Identity> sub = sm.tailMap(rand.nextInt(1000), finc == 0);
                    if(sub.firstEntry() == null)
                    {
                        continue;
                    }
                    NavigableMap<Integer, Ice.Identity> dmap = sub.descendingMap();
                    test(dmap.firstKey().equals(sub.lastKey()));
                    test(dmap.lastKey().equals(sub.firstKey()));
                    break;
                }
            }

            for(finc = 0; finc < 2; ++finc)
            {
                for(tinc = 0; tinc < 2; ++tinc)
                {
                    while(true)
                    {
                        int f = rand.nextInt(1000);
                        int t = rand.nextInt(1000);
                        int from = Math.min(f, t);
                        int to = Math.max(f, t);
                        if(to - from < 100)
                        {
                            continue;
                        }
                        NavigableMap<Integer, Ice.Identity> sub = sm.subMap(from, finc == 0, to, tinc == 0);
                        if(sub.firstEntry() == null)
                        {
                            continue;
                        }
                        NavigableMap<Integer, Ice.Identity> dmap = sub.descendingMap();
                        test(dmap.firstKey().equals(sub.lastKey()));
                        test(dmap.lastKey().equals(sub.firstKey()));
                        break;
                    }
                }
            }

            {
                NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isub, dmap;
                java.util.Comparator<? super String> c;

                isub = sm.mapForCategory(); // An iterator for this map visits keys in descending order.
                dmap = isub.descendingMap(); // An iterator for this map visits keys in ascending order.
                test(dmap.firstKey().equals(isub.lastKey()));
                test(dmap.lastKey().equals(isub.firstKey()));
                c = dmap.comparator();
                String prev = null;
                for(java.util.Map.Entry<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> e :
                    dmap.entrySet())
                {
                    if(prev != null)
                    {
                        test(c.compare(e.getKey(), prev) > 0);
                    }
                    prev = e.getKey();
                }

                dmap = dmap.descendingMap();
                test(dmap.firstKey().equals(isub.firstKey()));
                test(dmap.lastKey().equals(isub.lastKey()));
            }

            for(tinc = 0; tinc < 2; ++tinc)
            {
                while(true)
                {
                    String category = String.valueOf(alphabet.charAt(rand.nextInt(1000) % 26));
                    NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isub =
                        sm.headMapForCategory(category, tinc == 0);
                    if(isub.firstEntry() == null)
                    {
                        continue;
                    }
                    NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> dmap =
                        isub.descendingMap();
                    test(dmap.firstKey().equals(isub.lastKey()));
                    test(dmap.lastKey().equals(isub.firstKey()));
                    break;
                }
            }

            for(finc = 0; finc < 2; ++finc)
            {
                while(true)
                {
                    String category = String.valueOf(alphabet.charAt(rand.nextInt(1000) % 26));
                    NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isub =
                        sm.tailMapForCategory(category, finc == 0);
                    if(isub.firstEntry() == null)
                    {
                        continue;
                    }
                    NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> dmap =
                        isub.descendingMap();
                    test(dmap.firstKey().equals(isub.lastKey()));
                    test(dmap.lastKey().equals(isub.firstKey()));
                    break;
                }
            }

            for(finc = 0; finc < 2; ++finc)
            {
                for(tinc = 0; tinc < 2; ++tinc)
                {
                    while(true)
                    {
                        int f = rand.nextInt(1000) % 26;
                        int t = rand.nextInt(1000) % 26;
                        int f1 = Math.max(f, t);
                        int t1 = Math.min(f, t);
                        if(f1 - t1 < 10)
                        {
                            continue;
                        }
                        String from = String.valueOf(alphabet.charAt(f1 % 26));
                        String to = String.valueOf(alphabet.charAt(t1 % 26));
                        NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> isub =
                            sm.subMapForCategory(from, finc == 0, to, tinc == 0);
                        if(isub.firstEntry() == null)
                        {
                            continue;
                        }
                        NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> dmap =
                            isub.descendingMap();
                        test(dmap.firstKey().equals(isub.lastKey()));
                        test(dmap.lastKey().equals(isub.firstKey()));
                        break;
                    }
                }
            }

            {
                java.util.Set<Integer> keys = sm.descendingKeySet();
                Integer prev = null;
                for(Integer i : keys)
                {
                    if(prev != null)
                    {
                        test(i.compareTo(prev) < 0);
                    }
                    prev = i;
                }
            }

            {
                java.util.Set<String> keys = sm.mapForCategory().descendingKeySet();
                String prev = null;
                for(String category : keys)
                {
                    if(prev != null)
                    {
                        test(category.compareTo(prev) > 0);
                    }
                    prev = category;
                }
            }

            sm.close();

            System.out.println("ok");
        }

        {
            SortedMap sm = new SortedMap(connection, "sortedMap", true, less, indexComparators);

            System.out.print("testing empty map... ");
            System.out.flush();

            sm.clear();

            testEmptyMap(sm);
            testEmptyMap(sm.headMap(0, false));
            testEmptyMap(sm.headMap(0, false).headMap(0, false));
            testEmptyMap(sm.tailMap(0, false));
            testEmptyMap(sm.tailMap(0, false).tailMap(0, false));
            testEmptyMap(sm.subMap(0, false, 1000, false));
            testEmptyMap(sm.subMap(0, false, 1000, false).subMap(0, false, 1000, false));

            sm.close();

            System.out.println("ok");
        }

        connection.close();

        return 0;
    }

    static void
    testSortedMap(NavigableMap<Integer, Ice.Identity> sm, boolean ascending)
    {
        java.util.Comparator<? super Integer> c = sm.comparator();
        java.util.Random rand = new java.util.Random();
        NavigableMap<Integer, Ice.Identity> sub = null;

        final int first = sm.firstKey().intValue();
        final int last = sm.lastKey().intValue();

        testPrimaryKey(sm, null, false, null, false);

        //
        // Head map
        //
        sub = (NavigableMap<Integer, Ice.Identity>)sm.headMap(last);
        test(c.compare(sub.lastKey(), last) < 0);
        sub = sm.headMap(last, true);
        test(c.compare(sub.lastKey(), last) == 0);
        sub = (NavigableMap<Integer, Ice.Identity>)sm.headMap(first);
        test(sub.firstEntry() == null); // map is empty
        sub = sm.headMap(first, true);
        test(c.compare(sub.firstKey(), first) == 0);
        test(c.compare(sub.lastKey(), first) == 0);

        sm.headMap(first, true).headMap(first, true);
        sm.headMap(first, true).headMap(first);
        sm.headMap(first).headMap(first);
        try
        {
            ((NavigableMap<Integer, Ice.Identity>)sm.headMap(first)).headMap(first, true);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Expected.
        }

        {
            int k = 0;
            while(true)
            {
                k = rand.nextInt(1000);
                sub = (NavigableMap<Integer, Ice.Identity>)sm.headMap(k);
                if(sub.firstEntry() != null) // Make sure submap isn't empty.
                {
                    break;
                }
            }

            testPrimaryKey(sub, null, false, k, false);

            try
            {
                sub.tailMap(k);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
                // Expected.
            }
        }

        {
            int k = 0;
            while(true)
            {
                k = rand.nextInt(1000);
                sub = sm.headMap(k, true);
                if(sub.firstEntry() != null) // Make sure submap isn't empty.
                {
                    break;
                }
            }

            testPrimaryKey(sub, null, false, k, true);

            try
            {
                sub.tailMap(k, false);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
                // Expected.
            }
        }

        //
        // Tail map
        //
        sub = (NavigableMap<Integer, Ice.Identity>)sm.tailMap(first);
        test(c.compare(sub.firstKey(), first) == 0);
        sub = sm.tailMap(first, false);
        test(c.compare(sub.firstKey(), first) > 0);
        sub = (NavigableMap<Integer, Ice.Identity>)sm.tailMap(last);
        test(c.compare(sub.firstKey(), last) == 0);
        test(c.compare(sub.lastKey(), last) == 0);
        sub = sm.tailMap(last, false);
        test(sub.firstEntry() == null); // map is empty

        sm.tailMap(last).tailMap(last);
        ((NavigableMap<Integer, Ice.Identity>)sm.tailMap(last)).tailMap(last, false);
        try
        {
            sm.tailMap(last, false).tailMap(last);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Expected.
        }

        {
            int k = 0;
            while(true)
            {
                k = rand.nextInt(1000);
                sub = (NavigableMap<Integer, Ice.Identity>)sm.tailMap(k);
                if(sub.firstEntry() != null) // Make sure submap isn't empty.
                {
                    break;
                }
            }

            testPrimaryKey(sub, k, true, null, false);

            try
            {
                sub.headMap(k);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
                // Expected.
            }
        }

        {
            int k = 0;
            while(true)
            {
                k = rand.nextInt(1000);
                sub = sm.tailMap(k, false);
                if(sub.firstEntry() != null) // Make sure submap isn't empty.
                {
                    break;
                }
            }

            testPrimaryKey(sub, k, false, null, false);

            try
            {
                sub.headMap(k);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
                // Expected.
            }
        }

        //
        // Sub map
        //
        sub = (NavigableMap<Integer, Ice.Identity>)sm.subMap(first, last);
        test(c.compare(sub.firstKey(), first) == 0);
        test(c.compare(sub.lastKey(), last) < 0);
        sub = sm.subMap(first, false, last, false);
        test(c.compare(sub.firstKey(), first) > 0);
        sub = sm.subMap(first, true, last, true);
        test(c.compare(sub.firstKey(), first) == 0);
        test(c.compare(sub.lastKey(), last) == 0);

        try
        {
            sm.subMap(first, false, first, false);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Expected.
        }

        NavigableMap<Integer, Ice.Identity> subsub = null;

        subsub = sub.subMap(first, true, last, true);
        test(c.compare(subsub.firstKey(), first) == 0);
        test(c.compare(subsub.lastKey(), last) == 0);

        subsub = sub.subMap(first, false, last, false);
        test(c.compare(subsub.firstKey(), first) > 0);
        test(c.compare(subsub.lastKey(), last) < 0);

        try
        {
            subsub.subMap(first, true, last, false);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Expected.
        }

        try
        {
            subsub.subMap(first, false, last, true);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Expected.
        }

        {
            final boolean fromInclusive[] = { false, false, true, true };
            final boolean toInclusive[] = { false, true, false, true };
            for(int i = 0; i < 4; ++i)
            {
                int from = 0, to = 0;
                while(true)
                {
                    int f = rand.nextInt(1000);
                    int t = rand.nextInt(1000);
                    if(ascending)
                    {
                        from = Math.min(f, t);
                        to = Math.max(f, t);
                    }
                    else
                    {
                        from = Math.max(f, t);
                        to = Math.min(f, t);
                    }
                    if(Math.abs(to - from) < 100)
                    {
                        continue;
                    }
                    sub = sm.subMap(from, fromInclusive[i], to, toInclusive[i]);
                    if(sub.firstEntry() != null) // Make sure submap isn't empty.
                    {
                        break;
                    }
                }

                testPrimaryKey(sub, from, fromInclusive[i], to, toInclusive[i]);
            }
        }
    }

    static void
    testPrimaryKey(NavigableMap<Integer, Ice.Identity> m, Integer from, boolean fromInclusive, Integer to,
                   boolean toInclusive)
    {
        java.util.Comparator<? super Integer> c = m.comparator();

        Integer first = m.firstKey();
        Integer last = m.lastKey();

        test(inRange(c, first, from, fromInclusive, to, toInclusive));
        test(inRange(c, last, from, fromInclusive, to, toInclusive));

        java.util.Random rand = new java.util.Random();
        int i = 0;
        while(i < 100)
        {
            int k = rand.nextInt(1000);
            if(!inRange(c, k, from, fromInclusive, to, toInclusive))
            {
                continue;
            }

            java.util.Map.Entry<Integer, Ice.Identity> e;
            Integer key;

            key = m.ceilingKey(k);
            if(key == null)
            {
                test(c.compare(k, last) > 0);
            }
            else
            {
                test(c.compare(key, k) >= 0);
            }
            e = m.ceilingEntry(k);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.floorKey(k);
            if(key == null)
            {
                test(c.compare(k, first) < 0);
            }
            else
            {
                test(c.compare(key, k) <= 0);
            }
            e = m.floorEntry(k);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.higherKey(k);
            if(key == null)
            {
                test(c.compare(k, last) >= 0);
            }
            else
            {
                test(c.compare(key, k) > 0);
            }
            e = m.higherEntry(k);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.lowerKey(k);
            if(key == null)
            {
                test(c.compare(k, first) <= 0);
            }
            else
            {
                test(c.compare(key, k) < 0);
            }
            e = m.lowerEntry(k);
            test((key == null && e == null) || key.equals(e.getKey()));

            ++i;
        }

        for(java.util.Map.Entry<Integer, Ice.Identity> p : m.entrySet())
        {
            test(inRange(c, p.getKey(), from, fromInclusive, to, toInclusive));
        }
    }

    static boolean
    inRange(java.util.Comparator<? super Integer> c, Integer val, Integer from, boolean fromInclusive, Integer to,
            boolean toInclusive)
    {
        if(from != null)
        {
            int cmp = c.compare(val, from);
            if((fromInclusive && cmp < 0) || (!fromInclusive && cmp <= 0))
            {
                return false;
            }
        }
        if(to != null)
        {
            int cmp = c.compare(val, to);
            if((toInclusive && cmp > 0) || (!toInclusive && cmp >= 0))
            {
                return false;
            }
        }
        return true;
    }

    static void
    testSecondaryKey(NavigableMap<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> m, String from,
                     boolean fromInclusive, String to, boolean toInclusive)
    {
        java.util.Comparator<? super String> c = m.comparator();

        String first = m.firstKey();
        String last = m.lastKey();

        test(inRange(c, first, from, fromInclusive, to, toInclusive));
        test(inRange(c, last, from, fromInclusive, to, toInclusive));

        java.util.Random rand = new java.util.Random();
        int i = 0;
        while(i < 100)
        {
            String category = String.valueOf(alphabet.charAt(rand.nextInt(1000) % 26));
            if(!inRange(c, category, from, fromInclusive, to, toInclusive))
            {
                continue;
            }

            java.util.Map.Entry<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> e;
            String key;

            key = m.ceilingKey(category);
            if(key == null)
            {
                test(c.compare(category, last) > 0);
            }
            else
            {
                test(c.compare(key, category) >= 0);
            }
            e = m.ceilingEntry(category);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.floorKey(category);
            if(key == null)
            {
                test(c.compare(category, first) < 0);
            }
            else
            {
                test(c.compare(key, category) <= 0);
            }
            e = m.floorEntry(category);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.higherKey(category);
            if(key == null)
            {
                test(c.compare(category, last) >= 0);
            }
            else
            {
                test(c.compare(key, category) > 0);
            }
            e = m.higherEntry(category);
            test((key == null && e == null) || key.equals(e.getKey()));

            key = m.lowerKey(category);
            if(key == null)
            {
                test(c.compare(category, first) <= 0);
            }
            else
            {
                test(c.compare(key, category) < 0);
            }
            e = m.lowerEntry(category);
            test((key == null && e == null) || key.equals(e.getKey()));

            for(java.util.Map.Entry<String, java.util.Set<java.util.Map.Entry<Integer, Ice.Identity>>> p : m.entrySet())
            {
                test(inRange(c, p.getKey(), from, fromInclusive, to, toInclusive));
            }

            ++i;
        }
    }

    static boolean
    inRange(java.util.Comparator<? super String> c, String val, String from, boolean fromInclusive, String to,
            boolean toInclusive)
    {
        if(from != null)
        {
            int cmp = c.compare(val, from);
            if((fromInclusive && cmp < 0) || (!fromInclusive && cmp <= 0))
            {
                return false;
            }
        }
        if(to != null)
        {
            int cmp = c.compare(val, to);
            if((toInclusive && cmp > 0) || (!toInclusive && cmp >= 0))
            {
                return false;
            }
        }
        return true;
    }

    static private void
    testEmptyMap(NavigableMap<Integer, Ice.Identity> m)
    {
        test(m.firstEntry() == null);
        test(m.lastEntry() == null);
        test(m.ceilingEntry(0) == null);
        test(m.floorEntry(0) == null);
        test(m.higherEntry(0) == null);
        test(m.lowerEntry(0) == null);
        test(m.pollFirstEntry() == null);
        test(m.pollLastEntry() == null);

        try
        {
            m.firstKey();
            test(false);
        }
        catch(java.util.NoSuchElementException ex)
        {
            // Expected.
        }
        try
        {
            m.lastKey();
            test(false);
        }
        catch(java.util.NoSuchElementException ex)
        {
            // Expected.
        }

        test(m.ceilingKey(0) == null);
        test(m.floorKey(0) == null);
        test(m.higherKey(0) == null);
        test(m.lowerKey(0) == null);
    }

    @Override
    public int run(String[] args)
    {
        int status;
        String envName = "db";
        if(args.length > 0)
        {
            envName = args[0];
            envName += "/";
            envName += "db";
        }

        return run(args, communicator(), envName, "binary");
    }
    
    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);
        System.gc();
        System.exit(status);
    }
}
