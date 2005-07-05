// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INITIALIZE_H
#define ICEE_INITIALIZE_H

#include <IceE/CommunicatorF.h>
#include <IceE/PropertiesF.h>
#include <IceE/InstanceF.h>
#include <IceE/BuiltinSequences.h>

namespace IceE
{

ICEE_API void collectGarbage();

ICEE_API StringSeq argsToStringSeq(int, char*[]);
//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICEE_API void stringSeqToArgs(const StringSeq&, int&, char*[]);

ICEE_API PropertiesPtr createProperties();
ICEE_API PropertiesPtr createProperties(StringSeq&);
ICEE_API PropertiesPtr createProperties(int&, char*[]);

ICEE_API CommunicatorPtr initialize(int&, char*[], Int = ICEE_INT_VERSION);
ICEE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int = ICEE_INT_VERSION);

ICEE_API PropertiesPtr getDefaultProperties();
ICEE_API PropertiesPtr getDefaultProperties(StringSeq&);
ICEE_API PropertiesPtr getDefaultProperties(int&, char*[]);

}

namespace IceEInternal
{

//
// Some Ice extensions need access to the Ice internal instance. Do
// not use this operation for regular application code! It is intended
// to be used by modules such as Freeze.
//
ICEE_API InstancePtr getInstance(const ::IceE::CommunicatorPtr&);

}

#endif
