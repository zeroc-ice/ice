// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INITIALIZE_H
#define ICE_INITIALIZE_H

namespace Ice
{

ICE_API CommunicatorPtr initialize(int&, char*[], Int = ICE_INT_VERSION);
ICE_API CommunicatorPtr initializeWithProperties(const PropertiesPtr&, Int = ICE_INT_VERSION);

ICE_API PropertiesPtr getDefaultProperties(int&, char*[]);
ICE_API PropertiesPtr createProperties(int&, char*[]);
ICE_API PropertiesPtr createPropertiesFromFile(int&, char*[], const std::string&);

}

#endif
