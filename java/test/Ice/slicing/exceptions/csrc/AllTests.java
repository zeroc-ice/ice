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

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "Test:default -p 12345 -t 2000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestPrx test = TestPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        System.out.println("ok");

	System.out.print("testing throwing a base exception... ");
        System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.baseAsBase();
	    }
	    catch(Base b)
	    {
		test(b.b.equals("Base.b"));
		test(b.ice_name().equals("Base"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of unknown derived exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.unknownDerivedAsBase();
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownDerived.b"));
		test(b.ice_name().equals("Base"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing non-slicing of known derived exception thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownDerivedAsBase();
	    }
	    catch(KnownDerived k)
	    {
		test(k.b.equals("KnownDerived.b"));
		test(k.kd.equals("KnownDerived.kd"));
		test(k.ice_name().equals("KnownDerived"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing non-slicing of known derived exception thrown as derived exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownDerivedAsKnownDerived();
	    }
	    catch(KnownDerived k)
	    {
		test(k.b.equals("KnownDerived.b"));
		test(k.kd.equals("KnownDerived.kd"));
		test(k.ice_name().equals("KnownDerived"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of unknown intermediate exception thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.unknownIntermediateAsBase();
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownIntermediate.b"));
		test(b.ice_name().equals("Base"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of known intermediate exception thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownIntermediateAsBase();
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("KnownIntermediate.b"));
		test(ki.ki.equals("KnownIntermediate.ki"));
		test(ki.ice_name().equals("KnownIntermediate"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of known most derived exception thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownMostDerivedAsBase();
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("KnownMostDerived"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing non-slicing of known intermediate exception thrown as intermediate exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownIntermediateAsknownIntermediate();
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("KnownIntermediate.b"));
		test(ki.ki.equals("KnownIntermediate.ki"));
		test(ki.ice_name().equals("KnownIntermediate"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing non-slicing of known most derived thrown as intermediate exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownMostDerivedAsKnownIntermediate();
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("KnownMostDerived"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing non-slicing of known most derived thrown as most derived exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.knownMostDerivedAsKnownMostDerived();
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("KnownMostDerived"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of unknown most derived with known intermediate thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.unknownMostDerived1AsBase();
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("UnknownMostDerived1.b"));
		test(ki.ki.equals("UnknownMostDerived1.ki"));
		test(ki.ice_name().equals("KnownIntermediate"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of unknown most derived with known intermediate thrown as intermediate exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.unknownMostDerived1AsKnownIntermediate();
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("UnknownMostDerived1.b"));
		test(ki.ki.equals("UnknownMostDerived1.ki"));
		test(ki.ice_name().equals("KnownIntermediate"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

	System.out.print("testing slicing of unknown most derived with unknown intermediate thrown as base exception... ");
	System.out.flush();
	{
	    boolean gotException = false;
	    try
	    {
		test.unknownMostDerived2AsBase();
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownMostDerived2.b"));
		test(b.ice_name().equals("Base"));
		gotException = true;
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    test(gotException);
	}
	System.out.println("ok");

        return test;
    }
}
