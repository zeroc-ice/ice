//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Admin
{
    public class TestService : IService
    {
        public TestService(Communicator serviceManagerCommunicator)
        {
            var facet = new TestFacet();

            // Install a custom admin facet.
            serviceManagerCommunicator.AddAdminFacet("TestFacet", facet);

            // The TestFacetI servant also implements PropertiesAdminUpdateCallback. Set the callback on the admin facet.
            IObject? propFacet = serviceManagerCommunicator.FindAdminFacet("IceBox.Service.TestService.Properties");
            if (propFacet is IPropertiesAdmin admin)
            {
                admin.Updated += (_, updates) => facet.Updated(updates);
            }
        }

        public void Start(string name, Communicator communicator, string[] args)
        {
        }

        public void Stop()
        {
        }
    }
}
