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

#ifndef FREEZE_INITIALIZE_H
#define FREEZE_INITIALIZE_H

#include <Ice/Ice.h>
#include <Freeze/DBF.h>

#ifdef _WIN32
#   ifdef FREEZE_API_EXPORTS
#       define FREEZE_API __declspec(dllexport)
#   else
#       define FREEZE_API __declspec(dllimport)
#   endif
#else
#   define FREEZE_API /**/
#endif

namespace Freeze
{

FREEZE_API DBEnvironmentPtr initialize(const ::Ice::CommunicatorPtr&, const std::string&);
FREEZE_API DBEnvironmentPtr initializeWithTxn(const ::Ice::CommunicatorPtr&, const std::string&);

}

#endif
