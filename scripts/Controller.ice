// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

module Common
{

exception ServerFailedException
{
    string reason;
};

interface Server
{
    void waitTestSuccess();
    void waitForServer() throws ServerFailedException;
    void terminate();
};

class Config
{
    optional(1) string protocol;
    optional(2) bool mx;
    optional(3) bool serialize;
    optional(4) bool compress;
    optional(5) bool ipv6;
};

sequence<bool> BoolSeq;
sequence<string> StringSeq;

class OptionOverrides
{
    optional(1) StringSeq protocol;
    optional(2) BoolSeq mx;
    optional(3) BoolSeq serialize;
    optional(4) BoolSeq compress;
    optional(5) BoolSeq ipv6;
};

exception TestCaseNotExistException
{
    string reason;
};

exception TestCaseFailedException
{
    string output;
};

interface TestCase
{
    string startServerSide(Config config)
        throws TestCaseFailedException;

    string stopServerSide(bool success)
        throws TestCaseFailedException;

    string runClientSide(string host, Config config)
        throws TestCaseFailedException;

    void destroy();
};

interface Controller
{
    Server* runServer(string lang, string testsuite, string protocol, string host, bool winrt, string testcase,
                      StringSeq options);

    TestCase* runTestCase(string mapping, string testsuite, string testcase, string cross)
        throws TestCaseNotExistException;

    OptionOverrides getOptionOverrides();
};

};

};
