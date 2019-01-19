//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public ::Test::TestIntf
{
public:

    TestI(const Ice::StringSeq&);

    virtual std::string getProperty(ICE_IN(std::string), const Ice::Current&);
    virtual Ice::StringSeq getArgs(const Ice::Current&);

private:

    const Ice::StringSeq _args;
};

#endif
