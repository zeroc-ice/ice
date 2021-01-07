//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

exception TestException
{
}

interface TestIntf
{
    int op();
    int callOpOn(TestIntf* proxy);
    void throwUserException1() throws TestException;
    void throwUserException2() throws TestException;
    void throwUnhandledException1();
    void throwUnhandledException2();
    void sleep(int ms);
    void shutdown();
}

}
