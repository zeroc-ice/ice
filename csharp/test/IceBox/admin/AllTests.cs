//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using ZeroC.Ice;
using ZeroC.IceMX;
using Test;

namespace ZeroC.IceBox.Test.Admin
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var admin = IObjectPrx.Parse("DemoIceBox/admin:default -p 9996 -t 10000", communicator);

            ITestFacetPrx? facet;

            Console.Out.Write("testing custom facet... ");
            Console.Out.Flush();
            {
                //
                // Test: Verify that the custom facet is present.
                //
                facet = ITestFacetPrx.CheckedCast(admin.Clone(facet: "TestFacet", IObjectPrx.Factory));
                TestHelper.Assert(facet != null);
                facet.IcePing();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing properties facet... ");
            Console.Out.Flush();
            {
                var pa = IPropertiesAdminPrx.CheckedCast(
                    admin.Clone(facet: "IceBox.Service.TestService.Properties", IObjectPrx.Factory));

                //
                // Test: PropertiesAdmin::getProperty()
                //
                TestHelper.Assert(pa != null);
                TestHelper.Assert(pa.GetProperty("Prop1") == "1");
                TestHelper.Assert(pa.GetProperty("Bogus").Length == 0);

                //
                // Test: PropertiesAdmin::getProperties()
                //
                Dictionary<string, string> pd = pa.GetPropertiesForPrefix("");
                TestHelper.Assert(pd.Count == 6);
                TestHelper.Assert(pd["Prop1"] == "1");
                TestHelper.Assert(pd["Prop2"] == "2");
                TestHelper.Assert(pd["Prop3"] == "3");
                TestHelper.Assert(pd["Ice.Config"] == "config.service");
                TestHelper.Assert(pd["Ice.ProgramName"] == "IceBox-TestService");
                TestHelper.Assert(pd["Ice.Admin.Enabled"] == "1");

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
                pa.SetProperties(setProps);
                TestHelper.Assert(pa.GetProperty("Prop1") == "10");
                TestHelper.Assert(pa.GetProperty("Prop2") == "20");
                TestHelper.Assert(pa.GetProperty("Prop3").Length == 0);
                TestHelper.Assert(pa.GetProperty("Prop4") == "4");
                TestHelper.Assert(pa.GetProperty("Prop5") == "5");
                changes = facet.GetChanges();
                TestHelper.Assert(changes.Count == 5);
                TestHelper.Assert(changes["Prop1"] == "10");
                TestHelper.Assert(changes["Prop2"] == "20");
                TestHelper.Assert(changes["Prop3"].Length == 0);
                TestHelper.Assert(changes["Prop4"] == "4");
                TestHelper.Assert(changes["Prop5"] == "5");
                pa.SetProperties(setProps);
                changes = facet.GetChanges();
                TestHelper.Assert(changes.Count == 0);
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing metrics admin facet... ");
            Console.Out.Flush();
            {
                var ma = IMetricsAdminPrx.CheckedCast(
                    admin.Clone(facet: "IceBox.Service.TestService.Metrics", IObjectPrx.Factory));
                var pa = IPropertiesAdminPrx.CheckedCast(
                    admin.Clone(facet: "IceBox.Service.TestService.Properties", IObjectPrx.Factory));

                TestHelper.Assert(ma != null && pa != null);
                string[] views = ma.GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 0);

                Dictionary<string, string> setProps = new Dictionary<string, string>();
                setProps.Add("IceMX.Metrics.Debug.GroupBy", "id");
                setProps.Add("IceMX.Metrics.All.GroupBy", "none");
                setProps.Add("IceMX.Metrics.Parent.GroupBy", "parent");
                pa.SetProperties(setProps);
                pa.SetProperties(new Dictionary<string, string>());

                views = ma.GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 3);

                // Make sure that the IceBox communicator metrics admin is a separate instance.
                views = admin.Clone(facet: "Metrics", IMetricsAdminPrx.Factory).GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 0);
            }
            Console.Out.WriteLine("ok");
        }
    }
}
