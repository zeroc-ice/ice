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
ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int = ICE_INT_VERSION);

ICE_API PropertiesPtr getDefaultProperties();
ICE_API PropertiesPtr createProperties();
ICE_API PropertiesPtr loadProperties(const std::string&);

}

#endif
