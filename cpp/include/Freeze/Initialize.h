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

#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
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

FREEZE_API DBEnvPtr initialize(const ::Ice::CommunicatorPtr&);
FREEZE_API DBEnvPtr initializeWithProperties(const ::Ice::CommunicatorPtr&, const ::Ice::PropertiesPtr&);

}

#endif
