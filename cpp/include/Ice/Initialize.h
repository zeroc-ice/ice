// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
ICE_API PropertiesPtr getDefaultProperties(int&, char*[]); // getDefaultPropertiesWithArgs?
ICE_API PropertiesPtr createProperties();
ICE_API PropertiesPtr createProperties(int&, char*[]); // createPropertiesWithArgs?

ICE_API StringSeq argsToStringSeq(int, char*[]);
//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICE_API void stringSeqToArgs(const StringSeq&, int&, char*[]);

#if 0

ICE_API CommunicatorPtr initialize(int&, char*[], Int = ICE_INT_VERSION);
ICE_API CommunicatorPtr initializeWithProperties(const PropertiesPtr&, Int = ICE_INT_VERSION);

ICE_API PropertiesPtr getDefaultProperties(int&, char*[]);
ICE_API PropertiesPtr createProperties(int&, char*[]);
ICE_API PropertiesPtr createPropertiesFromFile(int&, char*[], const std::string&);

#endif

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
