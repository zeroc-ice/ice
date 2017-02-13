// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

import test.Ice.admin.Test.*;

public class RemoteCommunicatorFactoryI extends _RemoteCommunicatorFactoryDisp
{
    @Override
    public RemoteCommunicatorPrx createCommunicator(java.util.Map<String, String> props, Ice.Current current)
    {
        //
        // Prepare the property set using the given properties.
        //
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.classLoader = IceInternal.Util.getInstance(current.adapter.getCommunicator()).getClassLoader();
        initData.properties = Ice.Util.createProperties();
        for(java.util.Map.Entry<String, String> e : props.entrySet())
        {
            initData.properties.setProperty(e.getKey(), e.getValue());
        }

        if(initData.properties.getPropertyAsInt("NullLogger") > 0)
        {
            initData.logger = new Ice.Logger() {
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
                    
                    @Override public Ice.Logger cloneWithPrefix(String prefix)
                    {
                        return this;
                    }
                };
        }

        //
        // Initialize a new communicator.
        //
        Ice.Communicator communicator = Ice.Util.initialize(initData);

        //
        // Install a custom admin facet.
        //
        communicator.addAdminFacet(new TestFacetI(), "TestFacet");

        //
        // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        RemoteCommunicatorI servant = new RemoteCommunicatorI(communicator);
        Ice.Object propFacet = communicator.findAdminFacet("Properties");

        if(propFacet != null)
        {
            Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
            assert admin != null;
            admin.addUpdateCallback(servant);
        }

        Ice.ObjectPrx proxy = current.adapter.addWithUUID(servant);
        return RemoteCommunicatorPrxHelper.uncheckedCast(proxy);
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
