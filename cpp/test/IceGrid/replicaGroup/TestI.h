// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI : public ::Test::TestIntf
{
public:
    TestI(Ice::PropertiesPtr);

    std::string getReplicaId(const Ice::Current&) override;
    std::string getReplicaIdAndShutdown(const Ice::Current&) override;

private:
    Ice::PropertiesPtr _properties;
};

#endif
