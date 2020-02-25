using IceInternal;
using System;
using System.Collections.Generic;

namespace Ice
{
    public sealed class OutgoingRequestFrame : OutputStream
    {
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent,
            Dictionary<string, string>? context) : base(proxy.Communicator, proxy.EncodingVersion)
        {
            if (proxy.InvocationMode == InvocationMode.BatchOneway ||
                proxy.InvocationMode == InvocationMode.BatchDatagram)
            {
                throw new NotImplementedException("Batch invocation modes are not implemented");
            }

            WriteBlob(Protocol.requestHdr);
            proxy.Identity.IceWrite(this);
            string facet = proxy.Facet;
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

            // TODO: defer marshaling of context
            if (context != null)
            {
                // Explicit context
                ContextHelper.Write(this, context);
            }
            else
            {
                // Implicit context
                var implicitContext = (ImplicitContext?)proxy.Communicator.GetImplicitContext();

                // TODO: need better empty context
                context = proxy.Context ?? new Dictionary<string, string>();
                if (implicitContext == null)
                {
                    ContextHelper.Write(this, context);
                }
                else
                {
                    implicitContext.Write(context, this);
                }
            }
        }
    }
}
