// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
	String rf = "test:default -p 12345 -t 2000";
	Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
	test(baseProxy != null);
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
	
	Console.Out.Write("testing twoway operations... ");
	Console.Out.Flush();
	Twoways.twoways(cl);
	Twoways.twoways(derivedProxy);
	derivedProxy.opDerived();
	Console.Out.WriteLine("ok");
	
	if (!collocated)
	{
	    Console.Out.Write("testing twoway operations with AMI... ");
	    Console.Out.Flush();
	    //TwowaysAMI.twowaysAMI(cl);
	    //TwowaysAMI.twowaysAMI(derivedProxy);
	    Console.Out.WriteLine("ok");
	}
	
	return cl;
    }
}
