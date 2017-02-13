// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

module Common
{

sequence<bool> BoolSeq;
sequence<string> StringSeq;

class Config
{
    optional(1) string protocol;
    optional(2) bool mx;
    optional(3) bool serialize;
    optional(4) bool compress;
    optional(5) bool ipv6;
    optional(6) StringSeq cprops;
    optional(7) StringSeq sprops;
};

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
    TestCase* runTestCase(string mapping, string testsuite, string testcase, string cross)
        throws TestCaseNotExistException;

    OptionOverrides getOptionOverrides();

    StringSeq getTestSuites(string mapping);
};

exception ProcessFailedException
{
    string reason;
};

interface Process
{
    void waitReady(int timeout)
        throws ProcessFailedException;

    int waitSuccess(int timeout);

    string terminate();
};

interface ProcessController
{
    Process* start(string testsuite, string exe, StringSeq args)
        throws ProcessFailedException;

    string getHost(string protocol, bool ipv6);
};

interface ProcessControllerRegistry
{
    void setProcessController(ProcessController* controller);
};

};

};
