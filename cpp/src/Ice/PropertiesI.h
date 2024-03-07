//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROPERTIES_I_H
#define ICE_PROPERTIES_I_H

#include "Ice/Properties.h"
#include "Ice/StringConverter.h"

#include <mutex>
#include <set>

namespace Ice
{

class PropertiesI final : public Properties
{
public:

    std::string getProperty(std::string_view) noexcept final;
    std::string getPropertyWithDefault(std::string_view, std::string_view) noexcept final;
    std::int32_t getPropertyAsInt(std::string_view) noexcept final;
    std::int32_t getPropertyAsIntWithDefault(std::string_view, std::int32_t) noexcept final;
    Ice::StringSeq getPropertyAsList(std::string_view) noexcept final;
    Ice::StringSeq getPropertyAsListWithDefault(std::string_view, const Ice::StringSeq&) noexcept final;

    PropertyDict getPropertiesForPrefix(std::string_view) noexcept final;
    void setProperty(std::string_view, std::string_view) final;
    StringSeq getCommandLineOptions() noexcept final;
    StringSeq parseCommandLineOptions(std::string_view, const StringSeq&) final;
    StringSeq parseIceCommandLineOptions(const StringSeq&) final;
    void load(std::string_view) final;
    PropertiesPtr clone() noexcept final;

    std::set<std::string> getUnusedProperties();

    PropertiesI(const PropertiesI*);

    PropertiesI();
    PropertiesI(StringSeq&, const PropertiesPtr&);

private:

    void parseLine(std::string_view, const StringConverterPtr&);

    void loadConfig();

    struct PropertyValue
    {
        PropertyValue() :
            used(false)
        {
        }

        PropertyValue(std::string_view v, bool u) :
            value(std::string{v}),
            used(u)
        {
        }

        std::string value;
        bool used;
    };
    std::map<std::string, PropertyValue, std::less<>> _properties;
    std::mutex _mutex;
};

}

#endif
