// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROPERTIES_I_H
#define ICE_PROPERTIES_I_H

#include <Ice/Properties.h>
#include <Ice/CommunicatorF.h>
#include <map>

namespace Ice
{

class ICE_API PropertiesI : public Properties
{
public:
    
    std::string getProperty(const std::string&);
    void setProperty(const std::string&, const std::string&);

private:

    PropertiesI();
    PropertiesI(const std::string&);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API PropertiesPtr createProperties();
    friend ICE_API PropertiesPtr loadProperties(const std::string&);

    void load(const std::string&);
    void parse(std::istream&);

    std::map<std::string, std::string> _properties;
};

}

#endif
