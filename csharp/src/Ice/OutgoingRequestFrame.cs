//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using Context = System.Collections.Generic.Dictionary<string, string>;

namespace Ice
{
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

        private readonly Encoding _payloadEncoding; // TODO: move to OutputStream

        /// <summary>Creates a new outgoing request frame with no parameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public static OutgoingRequestFrame Empty(IObjectPrx proxy, string operation, bool idempotent,
                                                 Context? context = null)
            => new OutgoingRequestFrame(proxy, operation, idempotent, context, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing request frame. This frame is incomplete and its payload needs to be
        /// provided using StartParameters/EndParameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, Context? context = null)
            : base(proxy.Communicator)
        {
            proxy.IceReference.GetProtocol().CheckSupported();

            Identity = proxy.Identity;
            Facet = proxy.Facet;
            Operation = operation;
            IsIdempotent = idempotent;
            _payloadEncoding = proxy.Encoding;

            WriteSpan(Ice1Definitions.RequestHeader.AsSpan());
            Identity.IceWrite(this);
            if (Facet.Length == 0)
            {
                WriteStringSeq(Array.Empty<string>());
            }
            else
            {
                WriteStringSeq(new string[]{ Facet });
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
                if (proxy.Context != null)
                {
                    Context = new Context(proxy.Context);
                }
                else
                {
                    Context = new Context();
                }

                // TODO: simplify implicit context
                var implicitContext = (ImplicitContext?)proxy.Communicator.GetImplicitContext();
                if (implicitContext != null)
                {
                    Context = implicitContext.Combine(Context);
                }
            }

            ContextHelper.Write(this, Context);
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
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, Context? context,
                                    ArraySegment<byte> payload)
            : this(proxy, operation, idempotent, context)
        {
            if (payload.Count == 0)
            {
                WriteEmptyEncapsulation(_payloadEncoding);
            }
            else
            {
                WritePayload(payload);
            }
        }

        /// <summary>Starts writing the parameters for this request.</summary>
        /// <param name="format">The format for the parameters, null (meaning keep communicator's setting), SlicedFormat
        /// or CompactFormat.</param>
        public void StartParameters(FormatType? format = null)
            => StartEncapsulation(_payloadEncoding, format);

        /// <summary>Marks the end of the parameters.</summary>
        public void EndParameters() => EndEncapsulation();
    }
}
