//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceDiscovery;

public class PluginFactory implements Ice.PluginFactory
{
    @Override
    public Ice.Plugin
    create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
