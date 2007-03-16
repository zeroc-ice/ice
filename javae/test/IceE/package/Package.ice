// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

[["java:package:testpkg"]]

module Test2
{
class C1
{
    int i;
};

class C2 extends C1
{
    long l;
};

exception E1
{
    int i;
};

exception E2 extends E1
{
    long l;
};
};

module Test3
{
class C1
{
    int i;
};

class C2 extends C1
{
    long l;
};

exception E1
{
    int i;
};

exception E2 extends E1
{
    long l;
};
};
