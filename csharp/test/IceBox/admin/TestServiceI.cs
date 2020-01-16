//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceBox;
using Test;

public class TestService : IService
{
    public TestService(Ice.Communicator serviceManagerCommunicator)
    {
        var facet = new TestFacet();

        //
        // Install a custom admin facet.
        //
        serviceManagerCommunicator.AddAdminFacet<ITestFacet, TestFacetTraits>(facet, "TestFacet");

        //
        // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        object propFacet = serviceManagerCommunicator.FindAdminFacet("IceBox.Service.TestService.Properties").servant;
        if (propFacet != null)
        {
            var admin = (Ice.INativePropertiesAdmin)propFacet;
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
