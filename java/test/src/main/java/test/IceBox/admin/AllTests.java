// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.admin;

import test.IceBox.admin.Test.*;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();

        String ref = "DemoIceBox/admin:default -p 9996 -t 10000";
        com.zeroc.Ice.ObjectPrx admin = communicator.stringToProxy(ref);

        TestFacetPrx facet = null;

        System.out.print("testing custom facet... ");
        System.out.flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            facet = TestFacetPrx.checkedCast(admin, "TestFacet");
            facet.ice_ping();
        }
        System.out.println("ok");

        System.out.print("testing properties facet... ");
        System.out.flush();
        {
            com.zeroc.Ice.PropertiesAdminPrx pa =
                com.zeroc.Ice.PropertiesAdminPrx.checkedCast(admin, "IceBox.Service.TestService.Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop1").equals("1"));
            test(pa.getProperty("Bogus").equals(""));

            //
            // Test: PropertiesAdmin::getProperties()
            //
            java.util.Map<String, String> pd = pa.getPropertiesForPrefix("");
            test(pd.size() == 6);
            test(pd.get("Prop1").equals("1"));
            test(pd.get("Prop2").equals("2"));
            test(pd.get("Prop3").equals("3"));
            test(pd.get("Ice.Config").equals("config.service"));
            test(pd.get("Ice.ProgramName").equals("IceBox-TestService"));
            test(pd.get("Ice.Admin.Enabled").equals("1"));

            java.util.Map<String, String> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            java.util.Map<String, String> setProps = new java.util.HashMap<>();
            setProps.put("Prop1", "10"); // Changed
            setProps.put("Prop2", "20"); // Changed
            setProps.put("Prop3", ""); // Removed
            setProps.put("Prop4", "4"); // Added
            setProps.put("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1").equals("10"));
            test(pa.getProperty("Prop2").equals("20"));
            test(pa.getProperty("Prop3").equals(""));
            test(pa.getProperty("Prop4").equals("4"));
            test(pa.getProperty("Prop5").equals("5"));
            changes = facet.getChanges();
            test(changes.size() == 5);
            test(changes.get("Prop1").equals("10"));
            test(changes.get("Prop2").equals("20"));
            test(changes.get("Prop3").equals(""));
            test(changes.get("Prop4").equals("4"));
            test(changes.get("Prop5").equals("5"));
            pa.setProperties(setProps);
            changes = facet.getChanges();
            test(changes.isEmpty());
        }
        System.out.println("ok");

        System.out.print("testing metrics admin facet... ");
        System.out.flush();
        {
            com.zeroc.IceMX.MetricsAdminPrx ma =
                com.zeroc.IceMX.MetricsAdminPrx.checkedCast(admin, "IceBox.Service.TestService.Metrics");

            com.zeroc.Ice.PropertiesAdminPrx pa =
                com.zeroc.Ice.PropertiesAdminPrx.checkedCast(admin, "IceBox.Service.TestService.Properties");

            com.zeroc.IceMX.MetricsAdmin.GetMetricsViewNamesResult r = ma.getMetricsViewNames();
            test(r.returnValue.length == 0);

            java.util.Map<String, String> setProps = new java.util.HashMap<>();
            setProps.put("IceMX.Metrics.Debug.GroupBy", "id");
            setProps.put("IceMX.Metrics.All.GroupBy", "none");
            setProps.put("IceMX.Metrics.Parent.GroupBy", "parent");
            pa.setProperties(setProps);
            pa.setProperties(new java.util.HashMap<>());

            r = ma.getMetricsViewNames();
            test(r.returnValue.length == 3);

            // Make sure that the IceBox communicator metrics admin is a separate instance.
            test(com.zeroc.IceMX.MetricsAdminPrx.checkedCast(admin, "Metrics").
                 getMetricsViewNames().returnValue.length == 0);
        }
        System.out.println("ok");
    }
}
