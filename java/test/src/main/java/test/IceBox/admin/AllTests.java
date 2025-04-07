// Copyright (c) ZeroC, Inc.

package test.IceBox.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.IceMX.MetricsAdmin;
import com.zeroc.Ice.IceMX.MetricsAdminPrx;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.PropertiesAdminPrx;

import test.IceBox.admin.Test.*;
import test.TestHelper;

import java.util.HashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();

        String ref = "DemoIceBox/admin:default -p 9996 -t 10000";
        ObjectPrx admin = communicator.stringToProxy(ref);

        TestFacetPrx facet = null;

        System.out.print("testing custom facet... ");
        System.out.flush();
        {
            // Test: Verify that the custom facet is present.
            facet = TestFacetPrx.checkedCast(admin, "TestFacet");
            facet.ice_ping();
        }
        System.out.println("ok");

        System.out.print("testing properties facet... ");
        System.out.flush();
        {
            PropertiesAdminPrx pa =
                    PropertiesAdminPrx.checkedCast(
                            admin, "IceBox.Service.TestService.Properties");

            // Test: PropertiesAdmin::getProperty()
            test("1".equals(pa.getProperty("Prop1")));
            test(pa.getProperty("Bogus").isEmpty());

            // Test: PropertiesAdmin::getProperties()
            Map<String, String> pd = pa.getPropertiesForPrefix("");
            test(pd.size() == 6);
            test("1".equals(pd.get("Prop1")));
            test("2".equals(pd.get("Prop2")));
            test("3".equals(pd.get("Prop3")));
            test("config.service".equals(pd.get("Ice.Config")));
            test("IceBox-TestService".equals(pd.get("Ice.ProgramName")));
            test("1".equals(pd.get("Ice.Admin.Enabled")));

            Map<String, String> changes;

            // Test: PropertiesAdmin::setProperties()
            Map<String, String> setProps = new HashMap<>();
            setProps.put("Prop1", "10"); // Changed
            setProps.put("Prop2", "20"); // Changed
            setProps.put("Prop3", ""); // Removed
            setProps.put("Prop4", "4"); // Added
            setProps.put("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test("10".equals(pa.getProperty("Prop1")));
            test("20".equals(pa.getProperty("Prop2")));
            test(pa.getProperty("Prop3").isEmpty());
            test("4".equals(pa.getProperty("Prop4")));
            test("5".equals(pa.getProperty("Prop5")));
            changes = facet.getChanges();
            test(changes.size() == 5);
            test("10".equals(changes.get("Prop1")));
            test("20".equals(changes.get("Prop2")));
            test(changes.get("Prop3").isEmpty());
            test("4".equals(changes.get("Prop4")));
            test("5".equals(changes.get("Prop5")));
            pa.setProperties(setProps);
            changes = facet.getChanges();
            test(changes.isEmpty());
        }
        System.out.println("ok");

        System.out.print("testing metrics admin facet... ");
        System.out.flush();
        {
            MetricsAdminPrx ma =
                    MetricsAdminPrx.checkedCast(
                            admin, "IceBox.Service.TestService.Metrics");

            PropertiesAdminPrx pa =
                    PropertiesAdminPrx.checkedCast(
                            admin, "IceBox.Service.TestService.Properties");

            MetricsAdmin.GetMetricsViewNamesResult r = ma.getMetricsViewNames();
            test(r.returnValue.length == 0);

            Map<String, String> setProps = new HashMap<>();
            setProps.put("IceMX.Metrics.Debug.GroupBy", "id");
            setProps.put("IceMX.Metrics.All.GroupBy", "none");
            setProps.put("IceMX.Metrics.Parent.GroupBy", "parent");
            pa.setProperties(setProps);
            pa.setProperties(new HashMap<>());

            r = ma.getMetricsViewNames();
            test(r.returnValue.length == 3);

            // Make sure that the IceBox communicator metrics admin is a separate instance.
            test(
                    MetricsAdminPrx.checkedCast(admin, "Metrics")
                                    .getMetricsViewNames()
                                    .returnValue
                                    .length
                            == 0);
        }
        System.out.println("ok");
    }

    private AllTests() {
    }
}
