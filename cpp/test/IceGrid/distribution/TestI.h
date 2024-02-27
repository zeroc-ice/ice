//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public ::Test::TestIntf
{
public:
    TestI(const Ice::PropertiesPtr&);

    virtual std::string getServerFile(std::string, const Ice::Current&) override;
    virtual std::string getApplicationFile(std::string, const Ice::Current&) override;

private:
    Ice::PropertiesPtr _properties;
};

#endif
