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

    private static int
    run(String[] args, Ice.Communicator communicator)
	throws Test.AlreadyRegisteredException, Test.NotRegisteredException
    {
        String ref = "factory:default -p 12345 -t 30000";
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
	    servants[i] = evictor.createServant(i, i);
	    servants[i].addFacet("facet1", "data");
	    Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
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
	    
	    //
	    // Wait for setValueAsync to be dispatched.
	    //
	    try
	    {
		Thread.sleep(100);
	    }
	    catch(InterruptedException ex)
	    {
	    }
	    
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
	// Remove a facet that does exist
	//
	{
	    servants[0].removeFacet("facet1");
	    Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[0], "facet1");
	    test(facet1 == null);
	}

	//
	// Remove all facets
	//
	for(int i = 0; i < size; i++)
	{
	    servants[i].removeAllFacets();
	}

	evictor.setSize(0);
	evictor.setSize(size);
	
	for(int i = 0; i < size; i++)
	{
	    test(servants[i].getValue() == i + 300);
	    
	    Test.FacetPrx facet1 = Test.FacetPrxHelper.checkedCast(servants[i], "facet1");
	    test(facet1 == null);
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
	    servants[i] = evictor.createServant(i, i);
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
	    servants[i] = evictor.getServant(i);
	    test(servants[i].getValue() == i);
	}
	
	//
	// Clean up.
	//
	evictor.destroyAllServants();
	for(int i = 0; i < size; i++)
	{
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
