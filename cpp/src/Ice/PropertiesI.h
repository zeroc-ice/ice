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

namespace Ice
{

class ICE_API PropertiesI : public Properties
{
public:
    
    virtual std::string getProperty(const std::string&);
    virtual void setProperty(const std::string&, const std::string&);
    virtual PropertiesPtr clone();

private:

    PropertiesI(int&, char*[]);
    PropertiesI(int&, char*[], const std::string& file);
    friend ICE_API PropertiesPtr createProperties(int&, char*[]);
    friend ICE_API PropertiesPtr createPropertiesFromFile(int&, char*[], const std::string&);

    void parseArgs(int&, char*[]);
    void load(const std::string&);
    void parse(std::istream&);
    void parseLine(const std::string&);

    std::map<std::string, std::string> _properties;
};

}

#endif
