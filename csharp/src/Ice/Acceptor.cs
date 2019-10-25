//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface Acceptor
    {
        void close();
        EndpointI listen();
        bool startAccept(AsyncCallback callback, object state);
        void finishAccept();
        Transceiver accept();
        string protocol();
        string ToString();
        string toDetailedString();
    }

}
