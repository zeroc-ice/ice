//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

using Test;
using Ice;

public class AllTests : Test.AllTests
{
    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        var admin = IObjectPrx.Parse("DemoIceBox/admin:default -p 9996 -t 10000", communicator);

        TestFacetPrx facet = null;

        Console.Out.Write("testing custom facet... ");
        Console.Out.Flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            facet = TestFacetPrx.CheckedCast(admin.Clone(facet: "TestFacet"));
            facet.IcePing();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing properties facet... ");
        Console.Out.Flush();
        {
            var pa = PropertiesAdminPrx.CheckedCast(admin.Clone(facet: "IceBox.Service.TestService.Properties"));

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop1") == "1");
            test(pa.getProperty("Bogus") == "");

            //
            // Test: PropertiesAdmin::getProperties()
            //
            Dictionary<string, string> pd = pa.getPropertiesForPrefix("");
            test(pd.Count == 6);
            test(pd["Prop1"] == "1");
            test(pd["Prop2"] == "2");
            test(pd["Prop3"] == "3");
            test(pd["Ice.Config"] == "config.service");
            test(pd["Ice.ProgramName"] == "IceBox-TestService");
            test(pd["Ice.Admin.Enabled"] == "1");

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
            test(pa.getProperty("Prop1") == "10");
            test(pa.getProperty("Prop2") == "20");
            test(pa.getProperty("Prop3") == "");
            test(pa.getProperty("Prop4") == "4");
            test(pa.getProperty("Prop5") == "5");
            changes = facet.getChanges();
            test(changes.Count == 5);
            test(changes["Prop1"] == "10");
            test(changes["Prop2"] == "20");
            test(changes["Prop3"] == "");
            test(changes["Prop4"] == "4");
            test(changes["Prop5"] == "5");
            pa.setProperties(setProps);
            changes = facet.getChanges();
            test(changes.Count == 0);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing metrics admin facet... ");
        Console.Out.Flush();
        {
            var ma = IceMX.MetricsAdminPrx.CheckedCast(admin.Clone(facet: "IceBox.Service.TestService.Metrics"));
            var pa = PropertiesAdminPrx.CheckedCast(admin.Clone(facet: "IceBox.Service.TestService.Properties"));

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
            test(IceMX.MetricsAdminPrx.CheckedCast(admin.Clone(facet: "Metrics")).getMetricsViewNames(out disabledViews).Length == 0);
        }
        Console.Out.WriteLine("ok");
    }
}
