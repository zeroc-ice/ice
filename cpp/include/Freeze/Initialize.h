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
#include <Freeze/DBF.h>

#ifdef WIN32
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API __declspec(dllexport)
#   else
#       define SLICE_API __declspec(dllimport)
#   endif
#else
#   define SLICE_API /**/
#endif

namespace Freeze
{

FREEZE_API DBFactoryPtr initialize(int&, char*[], const CommunicatorPtr&);
FREEZE_API DBFactoryPtr initializeWithProperties(const PropertiesPtr&, const CommunicatorPtr&);

}

#endif
