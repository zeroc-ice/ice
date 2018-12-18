// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

import test.Ice.admin.Test.*;

public class RemoteCommunicatorFactoryI implements RemoteCommunicatorFactory
{
    @Override
    public RemoteCommunicatorPrx createCommunicator(java.util.Map<String, String> props, com.zeroc.Ice.Current current)
    {
        //
        // Prepare the property set using the given properties.
        //
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.classLoader =
            com.zeroc.IceInternal.Util.getInstance(current.adapter.getCommunicator()).getClassLoader();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        for(java.util.Map.Entry<String, String> e : props.entrySet())
        {
            initData.properties.setProperty(e.getKey(), e.getValue());
        }

        if(initData.properties.getPropertyAsInt("NullLogger") > 0)
        {
            initData.logger = new com.zeroc.Ice.Logger() {
                    @Override public void print(String message)
                    {
                    }

                    @Override public void trace(String category, String message)
                    {
                    }

                    @Override public void warning(String message)
                    {
                    }

                    @Override public void error(String message)
                    {
                    }

                    @Override public String getPrefix()
                    {
                        return "NullLogger";
                    }

                    @Override public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
                    {
                        return this;
                    }
                };
        }

        //
        // Initialize a new communicator.
        //
        com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(initData);

        //
        // Install a custom admin facet.
        //
        communicator.addAdminFacet(new TestFacetI(), "TestFacet");

        //
        // The RemoteCommunicator servant also implements java.util.function.Consumer<java.util.Map<String, String>>.
        // Set the callback on the admin facet.
        //
        RemoteCommunicatorI servant = new RemoteCommunicatorI(communicator);
        com.zeroc.Ice.Object propFacet = communicator.findAdminFacet("Properties");

        if(propFacet != null)
        {
            com.zeroc.Ice.NativePropertiesAdmin admin = (com.zeroc.Ice.NativePropertiesAdmin)propFacet;
            assert admin != null;
            admin.addUpdateCallback(servant);
        }

        com.zeroc.Ice.ObjectPrx proxy = current.adapter.addWithUUID(servant);
        return RemoteCommunicatorPrx.uncheckedCast(proxy);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
