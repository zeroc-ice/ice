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

ICE_API Communicator_ptr initialize(int&, char*[]);
ICE_API Communicator_ptr initializeWithProperties(int&, char*[],
						  const Properties_ptr&);

ICE_API Properties_ptr createProperties();
ICE_API Properties_ptr loadProperties(const std::string&);

}

#endif
