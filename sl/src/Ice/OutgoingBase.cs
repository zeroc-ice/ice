// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using System.Net;
    using System.IO;
    using System;
    using Ice;

#if !SILVERLIGHT
    using System.Collections.Specialized;
    using System.Web;
#endif

    public class OutgoingBase
    {
        protected void
        writeHeader__(Reference r, string operation, OperationMode mode,
                      Dictionary<string, string> context)
        {
            Reference.Mode reqmode = r.getMode();
            if(reqmode != Reference.Mode.ModeTwoway &&
               reqmode != Reference.Mode.ModeOneway)
            {
                throw new FeatureNotSupportedException("Only oneway and twoway invocations are supported.");
            }
            if(r.getSecure())
            {
                throw new FeatureNotSupportedException("Secure invocations are not supported.");
            }
            if(r.getCompress())
            {
                throw new FeatureNotSupportedException("Protocol compression is not supported.");
            }
            if(r.getTimeout() != -1)
            {
                throw new FeatureNotSupportedException("Timeouts are not supported.");
            }

            os__.writeBlob(IceInternal.Protocol.requestHdr);

            os__.writeString(operation);

            // Stream the proxy. This cannot use os.writeProxy since Reference is not a proxy.
            r.getIdentity().write__(os__);
            r.streamWrite(os__);

            os__.writeByte((byte)mode);

            if(context != null)
            {
                //
                // Explicit context
                //
                ContextHelper.write(os__, context);
            }
            else
            {
                //
                // Implicit context
                //
                ImplicitContextI implicitContext =
                    r.getInstance().getImplicitContext();
                Dictionary<string, string> prxContext = r.getContext();
                if(implicitContext == null)
                {
                    ContextHelper.write(os__, prxContext);
                }
                else
                {
                    implicitContext.write(prxContext, os__);
                }
            }

            //
            // Input and output parameters are always sent in an
            // encapsulation, which makes it possible to forward
            // requests as blobs.
            //
            os__.startWriteEncaps();
        }

        // Returns true if ok, false if user exception.
        public bool
        handleResponse__()
        {
            // Position the stream at the head.
            is__.pos(0);

            // Read the protocol header.
            byte[] m = is__.readBlob(4);
            if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
               m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
            {
                BadMagicException ex = new BadMagicException();
                ex.badMagic = m;
                throw ex;
            }
            byte pMajor = is__.readByte();
            byte pMinor = is__.readByte();
            if(pMajor != IceInternal.Protocol.protocolMajor)
            {
                UnsupportedProtocolException e = new UnsupportedProtocolException();
                e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                e.major = IceInternal.Protocol.protocolMajor;
                e.minor = IceInternal.Protocol.protocolMinor;
                throw e;
            }
            byte eMajor = is__.readByte();
            byte eMinor = is__.readByte();
            if(eMajor != IceInternal.Protocol.encodingMajor)
            {
                UnsupportedEncodingException e = new UnsupportedEncodingException();
                e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                e.major = IceInternal.Protocol.encodingMajor;
                e.minor = IceInternal.Protocol.encodingMinor;
                throw e;
            }

            is__.readBlob(4); // Reply size

            byte replyStatus = is__.readByte();
            switch(replyStatus)
            {
            case ReplyStatus.replyOK:
                //
                // Input and output parameters are always sent in an
                // encapsulation, which makes it possible to forward
                // oneway requests as blobs.
                //
                is__.startReadEncaps();
                return true;

            case ReplyStatus.replyUserException:
                //
                // Input and output parameters are always sent in an
                // encapsulation, which makes it possible to forward
                // oneway requests as blobs.
                //
                is__.startReadEncaps();
                return false;

            case ReplyStatus.replyObjectNotExist:
            case ReplyStatus.replyFacetNotExist:
            case ReplyStatus.replyOperationNotExist:
            {
                RequestFailedException ex = null;
                if(replyStatus.Equals(ReplyStatus.replyObjectNotExist))
                {
                    ex = new ObjectNotExistException();
                }
                else if(replyStatus.Equals(ReplyStatus.replyFacetNotExist))
                {
                    ex = new FacetNotExistException();
                }
                else if(replyStatus.Equals(ReplyStatus.replyOperationNotExist))
                {
                    ex = new OperationNotExistException();
                }
                else
                {
                    Debug.Assert(false);
                }
                ex.id = new Identity();
                ex.id.read__(is__);

                //
                // For compatibility with the old FacetPath.
                //
                string[] facetPath = is__.readStringSeq();
                if(facetPath.Length > 0)
                {
                    if(facetPath.Length > 1)
                    {
                        throw new MarshalException();
                    }
                    ex.facet = facetPath[0];
                }
                else
                {
                    ex.facet = "";
                }

                ex.operation = is__.readString();
                throw ex;
            }

            case ReplyStatus.replyUnknownException:
            case ReplyStatus.replyUnknownLocalException:
            case ReplyStatus.replyUnknownUserException:
            {
                UnknownException ex = null;
                if(replyStatus.Equals(ReplyStatus.replyUnknownException))
                {
                    ex = new UnknownException();
                }
                else if(replyStatus.Equals(ReplyStatus.replyUnknownLocalException))
                {
                    ex = new UnknownLocalException();
                }
                else if(replyStatus.Equals(ReplyStatus.replyUnknownUserException))
                {
                    ex = new UnknownUserException();
                }
                ex.unknown = is__.readString();
                throw ex;
            }

            default:
                throw new UnknownReplyStatusException();
            }
        }

        protected void readResponse__(Stream s, ByteBuffer buf, int sz, ref int position)
        {
            int remaining = sz;
            while(remaining > 0)
            {
                int ret = s.Read(buf.rawBytes(), position, remaining);
                if(ret == 0)
                {
                    throw new Ice.IllegalMessageSizeException("expected " + sz + " bytes, received " +
                                                              (sz - remaining) + " bytes");
                }
                if(traceLevels__.network >= 3)
                {
                    string str = "received " + ret + " of " + remaining + " bytes";
                    logger__.trace(traceLevels__.networkCat, str);
                }
                remaining -= ret;
                buf.position(position += ret);
            }
        }

        protected Uri bridgeUri__ = null;
        protected Logger logger__ = null;
        protected TraceLevels traceLevels__ = null;
        protected BasicStream is__ = null;
        protected BasicStream os__ = null;
    }
}
