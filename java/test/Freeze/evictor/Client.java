// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        String ref = "factory:default -p 12345 -t 2000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        Test.RemoteEvictorFactoryPrx factory = Test.RemoteEvictorFactoryPrxHelper.checkedCast(base);

        //
        // Test SaveUponEviction mode
        //
        {
            System.out.print("testing SaveUponEviction mode... ");
            System.out.flush();

            final int size = 5;

            Test.RemoteEvictorPrx evictor =
                factory.createEvictor("saveUponEviction", Test.EvictorPersistenceMode.SaveUponEviction);
            evictor.setSize(size);

            //
            // Create the same number of servants as the evictor size
            // (i.e., don't exceed queue size). Servants should be
            // saved immediately.
            //
            Test.ServantPrx[] servants = new Test.ServantPrx[size];
            for(int i = 0; i < size; i++)
            {
                servants[i] = evictor.createServant(i);
                test(evictor.getLastSavedValue() == i);
            }

            //
            // Evict and verify values.
            //
            evictor.setSize(0);
            evictor.setSize(size);
            evictor.clearLastSavedValue();
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i);
            }

            //
            // Mutate servants.
            //
            for(int i = 0; i < size; i++)
            {
                servants[i].setValue(i + 100);
            }

            //
            // Servants should not be saved yet.
            //
            test(evictor.getLastSavedValue() == -1);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i + 100);
            }

            //
            // Evict and verify values.
            //
            evictor.setSize(0);
            evictor.setSize(size);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i + 100);
            }

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
            }

            //
            // Allocate space for size+1 servants.
            //
            servants = new Test.ServantPrx[size + 1];

            //
            // Recreate servants.
            //
            for(int i = 0; i < size; i++)
            {
                servants[i] = evictor.createServant(i);
            }

            //
            // Deactivate and recreate evictor, to ensure that servants
            // are restored properly after database close and reopen.
            //
            evictor.deactivate();
            evictor = factory.createEvictor("saveUponEviction", Test.EvictorPersistenceMode.SaveUponEviction);
            evictor.setSize(size);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i);
            }

            //
            // No servants should have been saved yet.
            //
            test(evictor.getLastSavedValue() == -1);

            //
            // Create new servant - should cause eviction.
            //
            servants[size] = evictor.createServant(size);
            test(evictor.getLastSavedValue() == 0);

            //
            // Restore the evicted servant, which evicts another
            // servant, and so on.
            //
            for(int i = 0; i <= size; i++)
            {
                test(servants[i].getValue() == i);
                test(evictor.getLastSavedValue() == (i + 1) % (size + 1));
            }

            //
            // Destroy new servant and verify eviction no longer occurs.
            //
            servants[size].destroy();
            evictor.clearLastSavedValue();
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i);
            }
            test(evictor.getLastSavedValue() == -1);

            //
            // Clean up.
            //
            for(int i = 0; i < size; i++)
            {
                servants[i].destroy();
            }

            System.out.println("ok");
        }

        //
        // Test SaveAfterMutatingOperation mode
        //
        {
            System.out.print("testing SaveAfterMutatingOperation mode... ");
            System.out.flush();

            final int size = 5;

            Test.RemoteEvictorPrx evictor =
                factory.createEvictor("saveAfterMutatingOperation",
                                      Test.EvictorPersistenceMode.SaveAfterMutatingOperation);
            evictor.setSize(size);

            //
            // Create the same number of servants as the evictor size
            // (i.e., don't exceed queue size). Servants should be
            // saved immediately.
            //
            Test.ServantPrx[] servants = new Test.ServantPrx[size];
            for(int i = 0; i < size; i++)
            {
                servants[i] = evictor.createServant(i);
                test(evictor.getLastSavedValue() == i);
            }

            //
            // Evict and verify values.
            //
            evictor.setSize(0);
            evictor.setSize(size);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i);
            }

            //
            // Mutate servants and verify they have been saved.
            //
            for(int i = 0; i < size; i++)
            {
                servants[i].setValue(i + 100);
                test(evictor.getLastSavedValue() == i + 100);
            }

            //
            // Evict and verify values.
            //
            evictor.setSize(0);
            evictor.setSize(size);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i + 100);
            }

            //
            // No servants should have been saved yet.
            //
            test(evictor.getLastSavedValue() == -1);

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
            }

            //
            // Allocate space for size+1 servants.
            //
            servants = new Test.ServantPrx[size + 1];

            //
            // Recreate servants.
            //
            for(int i = 0; i < size; i++)
            {
                servants[i] = evictor.createServant(i);
            }

            //
            // Deactivate and recreate evictor, to ensure that servants
            // are restored properly after database close and reopen.
            //
            evictor.deactivate();
            evictor = factory.createEvictor("saveAfterMutatingOperation",
                                            Test.EvictorPersistenceMode.SaveAfterMutatingOperation);
            evictor.setSize(size);
            for(int i = 0; i < size; i++)
            {
                test(servants[i].getValue() == i);
            }

            //
            // No servants should have been saved yet.
            //
            test(evictor.getLastSavedValue() == -1);

            //
            // Create new servant - should cause eviction but no
            // servants should be saved.
            //
            servants[size] = evictor.createServant(size);
            test(evictor.getLastSavedValue() == size);

            //
            // Restore the evicted servant, which evicts another
            // servant, and so on.
            //
            for(int i = 0; i <= size; i++)
            {
                test(servants[i].getValue() == i);
            }
            test(evictor.getLastSavedValue() == -1);

            //
            // Clean up.
            //
            for(int i = 0; i <= size; i++)
            {
                servants[i].destroy();
            }

            System.out.println("ok");
        }

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

        System.exit(status);
    }
}
