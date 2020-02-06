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
        serviceManagerCommunicator.AddAdminFacet("TestFacet", facet);

        //
        // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        Ice.IObject? propFacet = serviceManagerCommunicator.FindAdminFacet("IceBox.Service.TestService.Properties");
        if (propFacet != null)
        {
            var admin = (Ice.INativePropertiesAdmin)propFacet;
            admin.AddUpdateCallback(facet.updated);
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
