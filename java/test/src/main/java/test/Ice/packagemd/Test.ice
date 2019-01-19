//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Package.ice>
#include <NoPackage.ice>

[["java:package:test.Ice.packagemd"]]
module Test
{

interface Initial
{
    Test1::C1 getTest1C2AsC1();
    Test1::C2 getTest1C2AsC2();
    void throwTest1E2AsE1() throws Test1::E1;
    void throwTest1E2AsE2() throws Test1::E2;
    void throwTest1Notify() throws Test1::notify;

    Test2::C1 getTest2C2AsC1();
    Test2::C2 getTest2C2AsC2();
    void throwTest2E2AsE1() throws Test2::E1;
    void throwTest2E2AsE2() throws Test2::E2;

    Test3::C1 getTest3C2AsC1();
    Test3::C2 getTest3C2AsC2();
    void throwTest3E2AsE1() throws Test3::E1;
    void throwTest3E2AsE2() throws Test3::E2;

    void shutdown();
}

}
