// Copyright (c) ZeroC, Inc.

using Test;

public class AllTests : Test.AllTests
{
    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Ice.ObjectPrx admin = communicator.stringToProxy("DemoIceBox/admin:default -p 9996 -t 10000");

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
            test(pa.getProperty("Prop1") == "1");
            test(pa.getProperty("Bogus").Length == 0);

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
            Dictionary<string, string> setProps = new Dictionary<string, string>
            {
                { "Prop1", "10" }, // Changed
                { "Prop2", "20" }, // Changed
                { "Prop3", "" }, // Removed
                { "Prop4", "4" }, // Added
                { "Prop5", "5" } // Added
            };
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1") == "10");
            test(pa.getProperty("Prop2") == "20");
            test(pa.getProperty("Prop3").Length == 0);
            test(pa.getProperty("Prop4") == "4");
            test(pa.getProperty("Prop5") == "5");
            changes = facet.getChanges();
            test(changes.Count == 5);
            test(changes["Prop1"] == "10");
            test(changes["Prop2"] == "20");
            test(changes["Prop3"].Length == 0);
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
            IceMX.MetricsAdminPrx ma =
                IceMX.MetricsAdminPrxHelper.checkedCast(admin, "IceBox.Service.TestService.Metrics");

            Ice.PropertiesAdminPrx pa =
                Ice.PropertiesAdminPrxHelper.checkedCast(admin, "IceBox.Service.TestService.Properties");

            string[] views;
            string[] disabledViews;
            views = ma.getMetricsViewNames(out disabledViews);
            test(views.Length == 0);

            Dictionary<string, string> setProps = new Dictionary<string, string>
            {
                { "IceMX.Metrics.Debug.GroupBy", "id" },
                { "IceMX.Metrics.All.GroupBy", "none" },
                { "IceMX.Metrics.Parent.GroupBy", "parent" }
            };
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
