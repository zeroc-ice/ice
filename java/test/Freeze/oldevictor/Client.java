// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class AMI_Servant_setValueAsyncI extends Test.AMI_Servant_setValueAsync
    {
        public void
        ice_response()
        {
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
        }
    }

    static class ReadThread extends  Thread
    {
        ReadThread(Test.ServantPrx[] servants)
        {
            _servants = servants;    
        } 

        public void run()
        {
            int loops = 10;
            while(loops-- > 0)
            {
                try
                {
                    _servants[0].getValue();
                    test(false);
                }
                catch(Ice.ObjectNotExistException e)
                {
                    // Expected
                }
                catch(Exception e)
                {
                    test(false);
                }
                
                for(int i = 1; i < _servants.length; ++i)
                {
                    test(_servants[i].getValue() == i);
                }
            }
        }
        
        private Test.ServantPrx[] _servants;
    }

    public static final int StateRunning = 0;
    public static final int StateDeactivating = 1;
    public static final int StateDeactivated = 2;

    static class ReadForeverThread extends Thread
    {
        ReadForeverThread(Test.ServantPrx[] servants)
        {
            _servants = servants;
        }
        
        public void
        run()
        {
            for(;;)
            {
                try
                {
                    for(int i = 0; i < _servants.length; ++i)
                    {
                        if(getEvictorState() == StateDeactivated)
                        {
                            _servants[i].slowGetValue();
                            test(false);
                        }
                        else
                        {
                            test(_servants[i].slowGetValue() == i);
                        }
                    }
                }
                catch(Ice.SocketException e)
                {
                    //
                    // Expected
                    //
                    test(validEx());
                    return;
                }
                catch(Ice.LocalException e)
                {
                    System.err.println("Caught unexpected : " + e.toString());
                    test(false);
                    return;
                }
                catch(Exception e)
                {
                    test(false);
                    return;
                }
            }
        }

        synchronized int
        getEvictorState()
        {
            return _state;
        }
        
        synchronized boolean
        validEx()
        {
            return _state == StateDeactivating || _state == StateDeactivated;
        }
        
        synchronized void
        setEvictorState(int s)
        {
            _state = s;
        }

        private Test.ServantPrx[] _servants;
        private int _state = StateRunning;
    }


    static class AddForeverThread extends Thread
    {   
        AddForeverThread(Test.RemoteEvictorPrx evictor, int prefix)
        {
            _evictor = evictor;
            _prefix = "" + prefix;
        }

        public void
        run()
        {
            int index = 0;
            
            for(;;)
            {
                try
                {
                    String id = _prefix + "-" + index++;
                    if(getEvictorState() == StateDeactivated)
                    {
                        _evictor.createServant(id, 0);
                        test(false);
                    }
                    else
                    {
                        _evictor.createServant(id, 0);
                    }
                }
                catch(Test.EvictorDeactivatedException e)
                {
                    //
                    // Expected
                    //
                    test(validEx());
                    return;
                }
                catch(Ice.ObjectNotExistException e)
                {
                    //
                    // Expected
                    //
                    test(validEx());
                    return;
                }
                catch(Test.AlreadyRegisteredException e)
                {
                    System.err.println("Caught unexpected AlreadyRegistedException:" + e.toString());
                    System.err.println("index is " + index);
                    test(false);
                    return;
                }
                catch(Ice.LocalException e)
                {
                    System.err.println("Caught unexpected : " + e.toString());
                    e.printStackTrace();
                    test(false);
                    return;
                }
                catch(Exception e)
                {
                    e.printStackTrace();
                    test(false);
                    return;
                }
            }
        }

        synchronized int
        getEvictorState()
        {
            return _state;
        }
        
        synchronized boolean
        validEx()
        {
            return _state == StateDeactivating || _state == StateDeactivated;
        }
        
        synchronized void
        setEvictorState(int s)
        {
            _state = s;
        }

        private Test.RemoteEvictorPrx _evictor;
        private String _prefix;
        private int _state = StateRunning;
    }


    static class CreateDestroyThread extends Thread
    {
        CreateDestroyThread(Test.RemoteEvictorPrx evictor, int id, int size) 
        {
            _evictor = evictor;
            _id = "" + id;
            _size = size;
        }
                   
        public void
        run()
        {
            try
            {
                int loops = 50;
                while(loops-- > 0)
                {
                    for(int i = 0; i < _size; i++)
                    {
                        String id = "" + i;
                        if(id.equals(_id))
                        {
                            //
                            // Create when odd, destroy when even.
                            //
                            
                            if(loops % 2 == 0)
                            {
                                Test.ServantPrx servant = _evictor.getServant(id);
                                servant.destroy();
                                
                                //
                                // Twice
                                //
                                try
                                {
                                    servant.destroy();
                                    test(false);
                                }
                                catch(Ice.ObjectNotExistException e)
                                {
                                    // Expected
                                }
                            }
                            else
                            {
                                Test.ServantPrx servant = _evictor.createServant(id, i);
                                
                                //
                                // Twice
                                //
                                try
                                {
                                    servant = _evictor.createServant(id, 0);
                                    test(false);
                                }
                                catch(Test.AlreadyRegisteredException e)
                                {
                                    // Expected
                                }
                            }
                        }
                        else
                        {
                            //
                            // Just read/write the value
                            //
                            Test.ServantPrx servant = _evictor.getServant(id);
                            try
                            {
                                int val = servant.getValue();
                                test(val == i || val == -i);
                                servant.setValue(-val);
                            }
                            catch(Ice.ObjectNotExistException e)
                            {
                                // Expected from time to time
                            }
                        }
                    }
                }
            }
            catch(Exception e)
            {
                //
                // Unexpected!
                //
                test(false);
            }
        }

        private Test.RemoteEvictorPrx _evictor;
        private String _id;
        private int _size;
    }



    private static int
    run(String[] args, Ice.Communicator communicator)
        throws Test.AlreadyRegisteredException, Test.NotRegisteredException, Test.EvictorDeactivatedException
    {
        String ref = "factory:default -p 12010 -t 30000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        Test.RemoteEvictorFactoryPrx factory = Test.RemoteEvictorFactoryPrxHelper.checkedCast(base);

     
        System.out.print("testing Freeze Evictor... ");
        System.out.flush();

        final int size = 5;
        
        Test.RemoteEvictorPrx evictor = factory.createEvictor("Test");
        evictor.setSize(size);

        //
        // Create some servants 
        //
        Test.ServantPrx[] servants = new Test.ServantPrx[size];
        for(int i = 0; i < size; i++)
        {
            String id = "" + i;
            servants[i] = evictor.createServant(id, i);
            servants[i].ice_ping();
            
            Test.FacetPrx facet1 = Test.FacetPrxHelper.uncheckedCast(servants[i], "facet1");
            try
            {
                facet1.ice_ping();
                test(false);
            }
            catch(Ice.FacetNotExistException e)
            {
                //
                // Expected
                //
            }
            servants[i].addFacet("facet1", "data");
            facet1.ice_ping();
            facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            facet1.setValue(10 * i);
            facet1.addFacet("facet2", "moreData");
            Test.FacetPrx facet2 = Test.FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            facet2.setValue(100 * i);
        }

        //
        // Evict and verify values.
        //
        evictor.setSize(0);
        evictor.setSize(size);
        for(int i = 0; i < size; i++)
        {
            servants[i].ice_ping();
            test(servants[i].getValue() == i);
            Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i);
            test(facet1.getData().equals("data"));
            Test.FacetPrx facet2 = Test.FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            test(facet2.getData().equals("moreData"));
        }
        
        //
        // Mutate servants.
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].setValue(i + 100);
            Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            facet1.setValue(10 * i + 100);
            Test.FacetPrx facet2 = Test.FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            facet2.setValue(100 * i + 100);
        }
        
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getValue() == i + 100);
            Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i + 100);
            Test.FacetPrx facet2 = Test.FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            test(facet2.getValue() == 100 * i + 100);
        }
        
        //
        // Evict and verify values.
        //
        evictor.setSize(0);
        evictor.setSize(size);
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getValue() == i + 100);
            Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i + 100);
            Test.FacetPrx facet2 = Test.FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            test(facet2.getValue() == 100 * i + 100);
        }

        // 
        // Test saving while busy
        //

        AMI_Servant_setValueAsyncI setCB = new AMI_Servant_setValueAsyncI();
        for(int i = 0; i < size; i++)
        {
            //
            // Start a mutating operation so that the object is not idle.
            //
            servants[i].setValueAsync_async(setCB, i + 300);
            
            test(servants[i].getValue() == i + 100);
            //
            // This operation modifies the object state but is not saved
            // because the setValueAsync operation is still pending.
            //
            servants[i].setValue(i + 200);
            test(servants[i].getValue() == i + 200);
            
            //
            // Force the response to setValueAsync
            //
            servants[i].releaseAsync();
            test(servants[i].getValue() == i + 300);
        }


        //
        // Add duplicate facet and catch corresponding exception
        // 
        for(int i = 0; i < size; i++)
        {
            try
            {
                servants[i].addFacet("facet1", "foobar");
                test(false);
            }
            catch(Test.AlreadyRegisteredException ex)
            {
            }
        }
        
        //
        // Remove a facet that does not exist
        // 
        try
        {
            servants[0].removeFacet("facet3");
            test(false);
        }
        catch(Test.NotRegisteredException ex)
        {
        }

        //
        // Remove all facets
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].removeFacet("facet1");
            servants[i].removeFacet("facet2");
        }

        evictor.setSize(0);
        evictor.setSize(size);
        
        //
        // Destroy servants and verify ObjectNotExistException.
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].destroy();
            try
            {
                servants[i].getValue();
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                // Expected
            }

            try
            {
                servants[i].ice_ping();
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                // Expected
            }

        }

        //
        // Allocate space for size servants.
        //
        servants = new Test.ServantPrx[size];

        //
        // Recreate servants, set transient value
        //  
        for(int i = 0; i < size; i++)
        {
            String id = "" + i;
            servants[i] = evictor.createServant(id, i);
            servants[i].setTransientValue(i);
        }

        //
        // Evict
        //
        evictor.saveNow();
        evictor.setSize(0);
        evictor.setSize(size);

        //
        // Check the transient value
        //
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getTransientValue() == -1);
        }
    
        //
        // Now with keep
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].keepInCache();
            servants[i].keepInCache();
            servants[i].setTransientValue(i);
        }
        evictor.saveNow();
        evictor.setSize(0);
        evictor.setSize(size);
        
        //
        // Check the transient value
        //
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getTransientValue() == i);
        }
        
        //
        // Again, after one release
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].release();
        }
        evictor.saveNow();
        evictor.setSize(0);
        evictor.setSize(size);
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getTransientValue() == i);
        }

        //
        // Again, after a second release
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].release();
        }
        evictor.saveNow();
        evictor.setSize(0);
        evictor.setSize(size);
        
        for(int i = 0; i < size; i++)
        {
            test(servants[i].getTransientValue() == -1);
        }
                
        //
        // Release one more time
        //
        for(int i = 0; i < size; i++)
        {
            try
            {
                servants[i].release();
                test(false);
            }
            catch(Test.NotRegisteredException e)
            {
                // Expected
            }
        }
        
        //
        // Deactivate and recreate evictor, to ensure that servants
        // are restored properly after database close and reopen.
        //
        evictor.deactivate();
        evictor = factory.createEvictor("Test");
        evictor.setSize(size);
        for(int i = 0; i < size; i++)
        {
            String id = "" + i;
            servants[i] = evictor.getServant(id);
            test(servants[i].getValue() == i);
        }
        

        //
        // Test concurrent lookups with a smaller evictor
        // size and one missing servant
        //
        evictor.setSize(size / 2);
        servants[0].destroy();
        
        {
            int threadCount = size * 2;
            
            Thread[] threads = new Thread[threadCount];
            for(int i = 0; i < threadCount; i++)
            {
                threads[i] = new ReadThread(servants);
                threads[i].start();
            }
            
            for(int i = 0; i < threadCount; i++)
            {
                for(;;)
                {
                    try
                    {
                        threads[i].join(0);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
            }
        }
        
        //
        // Clean up.
        //
        evictor.destroyAllServants("");
        evictor.destroyAllServants("facet1");
        evictor.destroyAllServants("facet2");
        
        //
        // CreateDestroy threads
        //
        {
            int threadCount = size;;
            
            Thread[] threads = new Thread[threadCount];
            for(int i = 0; i < threadCount; i++)
            {
                threads[i] = new CreateDestroyThread(evictor, i, size);
                threads[i].start();
            }
            
            for(int i = 0; i < threadCount; i++)
            {
                for(;;)
                {
                    try
                    {
                        threads[i].join(0);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
            }
            
            //
            // Verify all destroyed
            // 
            for(int i = 0; i < size; i++)   
            {
                try
                {
                    servants[i].getValue();
                    test(false);
                }
                catch(Ice.ObjectNotExistException e)
                {
                    // Expected
                }
            }
        }
        
        //
        // Recreate servants.
        //  
        servants = new Test.ServantPrx[size];
        for(int i = 0; i < size; i++)
        {
            String id = "" + i;
            servants[i] = evictor.createServant(id, i);
        }
        
        //
        // Deactivate in the middle of remote AMD operations
        // (really testing Ice here)
        //
        {
            int threadCount = size;
            
            Thread[] threads = new Thread[threadCount];
            for(int i = 0; i < threadCount; i++)
            {
                threads[i] = new ReadForeverThread(servants);
                threads[i].start();
            }
            
            try
            {
                Thread.currentThread().sleep(500);
            }
            catch(InterruptedException e)
            {
            }

            for(int i = 0; i < threadCount; i++)
            {
                ReadForeverThread t = (ReadForeverThread)threads[i];
                t.setEvictorState(StateDeactivating);
            }
            evictor.deactivate();
            for(int i = 0; i < threadCount; i++)
            {
                ReadForeverThread t = (ReadForeverThread)threads[i];
                t.setEvictorState(StateDeactivated);
            }
            
            for(int i = 0; i < threadCount; i++)
            {
                for(;;)
                {
                    try
                    {
                        threads[i].join(0);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
            }
        }
        
        //
        // Resurrect
        //
        evictor = factory.createEvictor("Test");
        evictor.destroyAllServants("");
        
        //
        // Deactivate in the middle of adds
        //
        {
            int threadCount = size;
            
            Thread[] threads = new Thread[threadCount];
            for(int i = 0; i < threadCount; i++)
            {
                threads[i] = new AddForeverThread(evictor, i);
                threads[i].start();
            }
            
            try
            {
                Thread.currentThread().sleep(500);
            }
            catch(InterruptedException e)
            {
            }

            for(int i = 0; i < threadCount; i++)
            {
                AddForeverThread t = (AddForeverThread)threads[i];
                t.setEvictorState(StateDeactivating);
            }
            evictor.deactivate();
            for(int i = 0; i < threadCount; i++)
            {
                AddForeverThread t = (AddForeverThread)threads[i];
                t.setEvictorState(StateDeactivated);
            }

            for(int i = 0; i < threadCount; i++)
            {
                for(;;)
                {
                    try
                    {
                        threads[i].join(0);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
            }
        }
        
        
        //
        // Clean up.
        //
        evictor = factory.createEvictor("Test");
        evictor.destroyAllServants("");
        evictor.deactivate();


        System.out.println("ok");
        
        factory.shutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }
        catch(Test.AlreadyRegisteredException ex)
        {
            ex.printStackTrace();
            status = 1;
        }
        catch(Test.NotRegisteredException ex)
        {
            ex.printStackTrace();
            status = 1;
        }
        catch(Test.EvictorDeactivatedException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
