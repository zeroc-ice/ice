//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

// TODO: move this test to Ice/discovery

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Discovery
{
    interface TestIntf
    {
        string getAdapterId();
    }

    interface Controller
    {
        void activateObjectAdapter(string name, string adapterId, string replicaGroupId);
        void deactivateObjectAdapter(string name);

        void addObject(string oaName, string identityAndFacet);
        void removeObject(string oaName, string identityAndFacet);

        void shutdown();
    }
}
