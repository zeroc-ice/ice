// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.IceSSL.configuration"]]
module Test
{

interface Server
{
    void noCert();
    void checkCert(string subjectDN, string issuerDN);
    void checkCipher(string cipher);
};

dictionary<string, string> Properties;

interface ServerFactory
{
    Server* createServer(Properties props);
    void destroyServer(Server* srv);
    void shutdown();
};

};
