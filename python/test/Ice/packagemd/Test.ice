// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Package.ice>
#include <NoPackage.ice>

module Test
{

interface Initial
{
    Test1::C1 getTest1C2AsC1();
    Test1::C2 getTest1C2AsC2();
    void throwTest1E2AsE1() throws Test1::E1;
    void throwTest1E2AsE2() throws Test1::E2;
    void throwTest1Def() throws Test1::def;

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
