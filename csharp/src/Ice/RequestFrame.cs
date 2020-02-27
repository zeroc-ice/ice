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

    /// <summary>Holds header information for request frames.</summary>
    public readonly struct RequestFrameHeader
    {
        /// <summary>The identity of the target Ice object.</summary>
        public readonly Identity Identity;

        /// <summary>The facet of the target Ice object.</summary>
        public readonly string Facet;

        /// <summary>The operation called on the Ice object.</summary>
        public readonly string Operation;

         /// <summary>When true, the operation is idempotent.</summary>
        public readonly bool IsIdempotent;

        public RequestFrameHeader(Identity identity, string facet, string operation, bool idempotent)
        {
            Identity = identity;
            Facet = facet;
            Operation = operation;
            IsIdempotent = idempotent;
        }
    }

     /// <summary>Represents a request protocol frame sent by the application.</summary>
     public sealed class OutgoingRequestFrame : OutputStream
    {
        public RequestFrameHeader Header { get; }

        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Context Context { get; }

        /// <summary>An OutputStream is sealed when it can no longer be written into, in particular the payload
        /// is complete and the context is written. Modidying Context once IsSealed is true has no effect.</summary>
        public bool IsSealed { get; private set; } = false; // TODO: move to OutputStream with protected set

        /// <summary>Creates a new outgoing request frame with an empty payload for the in-parameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <returns>A new OutgoingRequestFrame with an empty payload.</returns>
        public static OutgoingRequestFrame Empty(IObjectPrx proxy, string operation, bool idempotent,
            Context? context = null)
        {
            var frame = new OutgoingRequestFrame(proxy.Communicator, proxy.Identity, proxy.Facet, operation,
                idempotent, proxy.Context, context);
            frame.WriteEmptyEncapsulation(proxy.EncodingVersion);
            frame.IsSealed = true;
            return frame;
        }

        /// <summary>Creates a new outgoing request frame and starts the payload (encapsulation) that will hold the
        /// in-parameters in that frame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="format">The Slice format (Compact or Sliced) used by the encapsulation.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame Start(IObjectPrx proxy, string operation, bool idempotent,
            FormatType? format = null, Context? context = null)
        {
            var frame = new OutgoingRequestFrame(proxy.Communicator, proxy.Identity, proxy.Facet, operation, idempotent,
                proxy.Context, context);
            frame.StartEncapsulation(proxy.EncodingVersion, format);
            return frame;
        }

        /// <summary>Creates a new outgoing request frame with for in-parameters payload the in-parameters of an
        /// incoming request frame.</summary>
        /// <param name="incoming">An incoming request frame.</param>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet
        /// and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame FromIncoming(InputStream incoming, IObjectPrx proxy,
            string operation, bool idempotent, Context? context = null)
        {
            var frame = new OutgoingRequestFrame(proxy.Communicator, proxy.Identity, proxy.Facet, operation, idempotent,
                proxy.Context, context);

            incoming.RestartEncapsulation();
            incoming.Pos -= 6; // TODO: make cleaner
            frame.WriteBlob(incoming.ReadEncapsulation(out EncodingVersion _));
            frame.IsSealed = true;
            return frame;
        }

         /// <summary>Marks the end of the in-parameters for this request frame.</summary>
        public void EndParameters()
        {
            Debug.Assert(!IsSealed); // TODO: throw an exception
            EndEncapsulation();
            IsSealed = true;
            // Note: with the 2.0 encoding the frame will be sealed later, after the context is written.
        }

        private OutgoingRequestFrame(Communicator communicator, Identity identity, string facet, string operation,
            bool idempotent, Context? prxContext, Context? context)
            : base(communicator)
        {
            Header = new RequestFrameHeader(identity, facet, operation, idempotent);

            WriteBlob(Protocol.requestHdr);
            identity.IceWrite(this);
            if (facet.Length == 0)
            {
                WriteStringSeq(System.Array.Empty<string>());
            }
            else
            {
                string[] facetPath = { facet };
                WriteStringSeq(facetPath);
            }

            WriteString(operation);
            WriteByte(idempotent ? (byte)0x2 : (byte)0x0);

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
