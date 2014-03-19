// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Demo
{

struct Struct1
{
    long l;
};

struct Struct2
{
    string s;
    Struct1 s1;
};

class Class1
{
    string s;
};

class Class2 extends Class1
{
    Object obj;
    Object rec;
};

};
