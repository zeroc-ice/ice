//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice
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

        // TODO: temporary hack, it will be removed with the transport refactoring
        Task<ITransceiver> AcceptAsync()
        {
            var result = new TaskCompletionSource<ITransceiver>();
            if (StartAccept(state =>
            {
                try
                {
                    var acceptor = (IAcceptor)state;
                    acceptor.FinishAccept();
                    result.SetResult(acceptor.Accept());
                }
                catch (System.Exception ex)
                {
                    result.SetException(ex);
                }
            }, this))
            {
                try
                {
                    FinishAccept();
                    result.SetResult(Accept());
                }
                catch (System.Exception ex)
                {
                    result.SetException(ex);
                }
            }
            return result.Task;
        }
    }
}
