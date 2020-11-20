// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>An abstract multi-stream socket which is using a single stream socket for receiving and sending
    /// data.</summary>
    internal abstract class MultiStreamOverSingleStreamSocket : MultiStreamSocket
    {
        internal SingleStreamSocket Underlying { get; }

        public override string ToString() => Underlying.ToString()!;

        public override void Abort() => Underlying.Dispose();

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                Underlying.Dispose();
            }
        }

        protected MultiStreamOverSingleStreamSocket(
            Endpoint endpoint,
            ObjectAdapter? adapter,
            SingleStreamSocket socket)
            : base(endpoint, adapter) => Underlying = socket;
    }
}
