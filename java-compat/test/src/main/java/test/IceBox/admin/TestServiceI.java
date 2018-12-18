// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceBox.admin;

public class TestServiceI implements IceBox.Service
{
    public TestServiceI(Ice.Communicator serviceManagerCommunicator)
    {
        TestFacetI facet = new TestFacetI();

        //
        // Install a custom admin facet.
        //
        serviceManagerCommunicator.addAdminFacet(facet, "TestFacet");

        //
        // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        Ice.Object propFacet = serviceManagerCommunicator.findAdminFacet("IceBox.Service.TestService.Properties");
        if(propFacet != null)
        {
            Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
            admin.addUpdateCallback(facet);
        }
    }

    @Override
    public void
    start(String name, Ice.Communicator communicator, String[] args)
    {
    }

    @Override
    public void
    stop()
    {
    }
}
