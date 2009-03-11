// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef MY_STRING_SEQ
#define MY_STRING_SEQ

#include <vector>
#include <string>
#include <iostream>

class MyStringSeq : public std::vector<std::string>
{
public:

    MyStringSeq();

    MyStringSeq(size_t n);

    MyStringSeq(size_t n, const std::string& str);

    MyStringSeq(const MyStringSeq& seq);

};

#endif
