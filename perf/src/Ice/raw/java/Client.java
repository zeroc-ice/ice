import java.io.*;
import java.net.*;

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

public class Client 
{
    static public void run(Socket sock)
	throws IOException
    {
	InputStream is = sock.getInputStream();
	OutputStream os = sock.getOutputStream();

	int seq = 500000; // byte sequence length
	byte[] req = new byte[seq + Protocol.headerSize + 42]; // 42 is the size of the message header
	for(int i = 0; i < Protocol.headerSize; ++i)
	{
	    req[i] = Protocol.requestHdr[i];
	}
	java.nio.ByteBuffer reqBuf = java.nio.ByteBuffer.wrap(req);
	reqBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
	reqBuf.position(10);
	reqBuf.putInt(req.length);
	reqBuf.putInt(0); // request id
	// Ice::Identity
	String idName = "throughput";
	reqBuf.put((byte)idName.length());
	reqBuf.put(idName.getBytes());
	reqBuf.put((byte)0);
	reqBuf.put((byte)0); // facet
	String opName = "sendByteSeq"; // operation name
	reqBuf.put((byte)opName.length());
	reqBuf.put(opName.getBytes());
	reqBuf.put((byte)0); // mode
	reqBuf.put((byte)0); // context
	// encapsulation
	reqBuf.putInt(50011);
	reqBuf.put(Protocol.encodingMajor).put(Protocol.encodingMinor);
	reqBuf.put((byte)255);
	reqBuf.putInt(seq);
	
	int requestId = 1;

	byte[] reply = new byte[25];

	byte[] header = new byte[14];
	java.nio.ByteBuffer headerBuf = java.nio.ByteBuffer.wrap(header);
	headerBuf.order(java.nio.ByteOrder.LITTLE_ENDIAN);

	byte[] magic = new byte[4];

	// Read connection validation.
	int len = 0;
	int size = header.length;
	while(len != size)
	{
	    int n = is.read(header, len, size - len);
	    if(n <= 0)
	    {
		return;
	    }
	    len += n;
	}

	// Process validation message.
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
	assert(messageType == Protocol.validateConnectionMsg);

	byte compress = headerBuf.get();
	size = headerBuf.getInt();

	double tmsec = 0;

	// Send some requests. First we want to "warm up" the JIT compiler.
	int warm = 0;
	int repetitions = 200;
	for (int i = 0; i < repetitions + warm; ++i)
	{
	    if(i == warm)
	    {
		tmsec = System.currentTimeMillis();
	    }

	    // Request message body.
	    reqBuf.position(14);
	    reqBuf.putInt(requestId); // id
	    ++requestId;

	    os.write(req);
	    os.flush();

	    len = 0;
	    size = reply.length;
	    while(len != size)
	    {
		int n = is.read(reply, len, size - len);
		if(n <= 0)
		{
		    return;
		}
		len += n;
	    }
	}

	double dmsec = System.currentTimeMillis() - tmsec;
	System.err.println("time for " + repetitions + " sequences: " + dmsec + "ms");
	System.err.println("time per sequence: " + (dmsec / repetitions) + "ms");
    }

    public static void main(String[] args) 
    {
	try 
	{
	    Socket sock = new Socket("127.0.0.1", 10000);
	    System.out.println("connected");

	    run(sock);

	    sock.close();
	} 
	catch (UnknownHostException e) 
	{
	    System.err.println("Trying to connect to unknown host: " + e);
	} 
	catch (Exception e) 
	{
	    System.err.println("Exception:  " + e);
	}
    }
}
