// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INITIALIZE_H
#define ICEE_INITIALIZE_H

#include <IceE/CommunicatorF.h>
#include <IceE/PropertiesF.h>
#include <IceE/LoggerF.h>
#include <IceE/InstanceF.h>
#include <IceE/StringConverter.h>
#include <IceE/BuiltinSequences.h>

namespace Ice
{

ICE_API StringSeq argsToStringSeq(int, char*[]);
//
// This function assumes that the string sequence only contains
// elements of the argument vector. The function shifts the
// the argument vector elements so that the vector matches the
// contents of the sequence.
//
ICE_API void stringSeqToArgs(const StringSeq&, int&, char*[]);

ICE_API PropertiesPtr createProperties(const StringConverterPtr& = 0);
ICE_API PropertiesPtr createProperties(StringSeq&, const PropertiesPtr& = 0, const StringConverterPtr& = 0);
ICE_API PropertiesPtr createProperties(int&, char*[], const PropertiesPtr& = 0, const StringConverterPtr& = 0);

struct InitializationData
{
    PropertiesPtr properties;
    LoggerPtr logger;
    Context defaultContext;
    StringConverterPtr stringConverter;
    WstringConverterPtr wstringConverter;
};

ICE_API CommunicatorPtr initialize(int&, char*[], const InitializationData& = InitializationData(),
				   Int = ICEE_INT_VERSION);

ICE_API CommunicatorPtr initialize(const InitializationData&, Int = ICEE_INT_VERSION);

ICE_API ICE_DEPRECATED_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&,
								    Int = ICEE_INT_VERSION);

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
