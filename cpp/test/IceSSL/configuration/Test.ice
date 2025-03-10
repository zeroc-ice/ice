// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface Server
    {
        void noCert();
        void checkCert(string subjectDN, string issuerDN);
    }

    dictionary<string, string> Properties;

    interface ServerFactory
    {
        Server* createServer(Properties props);
        void destroyServer(Server* srv);
        void shutdown();
    }
}
