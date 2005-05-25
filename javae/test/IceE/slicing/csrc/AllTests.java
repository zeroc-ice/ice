// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

    private static class Callback
    {
	Callback()
	{
	    _called = false;
	}

	public synchronized boolean
	check()
	{
	    while(!_called)
	    {
		try
		{
		    wait(5000);
		}
		catch(InterruptedException ex)
		{
		    continue;
		}

		if(!_called)
		{
		    return false; // Must be timeout.
		}
	    }

	    _called = false;
	    return true;
	}
	
	public synchronized void
	called()
	{
	    IceUtil.Debug.Assert(!_called);
	    _called = true;
	    notify();
	}

	private boolean _called;
    }

    public static TestIntfPrx
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
        TestIntfPrx test = TestIntfPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        System.out.println("ok");

	System.out.print("base... ");
        System.out.flush();
	{
	    try
	    {
		test.baseAsBase();
		test(false);
	    }
	    catch(Base b)
	    {
		test(b.b.equals("Base.b"));
		test(b.ice_name().equals("Test::Base"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of unknown derived... ");
	System.out.flush();
	{
	    try
	    {
		test.unknownDerivedAsBase();
		test(false);
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownDerived.b"));
		test(b.ice_name().equals("Test::Base"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("non-slicing of known derived as base... ");
	System.out.flush();
	{
	    try
	    {
		test.knownDerivedAsBase();
		test(false);
	    }
	    catch(KnownDerived k)
	    {
		test(k.b.equals("KnownDerived.b"));
		test(k.kd.equals("KnownDerived.kd"));
		test(k.ice_name().equals("Test::KnownDerived"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("non-slicing of known derived as derived... ");
	System.out.flush();
	{
	    try
	    {
		test.knownDerivedAsKnownDerived();
		test(false);
	    }
	    catch(KnownDerived k)
	    {
		test(k.b.equals("KnownDerived.b"));
		test(k.kd.equals("KnownDerived.kd"));
		test(k.ice_name().equals("Test::KnownDerived"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of unknown intermediate as base... ");
	System.out.flush();
	{
	    try
	    {
		test.unknownIntermediateAsBase();
		test(false);
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownIntermediate.b"));
		test(b.ice_name().equals("Test::Base"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of known intermediate as base... ");
	System.out.flush();
	{
	    try
	    {
		test.knownIntermediateAsBase();
		test(false);
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("KnownIntermediate.b"));
		test(ki.ki.equals("KnownIntermediate.ki"));
		test(ki.ice_name().equals("Test::KnownIntermediate"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of known most derived as base... ");
	System.out.flush();
	{
	    try
	    {
		test.knownMostDerivedAsBase();
		test(false);
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("Test::KnownMostDerived"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("non-slicing of known intermediate as intermediate... ");
	System.out.flush();
	{
	    try
	    {
		test.knownIntermediateAsKnownIntermediate();
		test(false);
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("KnownIntermediate.b"));
		test(ki.ki.equals("KnownIntermediate.ki"));
		test(ki.ice_name().equals("Test::KnownIntermediate"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("non-slicing of known most derived as intermediate... ");
	System.out.flush();
	{
	    try
	    {
		test.knownMostDerivedAsKnownIntermediate();
		test(false);
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("Test::KnownMostDerived"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("non-slicing of known most derived as most derived... ");
	System.out.flush();
	{
	    try
	    {
		test.knownMostDerivedAsKnownMostDerived();
		test(false);
	    }
	    catch(KnownMostDerived kmd)
	    {
		test(kmd.b.equals("KnownMostDerived.b"));
		test(kmd.ki.equals("KnownMostDerived.ki"));
		test(kmd.kmd.equals("KnownMostDerived.kmd"));
		test(kmd.ice_name().equals("Test::KnownMostDerived"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of unknown most derived, known intermediate as base... ");
	System.out.flush();
	{
	    try
	    {
		test.unknownMostDerived1AsBase();
		test(false);
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("UnknownMostDerived1.b"));
		test(ki.ki.equals("UnknownMostDerived1.ki"));
		test(ki.ice_name().equals("Test::KnownIntermediate"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of unknown most derived, known intermediate as intermediate... ");
	System.out.flush();
	{
	    try
	    {
		test.unknownMostDerived1AsKnownIntermediate();
		test(false);
	    }
	    catch(KnownIntermediate ki)
	    {
		test(ki.b.equals("UnknownMostDerived1.b"));
		test(ki.ki.equals("UnknownMostDerived1.ki"));
		test(ki.ice_name().equals("Test::KnownIntermediate"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

	System.out.print("slicing of unknown most derived, unknown intermediate thrown as base... ");
	System.out.flush();
	{
	    try
	    {
		test.unknownMostDerived2AsBase();
		test(false);
	    }
	    catch(Base b)
	    {
		test(b.b.equals("UnknownMostDerived2.b"));
		test(b.ice_name().equals("Test::Base"));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}
	System.out.println("ok");

        return test;
    }
}
