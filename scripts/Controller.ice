// Copyright (c) ZeroC, Inc.

module Test::Common
{
    sequence<bool> BoolSeq;
    sequence<string> StringSeq;

    class Config
    {
        ["swift:identifier:transportProtocol"]
        optional(1) string protocol;
        optional(2) bool mx;
        optional(3) bool serialize;
        optional(4) bool compress;
        optional(5) bool ipv6;
        optional(6) StringSeq cprops;
        optional(7) StringSeq sprops;
    }

    class OptionOverrides
    {
        ["swift:identifier:transportProtocol"]
        optional(1) StringSeq protocol;
        optional(2) BoolSeq mx;
        optional(3) BoolSeq serialize;
        optional(4) BoolSeq compress;
        optional(5) BoolSeq ipv6;
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

        string getHost(["swift:identifier:transportProtocol"] string protocol, bool ipv6);
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

        string getHost(["swift:identifier:transportProtocol"] string protocol, bool ipv6);
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
