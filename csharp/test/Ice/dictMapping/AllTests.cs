// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

#if SILVERLIGHT
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
#endif

public class AllTests : TestCommon.TestApp
{
#if SILVERLIGHT
    override
    public void run(Ice.Communicator communicator)
#else
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
#endif
    {
#if SILVERLIGHT
        bool collocated = false;
#endif
        Flush();
        string rf = "test:default -p 12010";
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

        Write("testing twoway operations... ");
        Flush();
        Twoways.twoways(communicator, cl);
        WriteLine("ok");

        if(!collocated)
        {
            Write("testing twoway operations with AMI... ");
            Flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            WriteLine("ok");
        }
#if SILVERLIGHT
        cl.shutdown();
#else
        return cl;
#endif
    }
}
