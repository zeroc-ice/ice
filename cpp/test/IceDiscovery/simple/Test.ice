//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

interface TestIntf
{
    string getAdapterId();
}

interface Controller
{
    void activateObjectAdapter(string name, string adapterId, string replicaGroupId);
    void deactivateObjectAdapter(string name);

    void addObject(string oaName, string id);
    void removeObject(string oaName, string id);

    void shutdown();
}

}
