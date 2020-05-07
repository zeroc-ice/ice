//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

namespace IceInternal
{
    public interface IAcceptor
    {
        void Close();
        Endpoint Listen();
        bool StartAccept(AsyncCallback callback, object state);
        void FinishAccept();
        ITransceiver Accept();
        string Transport();
        string ToString();
        string ToDetailedString();
    }
}
