// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
    allTests(Ice.Communicator communicator, boolean collocated, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", "Test:default -p 12010 -t 2000");
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx test = TestIntfPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        out.println("ok");

	out.print("base... ");
        out.flush();
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
	out.println("ok");

	out.print("slicing of unknown derived... ");
	out.flush();
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
	out.println("ok");

	out.print("non-slicing of known derived as base... ");
	out.flush();
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
	out.println("ok");

	out.print("non-slicing of known derived as derived... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of unknown intermediate as base... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of known intermediate as base... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of known most derived as base... ");
	out.flush();
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
	out.println("ok");

	out.print("non-slicing of known intermediate as intermediate... ");
	out.flush();
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
	out.println("ok");

	out.print("non-slicing of known most derived as intermediate... ");
	out.flush();
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
	out.println("ok");

	out.print("non-slicing of known most derived as most derived... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of unknown most derived, known intermediate as base... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of unknown most derived, known intermediate as intermediate... ");
	out.flush();
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
	out.println("ok");

	out.print("slicing of unknown most derived, unknown intermediate thrown as base... ");
	out.flush();
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
	out.println("ok");

        return test;
    }
}
