// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_APPLICATION_H
#define FREEZE_APPLICATION_H

#ifdef _WIN32
#   ifdef FREEZE_API_EXPORTS
#       define FREEZE_API __declspec(dllexport)
#   else
#       define FREEZE_API __declspec(dllimport)
#   endif
#else
#   define FREEZE_API /**/
#endif

#include <Ice/Application.h>
#include <Freeze/Freeze.h>

namespace Freeze
{

class FREEZE_API Application : public Ice::Application
{
public:

    Application(const std::string&);
    virtual ~Application();

    virtual int runFreeze(int, char*[], const DBEnvironmentPtr&) = 0;

private:

    virtual int run(int, char*[]);

    const std::string _dbEnvName;
};

};

#endif
