// **********************************************************************
//
// Copyright (c) 2001
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

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/InstanceF.h>
#include <Ice/BuiltinSequences.h>

namespace Ice
{

ICE_API CommunicatorPtr initialize(int&, char*[], Int = ICE_INT_VERSION);
ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int = ICE_INT_VERSION);

ICE_API PropertiesPtr getDefaultProperties();
ICE_API PropertiesPtr getDefaultProperties(StringSeq&);
ICE_API PropertiesPtr getDefaultProperties(int&, char*[]);

ICE_API PropertiesPtr createProperties();
ICE_API PropertiesPtr createProperties(StringSeq&);
ICE_API PropertiesPtr createProperties(int&, char*[]);

ICE_API StringSeq argsToStringSeq(int, char*[]);
//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICE_API void stringSeqToArgs(const StringSeq&, int&, char*[]);

}

namespace IceInternal
{

//
// Some Ice extensions need access to the Ice internal instance. Do
// not use this operation for regular application code! It is intended
// to be used by modules such as Freeze.
//
ICE_API InstancePtr getInstance(const ::Ice::CommunicatorPtr&);

}

#endif
