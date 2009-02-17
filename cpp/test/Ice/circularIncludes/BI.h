// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef B_I_H
#define B_I_H

#include <B.h>

class BI : public Test::B
{
public:

    virtual void shutdown(const Ice::Current&);
};

#endif
