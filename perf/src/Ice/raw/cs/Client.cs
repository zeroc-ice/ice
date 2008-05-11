using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text;
using System.Net.Sockets;

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
	(byte)0, (byte)0, (byte)0, (byte)0	// Request ID (placeholder).
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
	(byte)0, (byte)0, (byte)0, (byte)0	// Number of requests in batch (placeholder).
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

public class Client
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
    public static void run(Socket sock)
    {
	System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

	int seq = 500000;
	byte[] req = new byte[seq + Protocol.headerSize + 42]; // 42 is the size of the message header
	Array.Copy(Protocol.requestHdr, 0, req, 0, Protocol.headerSize);

	int offset = 10;
	putInt(ref req, ref offset, req.Length);
	putInt(ref req, ref offset, 0); // request id
	String idName = "throughput";
	req[offset++] = (byte)idName.Length;
	byte[] data = utf8.GetBytes(idName);
	Array.Copy(data, 0, req, offset, data.Length);
	offset += data.Length;

	req[offset++] = 0;
	req[offset++] = 0; // facet

	String opName = "sendByteSeq"; // operation name
	req[offset++] = (byte)opName.Length;
	data = utf8.GetBytes(opName);
	Array.Copy(data, 0, req, offset, data.Length);
	offset += data.Length;

	req[offset++] = 0; // mode
	req[offset++] = 0; // context
	
        // encapsulation for the sendByteSeq request data.
	putInt(ref req, ref offset, 50011);
	req[offset++] = Protocol.encodingMajor;
	req[offset++] = Protocol.encodingMinor;
	req[offset++] = 255; // size
	putInt(ref req, ref offset, seq);

	int requestId = 1;
	byte[] reply = new byte[25];
	byte[] header = new byte[14];
	
	// Read connection validation.
	int len = 0;
	int size = header.Length;
	while(len != size)
	{
	    int n = sock.Receive(header, len, size - len, 0);
	    if(n <= 0)
	    {
		return;
	    }
	    len += n;
	}

	// Process validation message.
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
	Debug.Assert(messageType == Protocol.validateConnectionMsg);

	//byte compress = header[offset++];
	offset++;
	size = getInt(header, ref offset);

	long tmsec = System.DateTime.Now.Ticks / 10000;

	int repetitions = 200;
	for (int i = 0; i < repetitions; ++i)
	{
	    // Request message body.
	    offset = 14;
	    putInt(ref req, ref offset, requestId);
	    ++requestId;

	    if(sock.Send(req) != req.Length)
	    {
		return;
	    }

	    len = 0;
	    size = reply.Length;
	    while(len != size)
	    {
		int n = sock.Receive(reply, len, size - len, 0);
		if(n <= 0)
		{
		    return;
		}
		len += n;
	    }
	}
	
	double dmsec = System.DateTime.Now.Ticks / 10000 - tmsec;
	Console.WriteLine("time for " + repetitions + " sequences: " + dmsec.ToString("F") + "ms");
	Console.WriteLine("time per sequence: " + ((double)(dmsec / repetitions)).ToString("F") + "ms");
    }
    public static void Main() {
        
        try {
            IPEndPoint ipe = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 10000);
            Socket s = new Socket(ipe.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            Console.WriteLine("Connecting.....");
            s.Connect(ipe);

	    run(s);

            s.Close();
        }
        
        catch (Exception e) {
            Console.WriteLine("Error..... " + e.StackTrace);
        }
    }

}
