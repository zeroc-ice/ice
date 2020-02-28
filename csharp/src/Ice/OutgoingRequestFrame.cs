//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    using Context = Dictionary<string, string>;

    /// <summary>Represents a request protocol frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutputStream
    {
        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Context Context { get; }

        /// <summary>Creates a new outgoing request frame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="format">The Slice format (Compact or Sliced) used by the encapsulation.</param>
        /// <param name="payloadWriter">An action that writes the contents of the payload.</param>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, Context? context,
                                    FormatType? format, Action<OutputStream> payloadWriter)
            : this(proxy.Communicator, proxy.Identity, proxy.Facet, operation, idempotent, proxy.Context, context)
        {
            StartEncapsulation(proxy.EncodingVersion, format);
            payloadWriter(this);
            EndEncapsulation();
        }

        /// <summary>Creates a new outgoing request frame with a null format.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="payloadWriter">An action that writes the contents of the payload.</param>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, Context? context,
                                    Action<OutputStream> payloadWriter)
            : this(proxy, operation, idempotent, context, format:null, payloadWriter)
        {
        }

        /// <summary>Creates a new outgoing request frame with the given payload.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet
        /// and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="payload">The payload of this request frame, which represents the marshaled in-parameters.
        /// </param>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, Context? context = null,
                                    ArraySegment<byte>? payload = null)
            : this(proxy.Communicator, proxy.Identity, proxy.Facet, operation, idempotent, proxy.Context, context)
        {
            if (payload == null || payload.Value.Count == 0)
            {
                WriteEmptyEncapsulation(proxy.EncodingVersion);
            }
            else
            {
                // TODO: works only because we know how payload is created!
                WriteBlob(payload.Value.Array);
            }
        }

        private OutgoingRequestFrame(Communicator communicator, Identity identity, string facet, string operation,
                                     bool idempotent, Context? prxContext, Context? context)
            : base(communicator)
        {
            Identity = identity;
            Facet = facet;
            Operation = operation;
            IsIdempotent = idempotent;

            WriteBlob(Protocol.requestHdr);
            identity.IceWrite(this);
            if (facet.Length == 0)
            {
                WriteStringSeq(System.Array.Empty<string>());
            }
            else
            {
                WriteStringSeq(new string[]{ facet });
            }

            WriteString(operation);
            this.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);

            if (context != null)
            {
                // Explicit context
                Context = new Context(context);
            }
            else
            {
                 Context = new Context(prxContext) ?? new Dictionary<string, string>();

                // TODO: simplify implicit context
                var implicitContext = (ImplicitContext?)communicator.GetImplicitContext();
                if (implicitContext != null)
                {
                    Context = implicitContext.Combine(Context);
                }
            }

            ContextHelper.Write(this, Context);
        }
    }
}
