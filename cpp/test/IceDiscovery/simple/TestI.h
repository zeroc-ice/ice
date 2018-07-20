// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : public Test::TestIntf
{
public:

    virtual std::string getAdapterId(const Ice::Current&);
};

class ControllerI : public Test::Controller
{
public:

    virtual void activateObjectAdapter(ICE_IN(std::string), ICE_IN(std::string), ICE_IN(std::string), const Ice::Current&);
    virtual void deactivateObjectAdapter(ICE_IN(std::string), const Ice::Current&);

    virtual void addObject(ICE_IN(std::string), ICE_IN(std::string), const Ice::Current&);
    virtual void removeObject(ICE_IN(std::string), ICE_IN(std::string), const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

private:

    std::map<std::string, Ice::ObjectAdapterPtr> _adapters;
};

#endif
