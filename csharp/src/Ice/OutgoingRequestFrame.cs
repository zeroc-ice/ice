using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    using Context = Dictionary<string, string>;

    public readonly struct RequestFrameHeader
    {
        public readonly Identity Identity;
        public readonly string Facet;
        public readonly string Operation;
        public readonly bool IsIdempotent;

        public RequestFrameHeader(Identity identity, string facet, string operation, bool idempotent)
        {
            Identity = identity;
            Facet = facet;
            Operation = operation;
            IsIdempotent = idempotent;
        }
    }

    public sealed class OutgoingRequestFrame : OutputStream
    {
        public ref readonly RequestFrameHeader Header => ref _header;
        public Context Context { get; }

        /// <summary>An OutputStream is sealed when it can no longer be written into, in particular the payload
        /// is complete and the context is written. Modidying Context once IsSealed is true has no effect.</summary>
        public bool IsSealed { get; private set; } = false; // TODO: move to OutputStream with protected set

        private readonly RequestFrameHeader _header;
        public static OutgoingRequestFrame Empty(IObjectPrx proxy, string operation, bool idempotent,
            Dictionary<string, string>? context = null)
        {
            var frame = new OutgoingRequestFrame(proxy.Communicator, proxy.Identity, proxy.Facet, operation,
                idempotent, proxy.Context, context);
            frame.WriteEmptyEncapsulation(proxy.EncodingVersion);
            frame.IsSealed = true;
            return frame;
        }

        /// <summary>A convenience constructor.</summary>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent, FormatType? format = null,
            Dictionary<string, string>? context = null)
            : this(proxy.Communicator, proxy.Identity, proxy.Facet, operation, idempotent, proxy.Context, context)
        {
            // Start in-parameter encapsulation, with the same encoding:
            StartEncapsulation(proxy.EncodingVersion, format);
        }

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
            _header = new RequestFrameHeader(identity, facet, operation, idempotent);

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
