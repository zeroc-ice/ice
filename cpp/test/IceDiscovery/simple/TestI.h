// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI : public Test::TestIntf
{
public:
    std::string getAdapterId(const Ice::Current&) override;
};

class ControllerI : public Test::Controller
{
public:
    void activateObjectAdapter(std::string, std::string, std::string, const Ice::Current&) override;
    void deactivateObjectAdapter(std::string, const Ice::Current&) override;

    void addObject(std::string, std::string, const Ice::Current&) override;
    void removeObject(std::string, std::string, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

private:
    std::map<std::string, Ice::ObjectAdapterPtr> _adapters;
};

#endif
