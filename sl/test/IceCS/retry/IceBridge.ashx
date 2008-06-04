<%@ WebHandler Language="c#" class="IceBridge.Bridge" %>

// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Web;
using System.Collections.Specialized;
using System.Collections.Generic;

namespace IceBridge
{
    sealed class ReplyStatus
    {
        public const byte replyOK = 0;
        public const byte replyUserException = 1;
        public const byte replyObjectNotExist = 2;
        public const byte replyFacetNotExist = 3;
        public const byte replyOperationNotExist = 4;
        public const byte replyUnknownLocalException = 5;
        public const byte replyUnknownUserException = 6;
        public const byte replyUnknownException = 7;
    }

    sealed class Protocol
    {
        //
        // Size of the Ice http tunnelling protocol header
        //
        // Magic number (4 bytes)
        // Protocol version major (Byte)
        // Protocol version minor (Byte)
        // Encoding version major (Byte)
        // Encoding version minor (Byte)
        //
        internal const int headerSize = 8;
        
        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] magic
            = new byte[] { (byte)0x49, (byte)0x48, (byte)0x54, (byte)0x50 }; // 'I', 'H', 'T', 'P'
        
        //
        // The current Ice protocol and encoding version
        //
        internal const byte protocolMajor = 1;
        internal const byte protocolMinor = 0;
        internal const byte encodingMajor = 1;
        internal const byte encodingMinor = 0;
        
        internal static readonly byte[] requestHdr = new byte[]
        {
            Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], 
            Protocol.magic[3],
            Protocol.protocolMajor, Protocol.protocolMinor,
            Protocol.encodingMajor, Protocol.encodingMinor,
        };

        internal static readonly byte[] replyHdr = new byte[]
        {
            Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], 
            Protocol.magic[3],
            Protocol.protocolMajor, Protocol.protocolMinor,
            Protocol.encodingMajor, Protocol.encodingMinor,
        };
    }

    public class Bridge : IHttpHandler
    {
        private EventLog _log;

        public Bridge()
        {
            _log = new EventLog();
            _log.Source = "IceBridge.Bridge";
        }

        public void
        read(System.IO.Stream s, byte[] data, int pos, int remaining)
        {
            while(remaining > 0)
            {
                int ret = s.Read(data, pos, remaining);
                if(ret == 0)
                {
                    throw new System.IO.IOException();
                }
                remaining -= ret;
            }
        }

        public void ProcessRequest(HttpContext context)
        {
            try
            {
                Ice.Communicator com = (Ice.Communicator)context.Application["com"];

                byte[] requestData = new byte[context.Request.ContentLength];
                read(context.Request.InputStream, requestData, 0, context.Request.ContentLength);
                Ice.InputStream stream = Ice.Util.createInputStream(com, requestData);

                byte[] m = stream.readBlob(4);
                if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                   m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                {
                    Ice.BadMagicException ex = new Ice.BadMagicException();
                    ex.badMagic = m;
                    throw ex;
                }
                byte pMajor = stream.readByte();
                byte pMinor = stream.readByte();
                if(pMajor != Protocol.protocolMajor)
                {
                    Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
                    e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                    e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                    e.major = Protocol.protocolMajor;
                    e.minor = Protocol.protocolMinor;
                    throw e;
                }
                byte eMajor = stream.readByte();
                byte eMinor = stream.readByte();
                if(eMajor != Protocol.encodingMajor)
                {
                    Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                    e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                    e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                    e.major = Protocol.encodingMajor;
                    e.minor = Protocol.encodingMinor;
                    throw e;
                }

                String op = stream.readString();
                Ice.ObjectPrx proxy = stream.readProxy();
                Ice.OperationMode mode = (Ice.OperationMode)stream.readByte();
                if(mode != Ice.OperationMode.Normal && mode != Ice.OperationMode.Nonmutating && 
                   mode != Ice.OperationMode.Idempotent)
                {
                    throw new Ice.MarshalException("unexpected mode received: " + mode);
                }

                Dictionary<string, string> ctx = null;
                int sz = stream.readSize();
                while(sz-- > 0)
                {
                    string first = stream.readString();
                    string second = stream.readString();
                    if(ctx == null)
                    {
                        ctx = new Dictionary<string, string>();
                    }
                    ctx[first] = second;
                }

                Ice.OutputStream os = Ice.Util.createOutputStream(com);

                byte replyStatus = 0;
                if(op == "opkill")
                {
                    replyStatus = ReplyStatus.replyUnknownException;
                    //Ice.Identity id = proxy.ice_getIdentity();
                    //os.writeString(id.name);
                    //os.writeString(id.category);
                    //os.writeStringSeq(null);
                    //os.writeString(op);
                    os.writeString("foo");
                }
                else if(op == "operror")
                {
                    context.Response.StatusCode = 500;
                    return;
                }
                else
                {
                    replyStatus = ReplyStatus.replyOK;
                    os.startEncapsulation();
                    os.writeBlob(new byte[0]);
                    os.endEncapsulation();
                }

                byte[] replyData = os.finished();
                context.Response.ContentType = "application/binary";
                context.Response.OutputStream.Write(Protocol.replyHdr, 0, Protocol.replyHdr.Length);
                context.Response.OutputStream.Write(BitConverter.GetBytes(replyData.Length + 1), 0, 4);
                context.Response.OutputStream.WriteByte(replyStatus);
                context.Response.OutputStream.Write(replyData, 0, replyData.Length);
            }
            catch(Exception ex)
            {
                Console.WriteLine("Bridging error: " + ex.ToString());
                Debug.Write("Bridging error: " + ex.ToString());
                _log.WriteEntry("Bridging Error: " + ex.ToString());
                context.Response.StatusCode = 500;
            }
        }

        public bool IsReusable
        {
            get
            {
                return true;
            }
        }
    }
}
