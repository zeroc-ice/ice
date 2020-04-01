//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    virtual std::string getProperty(const std::string&) noexcept;
    virtual std::string getPropertyWithDefault(const std::string&, const std::string&) noexcept;
    virtual Ice::Int getPropertyAsInt(const std::string&) noexcept;
    virtual Ice::Int getPropertyAsIntWithDefault(const std::string&, Ice::Int) noexcept;
    virtual Ice::StringSeq getPropertyAsList(const std::string&) noexcept;
    virtual Ice::StringSeq getPropertyAsListWithDefault(const std::string&, const Ice::StringSeq&) noexcept;

    virtual PropertyDict getPropertiesForPrefix(const std::string&) noexcept;
    virtual void setProperty(const std::string&, const std::string&);
    virtual StringSeq getCommandLineOptions() noexcept;
    virtual StringSeq parseCommandLineOptions(const std::string&, const StringSeq&);
    virtual StringSeq parseIceCommandLineOptions(const StringSeq&);
    virtual void load(const std::string&);
    virtual PropertiesPtr clone() noexcept;

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
