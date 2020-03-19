//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>Represents a request protocol frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutgoingFrame
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
        public Dictionary<string, string> Context { get; }

        /// <summary>Creates a new outgoing request frame with no parameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public static OutgoingRequestFrame Empty(IObjectPrx proxy, string operation, bool idempotent,
                                                 IReadOnlyDictionary<string, string>? context = null)
            => new OutgoingRequestFrame(proxy, operation, idempotent, context, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing request frame. This frame is incomplete and its payload needs to be
        /// provided using StartParameters/EndParameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent,
                                    IReadOnlyDictionary<string, string>? context = null)
            : base(proxy.Encoding)
        {
            proxy.IceReference.GetProtocol().CheckSupported();

            Identity = proxy.Identity;
            Facet = proxy.Facet;
            Operation = operation;
            IsIdempotent = idempotent;
            var ostr = new OutputStream(proxy.Communicator.DefaultsAndOverrides.DefaultEncoding,
                Data, new OutputStream.Position(0, 0));
            Identity.IceWrite(ostr);
            if (Facet.Length == 0)
            {
                ostr.WriteStringSeq(Array.Empty<string>());
            }
            else
            {
                ostr.WriteStringSeq(new string[]{ Facet });
            }

            ostr.WriteString(operation);
            ostr.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);

            if (context != null)
            {
                Context = new Dictionary<string, string>(context);
            }
            else
            {
                Context = new Dictionary<string, string>(proxy.Communicator.CurrentContext);
                foreach ((string key, string value) in proxy.Context)
                {
                    Context[key] = value; // the proxy Context entry prevails.
                }
            }

            ContextHelper.Write(ostr, Context);
            PayloadStart = ostr.Tail;
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
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent,
                                    IReadOnlyDictionary<string, string>? context, ArraySegment<byte> payload)
            : this(proxy, operation, idempotent, context)
        {
            var ostr = new OutputStream(Encoding, Data, PayloadStart);
            if (payload.Count == 0)
            {
                ostr.WriteEmptyEncapsulation(Encoding);
            }
            else
            {
                ostr.WritePayload(payload);
            }
            OutputStream.Position payloadEnd = ostr.Finish();
            Size = Data.GetByteCount();
            PayloadEnd = payloadEnd;
            IsSealed = true;
        }

        /// <summary>Creates and returns an OutputStream that can be used to write the payload of this request,
        /// once the caller finish writing the payload it must call SavePayload passing the returned
        /// stream as argument.</summary>
        /// <param name="format">The format type for the payload.</param>
        /// <returns>An OutputStream instance that can be used to write the payload of this request.</returns>
        public override OutputStream WritePayload(FormatType? format = null)
        {
            if (PayloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a payload");
            }

            var ostr = new OutputStream(this, PayloadStart);
            ostr.StartEncapsulation(format);
            return ostr;
        }
    }
}
