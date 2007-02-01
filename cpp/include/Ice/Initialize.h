// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerF.h>
#include <Ice/StreamF.h>
#include <Ice/StatsF.h>
#include <Ice/StringConverter.h>
#include <Ice/BuiltinSequences.h>

namespace Ice
{

ICE_API void collectGarbage();

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

//
// This class is used to notify user of when Ice threads
// are started and stopped.
//
class ICE_API ThreadNotification : public IceUtil::Shared
{
public:

    virtual void start() = 0;
    virtual void stop() = 0;
};

typedef IceUtil::Handle<ThreadNotification> ThreadNotificationPtr;

//
// Communicator initialization info
//
struct InitializationData
{
    PropertiesPtr properties;
    LoggerPtr logger;
    StatsPtr stats;
    StringConverterPtr stringConverter;
    WstringConverterPtr wstringConverter;
    ThreadNotificationPtr threadHook;
};

ICE_API CommunicatorPtr initialize(int&, char*[], const InitializationData& = InitializationData(),
                                   Int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(Ice::StringSeq&, const InitializationData& = InitializationData(),
                                   Int = ICE_INT_VERSION);

ICE_API CommunicatorPtr initialize(const InitializationData& = InitializationData(), 
                                   Int = ICE_INT_VERSION);


ICE_API ICE_DEPRECATED_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&,
                                                                    Int = ICE_INT_VERSION);
ICE_API ICE_DEPRECATED_API CommunicatorPtr initializeWithLogger(int&, char*[], const Ice::LoggerPtr&,
                                                                Int = ICE_INT_VERSION);
ICE_API ICE_DEPRECATED_API CommunicatorPtr initializeWithPropertiesAndLogger(int&, char*[], const PropertiesPtr&,
                                                                             const Ice::LoggerPtr&, 
                                                                             Int = ICE_INT_VERSION);

ICE_API InputStreamPtr createInputStream(const CommunicatorPtr&, const ::std::vector< Byte >&);
ICE_API OutputStreamPtr createOutputStream(const CommunicatorPtr&);

ICE_API LoggerPtr getProcessLogger();
ICE_API void setProcessLogger(const LoggerPtr&);

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
