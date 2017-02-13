// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;
import java.io.PrintWriter;

public class Client extends test.Util.Application
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class ReadThread extends  Thread
    {
        ReadThread(ServantPrx[] servants)
        {
            _servants = servants;
        }

        public void
        run()
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

        private ServantPrx[] _servants;
    }

    public static final int StateRunning = 0;
    public static final int StateDeactivating = 1;
    public static final int StateDeactivated = 2;

    static class ReadForeverThread extends Thread
    {
        ReadForeverThread(ServantPrx[] servants)
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

        private ServantPrx[] _servants;
        private int _state = StateRunning;
    }

    static class AddForeverThread extends Thread
    {
        AddForeverThread(RemoteEvictorPrx evictor, int prefix)
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
                catch(EvictorDeactivatedException e)
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
                catch(AlreadyRegisteredException e)
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

        private RemoteEvictorPrx _evictor;
        private String _prefix;
        private int _state = StateRunning;
    }

    static class CreateDestroyThread extends Thread
    {
        CreateDestroyThread(RemoteEvictorPrx evictor, int id, int size)
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
                                ServantPrx servant = _evictor.getServant(id);
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
                                _evictor.createServant(id, i);

                                //
                                // Twice
                                //
                                try
                                {
                                    _evictor.createServant(id, 0);
                                    test(false);
                                }
                                catch(AlreadyRegisteredException e)
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
                            ServantPrx servant = _evictor.getServant(id);
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

        private RemoteEvictorPrx _evictor;
        private String _id;
        private int _size;
    }

    static class TransferThread extends Thread
    {
        TransferThread(AccountPrx[] accounts, int count)
        {
            _accounts = accounts;
            _random = new java.util.Random(count);
        }

        public void
        run()
        {
            int transferOp = 0;
            long now = System.currentTimeMillis();
            for(int i = 0; i < 500; i++)
            {
                if((System.currentTimeMillis() - now) > (60 * 1000))
                {
                    System.err.println("warning: exiting transfer thread after one minute and " + i + " iterations");
                    break;
                }

                //
                // Transfer 100 at random between two distinct accounts
                //
                AccountPrx from = _accounts[Math.abs(_random.nextInt() % _accounts.length)];

                AccountPrx to = null;
                do
                {
                    to = _accounts[Math.abs(_random.nextInt() % _accounts.length)];
                }
                while(from == to);

                try
                {
                    //
                    // Alternate between transfer methods
                    //
                    switch(transferOp)
                    {
                    case 0:
                    {
                        from.transfer(100, to);
                        break;
                    }
                    case 1:
                    {
                        from.transfer2(100, to);
                        break;
                    }
                    case 2:
                    {
                        from.transfer3(100, to);
                        break;
                    }
                    default:
                    {
                        test(false);
                    }
                    }
                    transferOp++;
                    transferOp = transferOp % 3;
                }
                catch(InsufficientFundsException e)
                {
                    //
                    // Expected from time to time
                    //
                }
                catch(Exception e)
                {
                    //
                    // Unexpected
                    //
                    test(false);
                }

                /*
                if(i % 100 == 0)
                {
                    System.err.print(".");
                    System.err.flush();
                }
                */
            }
        }

        private final AccountPrx[] _accounts;
        private final java.util.Random _random;
    }

    private int
    run(String[] args, PrintWriter out, boolean transactional, boolean shutdown)
        throws AlreadyRegisteredException, NotRegisteredException, EvictorDeactivatedException
    {
        Ice.Communicator communicator = communicator();
        String ref = "factory:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        //
        // Use an invocation tiemout to avoid createEvictor call to hang in case the server
        // fails to load DB classes. The timeout shouldn't be too low since the operation 
        // can take some time to complete on slow machines
        //
        RemoteEvictorFactoryPrx factory = RemoteEvictorFactoryPrxHelper.checkedCast(base.ice_invocationTimeout(60000));

        if(transactional)
        {
            out.print("testing transactional Freeze Evictor... ");
        }
        else
        {
            out.print("testing background-save Freeze Evictor... ");
        }
        out.flush();

        final int size = 5;

        RemoteEvictorPrx evictor = factory.createEvictor("Test", transactional);
        evictor.setSize(size);

        //
        // Create some servants
        //
        ServantPrx[] servants = new ServantPrx[size];
        for(int i = 0; i < size; i++)
        {
            String id = "" + i;
            servants[i] = evictor.createServant(id, i);
            servants[i].ice_ping();

            FacetPrx facet1 = FacetPrxHelper.uncheckedCast(servants[i], "facet1");
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
            facet1 = FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            facet1.setValue(10 * i);
            facet1.addFacet("facet2", "moreData");
            FacetPrx facet2 = FacetPrxHelper.checkedCast(facet1, "facet2");
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
            FacetPrx facet1 = FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i);
            test(facet1.getData().equals("data"));
            FacetPrx facet2 = FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            test(facet2.getData().equals("moreData"));
        }

        //
        // Mutate servants.
        //
        for(int i = 0; i < size; i++)
        {
            servants[i].setValue(i + 100);
            FacetPrx facet1 = FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            facet1.setValue(10 * i + 100);
            FacetPrx facet2 = FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            facet2.setValue(100 * i + 100);
        }

        for(int i = 0; i < size; i++)
        {
            test(servants[i].getValue() == i + 100);
            FacetPrx facet1 = FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i + 100);
            FacetPrx facet2 = FacetPrxHelper.checkedCast(facet1, "facet2");
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
            FacetPrx facet1 = FacetPrxHelper.checkedCast(servants[i], "facet1");
            test(facet1 != null);
            test(facet1.getValue() == 10 * i + 100);
            FacetPrx facet2 = FacetPrxHelper.checkedCast(facet1, "facet2");
            test(facet2 != null);
            test(facet2.getValue() == 100 * i + 100);
        }

        if(!transactional)
        {
            //
            // Test saving while busy
            //
            for(int i = 0; i < size; i++)
            {
                //
                // Start a mutating operation so that the object is not idle.
                //
                servants[i].begin_setValueAsync(i + 300);

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
            catch(AlreadyRegisteredException ex)
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
        catch(NotRegisteredException ex)
        {
        }

        //
        // Call an operation that does not exist on the servant
        //    
        try
        {
            AccountPrxHelper.uncheckedCast(servants[0]).getBalance();
            test(false);
        }
        catch(Ice.OperationNotExistException ex)
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
        servants = new ServantPrx[size];

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

        if(!transactional)
        {
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
                catch(NotRegisteredException e)
                {
                    // Expected
                }
            }
        }

        if(transactional)
        {
            int totalBalance = servants[0].getTotalBalance();
            test(totalBalance == 0);

            AccountPrx[] accounts = servants[0].getAccounts();
            test(accounts.length > 0);

            totalBalance = servants[0].getTotalBalance();
            test(totalBalance > 0);

            int threadCount = accounts.length;

            Thread[] threads = new Thread[threadCount];
            for(int i = 0; i < threadCount; i++)
            {
                threads[i] = new TransferThread(accounts, i);
                threads[i].start();
            }

            for(int i = 0; i < threadCount; i++)
            {
                try
                {
                    threads[i].join();
                }
                catch(InterruptedException e)
                {
                    break; // for
                }
            }

            //
            // Check that the total balance did not change!
            //
            test(totalBalance == servants[0].getTotalBalance());
        }

        //
        // Deactivate and recreate evictor, to ensure that servants
        // are restored properly after database close and reopen.
        //
        evictor.deactivate();
        evictor = factory.createEvictor("Test", transactional);
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
                        threads[i].join();
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
            int threadCount = size;

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
                        threads[i].join();
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
        servants = new ServantPrx[size];
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
                Thread.sleep(500);
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
                        threads[i].join();
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
        evictor = factory.createEvictor("Test", transactional);
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
                Thread.sleep(500);
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
                        threads[i].join();
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
        evictor = factory.createEvictor("Test", transactional);
        evictor.destroyAllServants("");
        evictor.deactivate();

        out.println("ok");

        if(shutdown)
        {
            factory.shutdown();
        }

        return 0;
    }

    public int
    run(String[] args)
    {
        PrintWriter out = getWriter();
        int status = 0;
        try
        {
            status = run(args, out, false, false);
            if(status == 0)
            {
                status = run(args, out, true, true);
            }
        }
        catch(AlreadyRegisteredException ex)
        {
            ex.printStackTrace(out);
            status = 1;
        }
        catch(NotRegisteredException ex)
        {
            ex.printStackTrace(out);
            status = 1;
        }
        catch(EvictorDeactivatedException ex)
        {
            ex.printStackTrace(out);
            status = 1;
        }
        return status;
    }

    protected Ice.InitializationData
    getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Freeze.evictor");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
