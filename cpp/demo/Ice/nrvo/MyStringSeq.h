// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef MY_STRING_SEQ
#define MY_STRING_SEQ

#include <IceUtil/Config.h>
#include <vector>
#include <string>

class MyStringSeq : public std::vector<std::string>
{
public:

    MyStringSeq();

    MyStringSeq(size_t);

    MyStringSeq(size_t, const std::string&);

    MyStringSeq(const MyStringSeq&);

};

#endif
