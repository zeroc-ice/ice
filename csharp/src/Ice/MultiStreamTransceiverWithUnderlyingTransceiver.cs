// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>An abstract multi-stream transceiver based on a single stream transceiver.</summary>
    internal abstract class MultiStreamTransceiverWithUnderlyingTransceiver : MultiStreamTransceiver
    {
        internal ITransceiver Underlying { get; }

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

        protected MultiStreamTransceiverWithUnderlyingTransceiver(
            Endpoint endpoint,
            ObjectAdapter? adapter,
            ITransceiver transceiver)
            : base(endpoint, adapter) => Underlying = transceiver;
    }
}
