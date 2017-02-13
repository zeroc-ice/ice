// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;

using Test;

public class AllTests
{
    private static void
    test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static void allTests(Ice.Communicator communicator)
    {
        string @ref = "DemoIceBox/admin:default -p 9996 -t 10000";
        Ice.ObjectPrx admin = communicator.stringToProxy(@ref);

        TestFacetPrx facet = null;

        Console.Out.Write("testing custom facet... ");
        Console.Out.Flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            facet = TestFacetPrxHelper.checkedCast(admin, "TestFacet");
            facet.ice_ping();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing properties facet... ");
        Console.Out.Flush();
        {
            Ice.PropertiesAdminPrx pa =
                Ice.PropertiesAdminPrxHelper.checkedCast(admin, "IceBox.Service.TestService.Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop1").Equals("1"));
            test(pa.getProperty("Bogus").Equals(""));

            //
            // Test: PropertiesAdmin::getProperties()
            //
            Dictionary<string, string> pd = pa.getPropertiesForPrefix("");
            test(pd.Count == 6);
            test(pd["Prop1"].Equals("1"));
            test(pd["Prop2"].Equals("2"));
            test(pd["Prop3"].Equals("3"));
            test(pd["Ice.Config"].Equals("config.service"));
            test(pd["Ice.ProgramName"].Equals("IceBox-TestService"));
            test(pd["Ice.Admin.Enabled"].Equals("1"));

            Dictionary<string, string> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            Dictionary<string, string> setProps = new Dictionary<string, string>();
            setProps.Add("Prop1", "10"); // Changed
            setProps.Add("Prop2", "20"); // Changed
            setProps.Add("Prop3", ""); // Removed
            setProps.Add("Prop4", "4"); // Added
            setProps.Add("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1").Equals("10"));
            test(pa.getProperty("Prop2").Equals("20"));
            test(pa.getProperty("Prop3").Equals(""));
            test(pa.getProperty("Prop4").Equals("4"));
            test(pa.getProperty("Prop5").Equals("5"));
            changes = facet.getChanges();
            test(changes.Count == 5);
            test(changes["Prop1"].Equals("10"));
            test(changes["Prop2"].Equals("20"));
            test(changes["Prop3"].Equals(""));
            test(changes["Prop4"].Equals("4"));
            test(changes["Prop5"].Equals("5"));
            pa.setProperties(setProps);
            changes = facet.getChanges();
            test(changes.Count == 0);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing metrics admin facet... ");
        Console.Out.Flush();
        {
            IceMX.MetricsAdminPrx ma = 
                IceMX.MetricsAdminPrxHelper.checkedCast(admin, "IceBox.Service.TestService.Metrics");

            Ice.PropertiesAdminPrx pa =
                Ice.PropertiesAdminPrxHelper.checkedCast(admin, "IceBox.Service.TestService.Properties");

            string[] views;
            string[] disabledViews;
            views = ma.getMetricsViewNames(out disabledViews);
            test(views.Length == 0);

            Dictionary<string, string> setProps = new Dictionary<string, string>();
            setProps.Add("IceMX.Metrics.Debug.GroupBy", "id");
            setProps.Add("IceMX.Metrics.All.GroupBy", "none");
            setProps.Add("IceMX.Metrics.Parent.GroupBy", "parent");
            pa.setProperties(setProps);
            pa.setProperties(new Dictionary<string, string>());

            views = ma.getMetricsViewNames(out disabledViews);
            test(views.Length == 3);
        
            // Make sure that the IceBox communicator metrics admin is a separate instance.
            test(IceMX.MetricsAdminPrxHelper.checkedCast(admin, 
                                                         "Metrics").getMetricsViewNames(out disabledViews).Length == 0);
        }
        Console.Out.WriteLine("ok");
    }
}
