// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class AllTests
{
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Flush();
        string rf = "test:default -p 12010";
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);

        Console.Out.Write("testing twoway operations... ");
        Console.Out.Flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derivedProxy);
        derivedProxy.opDerived();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing oneway operations... ");
        Console.Out.Flush();
        Oneways.oneways(communicator, cl);
        Console.Out.WriteLine("ok");

        if(!collocated)
        {
            Console.Out.Write("testing twoway operations with AMI... ");
            Console.Out.Flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            TwowaysAMI.twowaysAMI(communicator, derivedProxy);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing twoway operations with new AMI mapping... ");
            Console.Out.Flush();
            TwowaysNewAMI.twowaysNewAMI(communicator, cl);
            TwowaysNewAMI.twowaysNewAMI(communicator, derivedProxy);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing oneway operations with AMI... ");
            Console.Out.Flush();
            OnewaysAMI.onewaysAMI(communicator, cl);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing oneway operations with new AMI mapping... ");
            Console.Out.Flush();
            OnewaysNewAMI.onewaysNewAMI(communicator, cl);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing batch oneway operations... ");
            Console.Out.Flush();
            BatchOneways.batchOneways(cl);
            BatchOneways.batchOneways(derivedProxy);
            Console.Out.WriteLine("ok");
        }

        return cl;
    }
}
