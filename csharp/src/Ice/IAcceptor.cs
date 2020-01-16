//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface IAcceptor
    {
        void close();
        Endpoint listen();
        bool startAccept(AsyncCallback callback, object state);
        void finishAccept();
        ITransceiver accept();
        string protocol();
        string ToString();
        string toDetailedString();
    }

}
