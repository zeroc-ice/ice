// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;

    public interface EndpointFactory
    {
        short type();
        string protocol();
        EndpointI create(List<string> args, bool oaEndpoint);
        EndpointI read(BasicStream s);
        void destroy();

        EndpointFactory clone(ProtocolInstance instance);
    }

}
