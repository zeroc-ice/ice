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

using System;

public class AllTests
{
    private static void test(bool b)
    {
	if (!b)
	{
	    throw new SystemException();
	}
    }
    
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
    {
	Console.Out.Write("testing stringToProxy... ");
	Console.Out.Flush();
	String ref_Renamed = "test:default -p 12345 -t 2000";
	Ice.ObjectPrx base_Renamed = communicator.stringToProxy(ref_Renamed);
	test(base_Renamed != null);
	Console.Out.WriteLine("ok");
	
	Console.Out.Write("testing checked cast... ");
	Console.Out.Flush();
	Test.MyClassPrx cl = Test.MyClassPrxHelper._checkedCast(base_Renamed);
	test(cl != null);
	Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper._checkedCast(cl);
	test(derived != null);
	test(cl.Equals(base_Renamed));
	test(derived.Equals(base_Renamed));
	test(cl.Equals(derived));
	Console.Out.WriteLine("ok");
	
	Console.Out.Write("testing twoway operations... ");
	Console.Out.Flush();
	Twoways.twoways(cl);
	Twoways.twoways(derived);
	derived.opDerived();
	Console.Out.WriteLine("ok");
	
	if (!collocated)
	{
	    Console.Out.Write("testing twoway operations with AMI... ");
	    Console.Out.Flush();
	    //TwowaysAMI.twowaysAMI(cl);
	    //TwowaysAMI.twowaysAMI(derived);
	    Console.Out.WriteLine("ok");
	}
	
	return cl;
    }
}
