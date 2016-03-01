// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROPERTIES_I_H
#define ICE_PROPERTIES_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Properties.h>
#include <IceUtil/StringConverter.h>

#include <set>

namespace Ice
{

class PropertiesI : public Properties, public IceUtil::Mutex
{
public:
    
    virtual std::string getProperty(const std::string&);
    virtual std::string getPropertyWithDefault(const std::string&, const std::string&);
    virtual Ice::Int getPropertyAsInt(const std::string&);
    virtual Ice::Int getPropertyAsIntWithDefault(const std::string&, Ice::Int);
    virtual Ice::StringSeq getPropertyAsList(const std::string&);
    virtual Ice::StringSeq getPropertyAsListWithDefault(const std::string&, const Ice::StringSeq&);

    virtual PropertyDict getPropertiesForPrefix(const std::string&);
    virtual void setProperty(const std::string&, const std::string&);
    virtual StringSeq getCommandLineOptions();
    virtual StringSeq parseCommandLineOptions(const std::string&, const StringSeq&);
    virtual StringSeq parseIceCommandLineOptions(const StringSeq&);
    virtual void load(const std::string&);
    virtual PropertiesPtr clone();

    std::set<std::string> getUnusedProperties();

private:
    PropertiesI(const IceUtil::StringConverterPtr&);
    PropertiesI(StringSeq&, const PropertiesPtr&, const IceUtil::StringConverterPtr&);
    PropertiesI(const PropertiesI*);

    friend ICE_API PropertiesPtr createProperties();
    friend ICE_API PropertiesPtr createProperties(StringSeq&, const PropertiesPtr&);
    friend ICE_API PropertiesPtr createProperties(int&, char*[], const PropertiesPtr&);

    void parseLine(const std::string&, const IceUtil::StringConverterPtr&);

    void loadConfig();

    struct PropertyValue
    {
        PropertyValue() :
            used(false)
        {
        }

        PropertyValue(const std::string& v, bool u) :
            value(v),
            used(u)
        {
        }

        std::string value;
        bool used;
    };
    std::map<std::string, PropertyValue> _properties;
    const IceUtil::StringConverterPtr _converter;
};

}

#endif
