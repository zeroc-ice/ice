// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
