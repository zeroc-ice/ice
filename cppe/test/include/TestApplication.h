// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#include <IceE/Config.h>
#include <IceE/IceE.h>
#include <IceE/Mutex.h>

//
// XXX: For ICEE_TEST_COMMON_API. We could create a TestConfig.h later, or
// perhaps make Test/... subdirectory.
//
#include <TestCommon.h>

class ICEE_TEST_COMMON_API TestApplication
{
public:

    TestApplication(const std::string& = "");
#ifdef _WIN32_WCE
    int main(HINSTANCE);
#else
    int main(int, char*[]);
#endif

    virtual int run(int, char*[]) = 0;

    void setCommunicator(const Ice::CommunicatorPtr&);
    Ice::CommunicatorPtr communicator();

    bool terminated() const;

private:

    const std::string _name;
    Ice::CommunicatorPtr _communicator;
};

#endif
