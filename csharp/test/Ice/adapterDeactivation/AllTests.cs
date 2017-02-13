// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

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
    public static TestIntfPrx allTests(Ice.Communicator communicator)
#endif
    {
        Write("testing stringToProxy... ");
        Flush();
        string @ref = "test:default -p 12010";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        WriteLine("ok");

#if !SILVERLIGHT
        {
            Write("creating/destroying/recreating object adapter... ");
            Flush();
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch (Ice.AlreadyRegisteredException)
            {
            }
            adapter.destroy();

            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            adapter.destroy();
            Console.Out.WriteLine("ok");
        }
#endif
        Write("creating/activating/deactivating object adapter in one operation... ");
        Flush();
        obj.transient();
        obj.end_transient(obj.begin_transient());
        WriteLine("ok");

        {
            Write("testing connection closure... ");
            Flush();
            for(int i = 0; i < 10; ++i)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                Ice.Communicator comm = Ice.Util.initialize(initData);
                comm.stringToProxy("test:default -p 12010").begin_ice_ping();
                comm.destroy();
            }
            WriteLine("ok");
        }

        Write("deactivating object adapter in the server... ");
        Flush();
        obj.deactivate();
        WriteLine("ok");

        Write("testing whether server is gone... ");
        Flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            WriteLine("ok");
        }

#if !SILVERLIGHT
        return obj;
#endif
    }
}
