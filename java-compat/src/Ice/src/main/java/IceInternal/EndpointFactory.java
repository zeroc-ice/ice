//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface EndpointFactory
{
    void initialize();
    short type();
    String protocol();
    EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint);
    EndpointI read(Ice.InputStream s);
    void destroy();

    EndpointFactory clone(ProtocolInstance instance);
}
