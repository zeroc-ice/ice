// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI final : public ::Test::TestIntf
{
public:
    TestI(Ice::ObjectAdapterPtr, Ice::PropertiesPtr);

    void shutdown(const Ice::Current&) override;
    std::string getProperty(std::string, const Ice::Current&) override;

private:
    Ice::ObjectAdapterPtr _adapter;
    Ice::PropertiesPtr _properties;
};

#endif
