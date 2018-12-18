// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROPERTIES_I_H
#define ICE_PROPERTIES_I_H

#include <IceUtil/Mutex.h>
#include <Ice/Properties.h>
#include <Ice/StringConverter.h>

#include <set>

namespace Ice
{

class PropertiesI : public Properties, public IceUtil::Mutex
{
public:

    virtual std::string getProperty(const std::string&) ICE_NOEXCEPT;
    virtual std::string getPropertyWithDefault(const std::string&, const std::string&) ICE_NOEXCEPT;
    virtual Ice::Int getPropertyAsInt(const std::string&) ICE_NOEXCEPT;
    virtual Ice::Int getPropertyAsIntWithDefault(const std::string&, Ice::Int) ICE_NOEXCEPT;
    virtual Ice::StringSeq getPropertyAsList(const std::string&) ICE_NOEXCEPT;
    virtual Ice::StringSeq getPropertyAsListWithDefault(const std::string&, const Ice::StringSeq&) ICE_NOEXCEPT;

    virtual PropertyDict getPropertiesForPrefix(const std::string&) ICE_NOEXCEPT;
    virtual void setProperty(const std::string&, const std::string&);
    virtual StringSeq getCommandLineOptions() ICE_NOEXCEPT;
    virtual StringSeq parseCommandLineOptions(const std::string&, const StringSeq&);
    virtual StringSeq parseIceCommandLineOptions(const StringSeq&);
    virtual void load(const std::string&);
    virtual PropertiesPtr clone() ICE_NOEXCEPT;

    std::set<std::string> getUnusedProperties();

    PropertiesI(const PropertiesI*);

    PropertiesI();
    PropertiesI(StringSeq&, const PropertiesPtr&);

private:

    void parseLine(const std::string&, const StringConverterPtr&);

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
};

}

#endif
