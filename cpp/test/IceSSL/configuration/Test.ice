// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

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
