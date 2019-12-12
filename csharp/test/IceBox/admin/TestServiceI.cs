//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceBox;
using Test;

public class TestServiceI : Service
{
    public TestServiceI(Ice.Communicator serviceManagerCommunicator)
    {
        TestFacetI facet = new TestFacetI();

        //
        // Install a custom admin facet.
        //
        serviceManagerCommunicator.AddAdminFacet<TestFacet, TestFacetTraits>(facet, "TestFacet");

        //
        // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        object propFacet = serviceManagerCommunicator.FindAdminFacet("IceBox.Service.TestService.Properties").servant;
        if (propFacet != null)
        {
            Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
            admin.addUpdateCallback(facet.updated);
        }
    }

    public void
    start(string name, Ice.Communicator communicator, string[] args)
    {
    }

    public void
    stop()
    {
    }
}
