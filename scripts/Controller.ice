//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

module Common
{

sequence<bool> BoolSeq;
sequence<string> StringSeq;

class Config
{
    tag(1) string protocol;
    tag(2) bool mx;
    tag(3) bool serialize;
    tag(4) bool compress;
    tag(5) bool ipv6;
    tag(6) StringSeq cprops;
    tag(7) StringSeq sprops;
}

class OptionOverrides
{
    tag(1) StringSeq protocol;
    tag(2) BoolSeq mx;
    tag(3) BoolSeq serialize;
    tag(4) BoolSeq compress;
    tag(5) BoolSeq ipv6;
}

exception TestCaseNotExistException
{
    string reason;
}

exception TestCaseFailedException
{
    string output;
}

interface TestCase
{
    string startServerSide(Config config)
        throws TestCaseFailedException;

    string stopServerSide(bool success)
        throws TestCaseFailedException;

    string runClientSide(string host, Config config)
        throws TestCaseFailedException;

    void destroy();
}

interface Controller
{
    TestCase* runTestCase(string mapping, string testsuite, string testcase, string cross)
        throws TestCaseNotExistException;

    OptionOverrides getOptionOverrides();

    StringSeq getTestSuites(string mapping);

    string getHost(string protocol, bool ipv6);
}

exception ProcessFailedException
{
    string reason;
}

interface Process
{
    void waitReady(int timeout)
        throws ProcessFailedException;

    int waitSuccess(int timeout)
        throws ProcessFailedException;

    string terminate();
}

interface ProcessController
{
    Process* start(string testsuite, string exe, StringSeq args)
        throws ProcessFailedException;

    string getHost(string protocol, bool ipv6);
}

interface BrowserProcessController extends ProcessController
{
    void redirect(string url);
}

interface ProcessControllerRegistry
{
    void setProcessController(ProcessController* controller);
}

}

}
