//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifdef __SLICE2PY__
module Test
{
module Common
#else
module Test::Common
#endif
{
    sequence<bool> BoolSeq;
    sequence<string> StringSeq;

    class Config
    {
    #ifdef __SLICE2PY__ // Temporary workaround until python is ported to use the cpp17 mapping.
        optional(1) string protocol;
        optional(2) bool mx;
        optional(3) bool serialize;
        optional(4) bool compress;
        optional(5) bool ipv6;
        optional(6) StringSeq cprops;
        optional(7) StringSeq sprops;
    #else
        tag(1) string? protocol;
        tag(2) bool? mx;
        tag(3) bool? serialize;
        tag(4) bool? compress;
        tag(5) bool? ipv6;
        tag(6) StringSeq? cprops;
        tag(7) StringSeq? sprops;
    #endif
    }

    class OptionOverrides
    {
    #ifdef __SLICE2PY__ // Temporary workaround until python is ported to use the cpp17 mapping.
        optional(1) StringSeq protocol;
        optional(2) BoolSeq mx;
        optional(3) BoolSeq serialize;
        optional(4) BoolSeq compress;
        optional(5) BoolSeq ipv6;
    #else
        tag(1) StringSeq? protocol;
        tag(2) BoolSeq? mx;
        tag(3) BoolSeq? serialize;
        tag(4) BoolSeq? compress;
        tag(5) BoolSeq? ipv6;
    #endif
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

    #ifdef __SLICE2SWIFT__ // Temporary workaround until swift optionals are fixed.
        OptionOverrides? getOptionOverrides();
    #else
        OptionOverrides getOptionOverrides();
    #endif

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
#ifdef __SLICE2PY__
}
#endif
