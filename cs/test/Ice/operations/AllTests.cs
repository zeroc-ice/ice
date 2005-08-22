// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class AllTests
{
    private static void test(bool b)
    {
	if (!b)
	{
	    throw new Exception();
	}
    }
    
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
    {
	Console.Out.Write("testing stringToProxy... ");
	Console.Out.Flush();
	string rf = "test:default -p 12345 -t 2000";
	Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
	test(baseProxy != null);
	Console.Out.WriteLine("ok");
    	Console.Out.Write("testing ice_communicator... ");
	Console.Out.Flush();
	test(baseProxy.ice_communicator() == communicator);
	Console.Out.WriteLine("ok");

	Console.Out.Write("testing checked cast... ");
	Console.Out.Flush();
	Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
	test(cl != null);
	Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);
	test(derivedProxy != null);
	test(cl.Equals(baseProxy));
	test(derivedProxy.Equals(baseProxy));
	test(cl.Equals(derivedProxy));
	Console.Out.WriteLine("ok");

	Console.Out.Write("testing checked cast with context... ");
	Console.Out.Flush();
	string cref = "context:default -p 12345 -t 2000";
	Ice.ObjectPrx cbase = communicator.stringToProxy(cref);
	test(cbase != null);

	Test.TestCheckedCastPrx tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase);
	Ice.Context c = tccp.getContext();
	test(c == null || c.Count == 0);

	c = new Ice.Context();
	c["one"] = "hello";
	c["two"] = "world";
	tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase, c);
	Ice.Context c2 = tccp.getContext();
	test(c.Equals(c2));
	Console.Out.WriteLine("ok");
	
	Console.Out.Write("testing twoway operations... ");
	Console.Out.Flush();
	Twoways.twoways(communicator, cl);
	Twoways.twoways(communicator, derivedProxy);
	derivedProxy.opDerived();
	Console.Out.WriteLine("ok");
	
	if (!collocated)
	{
	    Console.Out.Write("testing twoway operations with AMI... ");
	    Console.Out.Flush();
	    TwowaysAMI.twowaysAMI(communicator, cl);
	    TwowaysAMI.twowaysAMI(communicator, derivedProxy);
	    Console.Out.WriteLine("ok");
	}
	
	return cl;
    }
}
