//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface IAcceptor
    {
        void Close();
        Endpoint Listen();
        bool StartAccept(AsyncCallback callback, object state);
        void FinishAccept();
        ITransceiver Accept();
        string Protocol();
        string ToString();
        string ToDetailedString();
    }

}
