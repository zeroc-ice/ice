import java.net.*;
import java.io.*;

final class Protocol
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
    public final static int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    public final static byte magic[] = { 0x49, 0x63, 0x65, 0x50 };	// 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    public final static byte protocolMajor = 1;
    public final static byte protocolMinor = 0;
    public final static byte encodingMajor = 1;
    public final static byte encodingMinor = 0;

    //
    // The Ice protocol message types
    //
    public final static byte requestMsg = 0;
    public final static byte requestBatchMsg = 1;
    public final static byte replyMsg = 2;
    public final static byte validateConnectionMsg = 3;
    public final static byte closeConnectionMsg = 4;

    public final static byte[] requestHdr =
    {
	Protocol.magic[0],
	Protocol.magic[1],
	Protocol.magic[2],
	Protocol.magic[3],
	Protocol.protocolMajor,
	Protocol.protocolMinor,
	Protocol.encodingMajor,
	Protocol.encodingMinor,
	Protocol.requestMsg,
	(byte)0, // Compression status.
	(byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
	(byte)0, (byte)0, (byte)0, (byte)0  // Request ID (placeholder).
    };

    public final static byte[] requestBatchHdr =
    {
	Protocol.magic[0],
	Protocol.magic[1],
	Protocol.magic[2],
	Protocol.magic[3],
	Protocol.protocolMajor,
	Protocol.protocolMinor,
	Protocol.encodingMajor,
	Protocol.encodingMinor,
	Protocol.requestBatchMsg,
	0, // Compression status.
	(byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
	(byte)0, (byte)0, (byte)0, (byte)0  // Number of requests in batch (placeholder).
    };

    public final static byte[] replyHdr =
    {
	Protocol.magic[0],
	Protocol.magic[1],
	Protocol.magic[2],
	Protocol.magic[3],
	Protocol.protocolMajor,
	Protocol.protocolMinor,
	Protocol.encodingMajor,
	Protocol.encodingMinor,
	Protocol.replyMsg,
	(byte)0, // Compression status.
	(byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

}

class Server 
{
    static int
    readSize(java.nio.ByteBuffer buf)
    {
	int sz = buf.get();
	if(sz == 255)
	{
	    return buf.getInt();
	}
	return sz;
    }

    static boolean
    rawcmp(byte[] src, byte[] dest, int len)
    {
	if(src.length != len)
	{
	    return false;
	}
	for(int i = 0; i < len; ++i)
	{
	    if(src[i] != dest[i])
	    {
		return false;
	    }
	}
	return true;
    }

    static void
    run(Socket clientSocket)
	throws IOException
    {
	InputStream is = clientSocket.getInputStream();
	OutputStream os = clientSocket.getOutputStream();

	byte[] header = new byte[14];

	//
	// Construct validation message, send.
	//
	java.nio.ByteBuffer headerBuf = java.nio.ByteBuffer.wrap(header);
	headerBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
	headerBuf.put(Protocol.magic);
	headerBuf.put(Protocol.protocolMajor).put(Protocol.protocolMinor);
	headerBuf.put(Protocol.encodingMajor).put(Protocol.encodingMinor);
	headerBuf.put(Protocol.validateConnectionMsg);
	headerBuf.put((byte)0);
	headerBuf.putInt(Protocol.headerSize);

	os.write(header);
	os.flush();

	byte[] opNameRaw = new byte[100];
	byte[] magic = new byte[4];
	byte[] req = new byte[500042];
	java.nio.ByteBuffer reqBuf = java.nio.ByteBuffer.wrap(req);
	reqBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);

	byte[] reply = new byte[25];
	for(int i = 0; i < Protocol.replyHdr.length; ++i)
	{
	    reply[i] = Protocol.replyHdr[i];
	}
	java.nio.ByteBuffer replyBuf = java.nio.ByteBuffer.wrap(reply);
	replyBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
	replyBuf.position(10);
	replyBuf.putInt(reply.length);
	replyBuf.putInt(0); // requestId
	replyBuf.put((byte)0); // Success
	// Encapsulation
	replyBuf.putInt(6);
	replyBuf.put(Protocol.encodingMajor);
	replyBuf.put(Protocol.encodingMinor);

	byte[] isaReply = new byte[26];
	for(int i = 0; i < Protocol.replyHdr.length; ++i)
	{
	    isaReply[i] = Protocol.replyHdr[i];
	}
	java.nio.ByteBuffer isaReplyBuf = java.nio.ByteBuffer.wrap(isaReply);
	isaReplyBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
	isaReplyBuf.position(10);
	isaReplyBuf.putInt(isaReply.length);
	isaReplyBuf.putInt(0); // requestId
	isaReplyBuf.put((byte)0); // Success
	// Encapsulation
	isaReplyBuf.putInt(7);
	isaReplyBuf.put(Protocol.encodingMajor);
	isaReplyBuf.put(Protocol.encodingMinor);
	isaReplyBuf.put((byte)1);

	byte[] opIceIsA = "ice_isA".getBytes();
	byte[] opIcePing = "ice_ping".getBytes();
	byte[] opSendByteSeq = "sendByteSeq".getBytes();

	//
	// Process requests.
	//
	while(true)
	{
	    int len = 0;
	    while(len != header.length)
	    {
		int n = is.read(header, len, header.length - len);
		if(n <= 0)
		{
		    return;
		}
		len += n;
	    }

	    headerBuf.position(0);
	    magic[0] = headerBuf.get();
	    magic[1] = headerBuf.get();
	    magic[2] = headerBuf.get();
	    magic[3] = headerBuf.get();
	    assert(magic[0] == Protocol.magic[0] && magic[1] == Protocol.magic[1] &&
		   magic[2] == Protocol.magic[2] && magic[3] == Protocol.magic[3]);

	    byte pMajor = headerBuf.get();
	    assert(pMajor == Protocol.protocolMajor);
	    byte pMinor = headerBuf.get();

	    byte eMajor = headerBuf.get();
	    assert(eMajor == Protocol.encodingMajor);
	    byte eMinor = headerBuf.get();

	    byte messageType = headerBuf.get();
	    byte compress = headerBuf.get();
	    int size = headerBuf.getInt();
	    //System.err.println("size: " + size);

	    if(messageType == Protocol.closeConnectionMsg)
	    {
		return;
	    }
	    if(messageType != Protocol.requestMsg)
	    {
		System.err.println("unexpected message: " + messageType);
		return;
	    }
		    
	    size -= Protocol.headerSize;
	    if(size < 0 || size > req.length)

	    {
		System.err.println("booting client: unsupported size");
		return;
	    }
	    //assert(size <= req.length);
	    //System.err.println("size: " + size);

	    len = 0;
	    while(len != size)
	    {
		int n = is.read(req, len, size - len);
		if(n <= 0)
		{
		    return;
		}
		//System.out.println("read: " + n);
		len += n;
	    }

	    reqBuf.position(0);
	    int requestId = reqBuf.getInt();

	    //
	    // Partially parse the message header.
	    //
		
	    // id
	    int sz = readSize(reqBuf);
	    while(sz != 0)
	    {
		reqBuf.get(); --sz;
	    }
	    sz = readSize(reqBuf);
	    while(sz != 0)
	    {
		reqBuf.get(); --sz;
	    }
	    // facet
	    sz = reqBuf.get();
	    assert(sz == 0);
	    // operation
	    sz = readSize(reqBuf);
	    assert(sz < opNameRaw.length);
	    if(sz != 0)
	    {
		reqBuf.get(opNameRaw, 0, sz);
	    }

	    if(!rawcmp(opIceIsA, opNameRaw, sz) && !rawcmp(opIcePing, opNameRaw, sz) &&
	       !rawcmp(opSendByteSeq, opNameRaw, sz))
	    {
		System.err.println("unsupported op: " + new String(opNameRaw, 0, sz));
		return;
	    }

	    java.nio.ByteBuffer rb;
	    byte[] r;
	    if(rawcmp(opIceIsA, opNameRaw, sz))
	    {
		r = isaReply;
		rb = isaReplyBuf;
	    }
	    else
	    {
		r = reply;
		rb = replyBuf;
	    }

	    //
	    // Compose the reply.
	    //
	    rb.position(14);
	    rb.putInt(requestId);

	    os.write(r);
	    os.flush();
	}
    }

    public static void
    main(String args[]) 
    {
	int port = 10000;
	ServerSocket serverSocket = null;
	try 
	{
	    serverSocket = new ServerSocket(port);
	} 
	catch (IOException e) 
	{
	    System.out.println("Could not listen on port: " + port + ", " + e);
	    System.exit(1);
	}

	while(true)
	{
	    Socket clientSocket = null;
	    try 
	    {
		clientSocket = serverSocket.accept();
	    } 
	    catch (IOException e) 
	    {
		System.out.println("accept failed");
		e.printStackTrace();
		System.exit(1);
	    }
	    System.out.println("accepted new client");

	    try 
	    {
		run(clientSocket);
	    }
	    catch (IOException e) 
	    {
		e.printStackTrace();
	    }

	    try
	    {
		clientSocket.close();
	    }
	    catch (IOException e) 
	    {
		e.printStackTrace();
	    }
	    System.out.println("client is gone");
	}
	//serverSocket.close();
    }
}
