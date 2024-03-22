//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI final : public ::Test::TestIntf
{
public:
    TestI(const Ice::PropertiesPtr&);

    std::string getServerFile(std::string, const Ice::Current&) final;
    std::string getApplicationFile(std::string, const Ice::Current&) final;

private:
    Ice::PropertiesPtr _properties;
};

#endif
