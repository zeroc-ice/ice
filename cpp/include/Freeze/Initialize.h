// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_INITIALIZE_H
#define FREEZE_INITIALIZE_H

#include <Ice/Ice.h>
#include <Freeze/DBF.h>

#ifdef WIN32
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

}

#endif
