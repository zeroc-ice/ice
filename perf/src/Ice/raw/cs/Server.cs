// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

sealed class Protocol
{
    //
    // Size of the Ice protocol header
    //
    // Magic number (4 bytes)
    // Protocol version major (Byte)
    // Protocol version minor (Byte)
    // Encoding version major (Byte)
    // Encoding version minor (Byte)
    // Message type (Byte)
    // Compression status (Byte)
    // Message size (Int)
    //
    internal const int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    internal static readonly byte[] magic
        = new byte[] { (byte)0x49, (byte)0x63, (byte)0x65, (byte)0x50 }; // 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    internal const byte protocolMajor = 1;
    internal const byte protocolMinor = 0;
    internal const byte encodingMajor = 1;
    internal const byte encodingMinor = 0;

    //
    // The Ice protocol message types
    //
    internal const byte requestMsg = 0;
    internal const byte requestBatchMsg = 1;
    internal const byte replyMsg = 2;
    internal const byte validateConnectionMsg = 3;
    internal const byte closeConnectionMsg = 4;

    internal static readonly byte[] requestHdr = new byte[]
    {
        Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], 
        Protocol.magic[3],
        Protocol.protocolMajor, Protocol.protocolMinor,
        Protocol.encodingMajor, Protocol.encodingMinor,
        Protocol.requestMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0      // Request ID (placeholder).
    };

    internal static readonly byte[] requestBatchHdr = new byte[]
    {
        Protocol.magic[0], Protocol.magic[1], Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor, Protocol.protocolMinor,
        Protocol.encodingMajor, Protocol.encodingMinor,
        Protocol.requestBatchMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0      // Number of requests in batch (placeholder).
    };

    internal static readonly byte[] replyHdr = new byte[]
    {
        Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], 
        Protocol.magic[3],
        Protocol.protocolMajor, Protocol.protocolMinor,
        Protocol.encodingMajor, Protocol.encodingMinor,
        Protocol.replyMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

}

public class Server
{
    public enum ByteOrder { BIG_ENDIAN, LITTLE_ENDIAN };
    private class NO // Native Order
    {
        static NO()
        {
            byte[] b = BitConverter.GetBytes((int)1);
            _o = b[0] == 1 ? ByteOrder.LITTLE_ENDIAN : ByteOrder.BIG_ENDIAN;
        }
        internal static readonly ByteOrder _o;
    }
    //static ByteOrder _order = ByteOrder.BIG_ENDIAN;
    static ByteOrder _order = ByteOrder.LITTLE_ENDIAN;

    static unsafe void putInt(ref byte[] buf, ref int pos, int val)
    {
        if(NO._o == _order)
        {
            fixed(byte* p = &buf[pos])
            {
                *((int*)p) = val;
            }   
            pos += 4;
        }
        else
        {
            byte* p = (byte*)&val;
            buf[pos++] = *(p+3);
            buf[pos++] = *(p+2);
            buf[pos++] = *(p+1);
            buf[pos++] = *(p+0);
        }
    }

    static unsafe int getInt(byte[] buf, ref int pos)
    {
        int ret;
        if(NO._o == _order)
        {
            fixed(byte* p = &buf[pos])
            {
                ret = *((int*)p);
            }
        }
        else
        {
            byte* p = (byte*)&ret;
            *p++ = buf[pos+3];
            *p++ = buf[pos+2];
            *p++ = buf[pos+1];
            *p = buf[pos+0];
        }
        pos += 4;
        return ret;
    }

    static int readSize(byte[] buf, ref int pos)
    {
        int sz = buf[pos++];
        if(sz == 255)
        {
            return getInt(buf, ref pos);
        }
        return sz;
    }

    static bool rawcmp(byte[] src, byte[] dest, int offset, int len)
    {
        if(src.Length != len)
        {
            return false;
        }
        for(int i = 0; i < len; ++i)
        {
            if(src[i] != dest[offset + i])
            {
                return false;
            }
        }
        return true;
    }

    public static void
    run(Socket sock)
    {
        System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        byte[] header = new byte[Protocol.headerSize];

        //
        // Construct validation message, send.
        //
        int offset = 0;
        Array.Copy(Protocol.magic, 0, header, 0, Protocol.magic.Length);
        offset += Protocol.magic.Length;
        header[offset++] = Protocol.protocolMajor;
        header[offset++] = Protocol.protocolMinor;
        header[offset++] = Protocol.encodingMajor;
        header[offset++] = Protocol.encodingMinor;
        header[offset++] = Protocol.validateConnectionMsg;
        header[offset++] = (byte)0;
        putInt(ref header, ref offset, Protocol.headerSize);

        if(sock.Send(header) != header.Length)
        {
            return;
        }

        byte[] req = new byte[500042];

        byte[] reply = new byte[25];
        Array.Copy(Protocol.replyHdr, 0, reply, 0, Protocol.replyHdr.Length);
        offset = 10;
        putInt(ref reply, ref offset, reply.Length);
        putInt(ref reply, ref offset, 0); // request id
        reply[offset++] = (byte)0; // Success
        // Encapsulation
        putInt(ref reply, ref offset, 6);
        reply[offset++] = Protocol.encodingMajor;
        reply[offset++] = Protocol.encodingMinor;

        byte[] isaReply = new byte[26];
        Array.Copy(Protocol.replyHdr, 0, isaReply, 0, Protocol.replyHdr.Length);
        offset = 10;
        putInt(ref isaReply, ref offset, isaReply.Length);
        putInt(ref isaReply, ref offset, 0); // request id
        isaReply[offset++] = (byte)0; // Success
        // Encapsulation
        putInt(ref isaReply, ref offset, 7);
        isaReply[offset++] = Protocol.encodingMajor;
        isaReply[offset++] = Protocol.encodingMinor;
        isaReply[offset++] = (byte)1;

        byte[] opIceIsA = utf8.GetBytes("ice_isA");
        byte[] opIcePing = utf8.GetBytes("ice_ping");
        byte[] opSendByteSeq = utf8.GetBytes("sendByteSeq");

        //
        // Process requests.
        //
        while(true)
        {
            int len = 0;
            while(len != header.Length)
            {
                int n = sock.Receive(header, len, header.Length - len, 0);
                if(n <= 0)
                {
                    return;
                }
                len += n;
            }

            Debug.Assert(header[0] == Protocol.magic[0] && header[1] == Protocol.magic[1] &&
                         header[2] == Protocol.magic[2] && header[3] == Protocol.magic[3]);

            offset = 4;
            byte pMajor = header[offset++];
            Debug.Assert(pMajor == Protocol.protocolMajor);
            //byte pMinor = header[offset++];
            offset++;

            byte eMajor = header[offset++];
            Debug.Assert(eMajor == Protocol.encodingMajor);
            //byte eMinor = header[offset++];
            offset++;

            byte messageType = header[offset++];
            //byte compress = header[offset++];
            offset++;
            int size = getInt(header, ref offset);
            //Console.WriteLine("size: " + size);

            if(messageType == Protocol.closeConnectionMsg)
            {
                return;
            }
            if(messageType != Protocol.requestMsg)
            {
                Console.WriteLine("unexpected message: " + messageType);
                return;
            }
                    
            size -= Protocol.headerSize;
            if(size < 0 || size > req.Length)
            {
                Console.WriteLine("booting client: unsupported size");
                return;
            }
            //assert(size <= req.Length);

            len = 0;
            while(len != size)
            {
                int n = sock.Receive(req, len, size - len, 0);
                if(n <= 0)
                {
                    return;
                }
                //Console.WriteLine("read: " + n);
                len += n;
            }

            offset = 0;
            int requestId = getInt(req, ref offset);

            // id
            int sz = readSize(req, ref offset);
            offset += sz;
            sz = readSize(req, ref offset);
            offset += sz;
            Debug.Assert(req[offset] == 0);
            ++offset; // facet
            // operation
            sz = readSize(req, ref offset);

            if(!rawcmp(opIceIsA, req, offset, sz) && !rawcmp(opIcePing, req, offset, sz) &&
               !rawcmp(opSendByteSeq, req, offset, sz))
            {
                Console.WriteLine("unsupported op: " + utf8.GetString(req, offset, sz));
                return;
            }

            byte[] r;
            if(rawcmp(opIceIsA, req, offset, sz))
            {
                r = isaReply;
            }
            else
            {
                r = reply;
            }

            //
            // Compose the reply.
            //
            offset = 14;
            putInt(ref r, ref offset, requestId);

            if(sock.Send(r) != r.Length)
            {
                return;
            }
        }
    }

    public static void Main()
    {
        try
        {
            IPAddress address = IPAddress.Parse("127.0.0.1");
            TcpListener listener = new TcpListener(address,10000);
            listener.Start();
            
            while (true)
            {
                Socket s = listener.AcceptSocket();
                Console.WriteLine("Connection accepted from " + s.RemoteEndPoint);
                run(s);
                s.Close();
            }
        }
        catch (Exception e)
        {
            Console.WriteLine("Error..... " + e.StackTrace);
        }    
    }
}
