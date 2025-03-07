// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    interface TestIntf
    {
        string getAdapterId();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Controller
    {
        void activateObjectAdapter(string name, string adapterId, string replicaGroupId);
        void deactivateObjectAdapter(string name);
<<<<<<< Updated upstream

        void addObject(string oaName, string id);
        void removeObject(string oaName, string id);

=======
            
        void addObject(string oaName, string id);
        void removeObject(string oaName, string id);
            
>>>>>>> Stashed changes
        void shutdown();
    }
}
