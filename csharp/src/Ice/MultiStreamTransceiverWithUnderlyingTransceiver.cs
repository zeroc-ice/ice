// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>An abstract multi-stream transceiver based on a single stream transceiver.</summary>
    internal abstract class MultiStreamTransceiverWithUnderlyingTransceiver : MultiStreamTransceiver
    {
        public override TimeSpan IdleTimeout
        {
            get
            {
                lock (_mutex)
                {
                    return _idleTimeout;
                }
            }
            internal set
            {
                lock (_mutex)
                {
                    _idleTimeout = value;
                }
            }
        }

        internal ITransceiver Underlying { get; }
        private TimeSpan _idleTimeout;

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
            : base(endpoint, adapter)
        {
            Underlying = transceiver;
            _idleTimeout = endpoint.Communicator.IdleTimeout;
        }
    }
}
