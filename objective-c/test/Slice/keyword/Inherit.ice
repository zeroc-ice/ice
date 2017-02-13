// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test overridding interherited names
module Test
{

struct Struct1
{
    int isa;
    int retainCount;
};

exception Exception1
{
    int isa;
    int reason;
    int raise;
    int name;
    int callStackReturnAddresses;
    int userInfo;
    int reserved;
};

class Class1
{
    int reason;

    int isa;
    int autorelease;
    int classForCoder;
    int copy;
    int dealloc;
    int description;
    int hash;
    int init;
    int isProxy;
    int mutableCopy;
    int release;
    int retain;
    int retainCount;
    int self;
    int superclass;
    int zone;
};

interface Intf1
{
    void isa();
    void reason();
    void autorelease();
    void classForCoder();
    void copy();
    void dealloc();
    void description();
    void hash();
    void init();
    void isProxy();
    void mutableCopy();
    void release();
    void retain();
    void retainCount();
    void self();
    void superclass();
    void zone();
};

interface Intf2
{
    void isa(int a);
    void reason(int a);
    void autorelease(int a);
    void classForCoder(int a);
    void copy(int a);
    void dealloc(int a);
    void description(int a);
    void hash(int a);
    void init(int a);
    void isProxy(int a);
    void mutableCopy(int a);
    void release(int a);
    void retain(int a);
    void retainCount(int a);
    void self(int a);
    void superclass(int a);
    void zone(int a);
};

};
