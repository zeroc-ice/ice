// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBT;

final class EndpointFactoryI implements IceInternal.EndpointFactory
{
    EndpointFactoryI(Instance instance)
    {
        _instance = instance;
    }

    @Override
    public void initialize()
    {
        // Nothing to do.
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        EndpointI endpt = new EndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public IceInternal.EndpointI read(Ice.InputStream s)
    {
        return new EndpointI(_instance, s);
    }

    @Override
    public void destroy()
    {
        _instance.destroy();
        _instance = null;
    }

    @Override
    public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance inst)
    {
        return new EndpointFactoryI(new Instance(_instance.communicator(), inst.type(), inst.protocol()));
    }

    private Instance _instance;
}
