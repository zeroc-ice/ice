// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointFactoryI extends IceInternal.EndpointFactoryWithUnderlying
{
    EndpointFactoryI(Instance instance, short type)
    {
        super(instance, type);
        _instance = instance;
    }

    @Override
    public IceInternal.EndpointFactory cloneWithUnderlying(IceInternal.ProtocolInstance instance,
                                                           short underlying)
    {
        return new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);
    }

    @Override
    public IceInternal.EndpointI createWithUnderlying(IceInternal.EndpointI underlying,
                                                      java.util.ArrayList<String> args,
                                                      boolean oaEndpoint)
    {
        return new EndpointI(_instance, underlying);
    }

    @Override
    public IceInternal.EndpointI readWithUnderlying(IceInternal.EndpointI underlying, Ice.InputStream s)
    {
        return new EndpointI(_instance, underlying);
    }

    private Instance _instance;
}
