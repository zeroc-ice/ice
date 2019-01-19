//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
