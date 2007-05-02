// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROPERTIES_H
#define ICEE_PROPERTIES_H

#include <IceE/PropertiesF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/BuiltinSequences.h>

namespace Ice
{

typedef ::std::map< ::std::string, ::std::string> PropertyDict;

class ICE_API Properties : public IceUtil::Mutex, public ::IceUtil::Shared
{
public:
    
    std::string getProperty(const std::string&);
    std::string getPropertyWithDefault(const std::string&, const std::string&);
    Ice::Int getPropertyAsInt(const std::string&);
    Ice::Int getPropertyAsIntWithDefault(const std::string&, Ice::Int);
    PropertyDict getPropertiesForPrefix(const std::string&);
    void setProperty(const std::string&, const std::string&);
    StringSeq getCommandLineOptions();
    StringSeq parseCommandLineOptions(const std::string&, const StringSeq&);
    StringSeq parseIceCommandLineOptions(const StringSeq&);
    void load(const std::string&);
    PropertiesPtr clone();

private:

    Properties(const Properties*);
    Properties(
#ifdef ICEE_HAS_WSTRING
               const StringConverterPtr&
#endif
              );
    Properties(StringSeq&, const PropertiesPtr&
#ifdef ICEE_HAS_WSTRING
               , const StringConverterPtr&
#endif
              );

    friend ICE_API PropertiesPtr createProperties(
#ifdef ICEE_HAS_WSTRING
                                                  const StringConverterPtr&
#endif
                                                 );
    friend ICE_API PropertiesPtr createProperties(StringSeq&, const PropertiesPtr&
#ifdef ICEE_HAS_WSTRING
                                                  , const StringConverterPtr&
#endif
                                                 );

    void parseLine(const std::string&
#ifdef ICEE_HAS_WSTRING
                   , const StringConverterPtr&
#endif
                  );

    void loadConfig();

    std::map<std::string, std::string> _properties;
#ifdef ICEE_HAS_WSTRING
    const StringConverterPtr _converter;
#endif
};

}

#endif
