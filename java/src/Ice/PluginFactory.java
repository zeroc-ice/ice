// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public interface PluginFactory
{
    Plugin create(Communicator communicator, String name, String[] args);
}
