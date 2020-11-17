//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::IceSSL::Test::Configuration
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
    Server createServer(Properties properties, bool requireClientCertificate);
    void destroyServer(Server srv);
    void shutdown();
}

}
