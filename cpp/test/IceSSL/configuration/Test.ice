// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    interface Server
    {
        void noCert();
        void checkCert(string subjectDN, string issuerDN);
    }
<<<<<<< Updated upstream

    dictionary<string, string> Properties;

=======
        
    dictionary<string, string> Properties;
        
>>>>>>> Stashed changes
    interface ServerFactory
    {
        Server* createServer(Properties props);
        void destroyServer(Server* srv);
        void shutdown();
    }
}
