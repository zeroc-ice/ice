//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.IceSSL.configuration"]]
module Test
{

interface Server
{
    void noCert();
    void checkCert(string subjectDN, string issuerDN);
    void checkCipher(string cipher);
}

dictionary<string, string> Properties;

interface ServerFactory
{
    Server* createServer(Properties props);
    void destroyServer(Server* srv);
    void shutdown();
}

}
