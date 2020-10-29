// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using Test;
using ZeroC.Ice;
using ZeroC.IceMX;

namespace ZeroC.IceBox.Test.Admin
{
    public static class AllTests
    {
        public static void Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            var admin = IObjectPrx.Parse("DemoIceBox/admin:default -h localhost -p 9996 -t 10000", communicator);

            ITestFacetPrx? facet;

            Console.Out.Write("testing custom facet... ");
            Console.Out.Flush();
            {
                // Test: Verify that the custom facet is present.
                facet = admin.Clone(IObjectPrx.Factory, facet: "TestFacet").Clone(ITestFacetPrx.Factory);
                TestHelper.Assert(facet != null);
                facet.IcePing();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing properties facet... ");
            Console.Out.Flush();
            {
                IPropertiesAdminPrx pa = admin.Clone(IObjectPrx.Factory,
                    facet: "IceBox.Service.TestService.Properties").Clone(IPropertiesAdminPrx.Factory);

                // Test: PropertiesAdmin.GetProperty()
                TestHelper.Assert(pa != null);
                TestHelper.Assert(pa.GetProperty("Prop1") == "1");
                TestHelper.Assert(pa.GetProperty("Bogus").Length == 0);

                // Test: PropertiesAdmin.GetProperties()
                Dictionary<string, string> pd = pa.GetPropertiesForPrefix("");
                TestHelper.Assert(pd.Count == 6);
                TestHelper.Assert(pd["Prop1"] == "1");
                TestHelper.Assert(pd["Prop2"] == "2");
                TestHelper.Assert(pd["Prop3"] == "3");
                TestHelper.Assert(pd["Ice.Config"] == "config.service");
                TestHelper.Assert(pd["Ice.ProgramName"] == "IceBox-TestService");
                TestHelper.Assert(pd["Ice.Admin.Enabled"] == "1");

                Dictionary<string, string> changes;

                // Test: PropertiesAdmin.SetProperties()
                var setProps = new Dictionary<string, string>
                {
                    { "Prop1", "10" }, // Changed
                    { "Prop2", "20" }, // Changed
                    { "Prop3", "" }, // Removed
                    { "Prop4", "4" }, // Added
                    { "Prop5", "5" } // Added
                };
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
                IMetricsAdminPrx? ma = admin.Clone(
                    IObjectPrx.Factory,
                    facet: "IceBox.Service.TestService.Metrics").CheckedCast(IMetricsAdminPrx.Factory);

                IPropertiesAdminPrx? pa = admin.Clone(
                    IObjectPrx.Factory,
                    facet: "IceBox.Service.TestService.Properties").CheckedCast(IPropertiesAdminPrx.Factory);

                TestHelper.Assert(ma != null && pa != null);
                string[] views = ma.GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 0);

                var setProps = new Dictionary<string, string>
                {
                    { "IceMX.Metrics.Debug.GroupBy", "id" },
                    { "IceMX.Metrics.All.GroupBy", "none" },
                    { "IceMX.Metrics.Parent.GroupBy", "parent" }
                };
                pa.SetProperties(setProps);
                pa.SetProperties(new Dictionary<string, string>());

                views = ma.GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 3);

                // Make sure that the IceBox communicator metrics admin is a separate instance.
                views = admin.Clone(IMetricsAdminPrx.Factory, facet: "Metrics").GetMetricsViewNames().ReturnValue;
                TestHelper.Assert(views.Length == 0);
            }
            Console.Out.WriteLine("ok");
        }
    }
}
