// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROPERTIES_H
#define ICE_PROPERTIES_H

#include <Ice/PropertiesF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API Properties : public ::__Ice::Shared
{
public:
    
    std::string getProperty(const std::string&);
    void setProperty(const std::string&, const std::string&);

private:

    Properties();
    Properties(const std::string&);
    Properties(const Properties_ptr&);

    friend ICE_API Communicator_ptr initialize(int&, char*[]);
    friend ICE_API Properties_ptr createProperties();
    friend ICE_API Properties_ptr createProperties(const Properties_ptr&);
    friend ICE_API Properties_ptr createProperties(const std::string&);

    void load(const std::string&);
    void parse(std::istream&);

    std::map<std::string, std::string> properties_;
};

}

#endif
