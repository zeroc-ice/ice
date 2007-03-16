// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Package.ice>
#include <NoPackage.ice>

module Test
{
interface Initial
{
    void throwTest1E2AsE1() throws Test1::E1;
    void throwTest1E2AsE2() throws Test1::E2;
    void throwTest1Notify() throws Test1::notify;

    void throwTest2E2AsE1() throws Test2::E1;
    void throwTest2E2AsE2() throws Test2::E2;

    void throwTest3E2AsE1() throws Test3::E1;
    void throwTest3E2AsE2() throws Test3::E2;

    void shutdown();
};
};
