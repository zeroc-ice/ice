// Copyright (c) ZeroC, Inc.

public class TestServiceI : IceBox.Service
{
    public TestServiceI(Ice.Communicator serviceManagerCommunicator)
    {
        TestFacetI facet = new TestFacetI();

        //
        // Install a custom admin facet.
        //
        serviceManagerCommunicator.addAdminFacet(facet, "TestFacet");

        //
        // Set the callback on the admin facet.
        //
        Ice.Object propFacet = serviceManagerCommunicator.findAdminFacet("IceBox.Service.TestService.Properties");
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
