// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <MyStringSeq.h>

#include <iostream>

using namespace std;

MyStringSeq::MyStringSeq()
{
}

MyStringSeq::MyStringSeq(size_t n) : 
    vector<string>(n)
{
}

MyStringSeq::MyStringSeq(size_t n, const string& str) : 
    vector<string>(n, str)
{
}

MyStringSeq::MyStringSeq(const MyStringSeq& seq) : 
    vector<string>(seq)
{
    cout << "MyStringSeq copy ctor" << endl;
}
