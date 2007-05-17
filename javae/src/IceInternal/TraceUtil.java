// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class TraceUtil
{
    public static void
    traceHeader(String heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

	    java.io.OutputStream os = new java.io.ByteArrayOutputStream();
	    java.io.PrintStream ps = new java.io.PrintStream(os);
            ps.print(heading);
            printHeader(ps, str);

            logger.trace(tl.protocolCat, os.toString());
            str.pos(p);
        }
    }

    public static void
    traceRequest(String heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

	    java.io.OutputStream os = new java.io.ByteArrayOutputStream();
	    java.io.PrintStream ps = new java.io.PrintStream(os);
            ps.print(heading);
            printHeader(ps, str);

            int requestId = str.readInt();
            ps.print("\nrequest id = " + requestId);
            if(requestId == 0)
            {
                ps.print(" (oneway)");
            }

            printRequestHeader(ps, str);
            logger.trace(tl.protocolCat, os.toString());
            str.pos(p);
        }
    }

    public static void
    traceBatchRequest(String heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

	    java.io.OutputStream os = new java.io.ByteArrayOutputStream();
	    java.io.PrintStream ps = new java.io.PrintStream(os);
            ps.print(heading);
            printHeader(ps, str);

            int batchRequestNum = str.readInt();
 	    ps.print("\nnumber of requests = " + batchRequestNum);
		
	    for(int i = 0; i < batchRequestNum; ++i)
            {
                ps.print("\nrequest #" + i + ':');
                printRequestHeader(ps, str);
                str.skipEncaps();
            }

            logger.trace(tl.protocolCat, os.toString());
            str.pos(p);
        }
    }

    public static void
    traceReply(String heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

	    java.io.OutputStream os = new java.io.ByteArrayOutputStream();
	    java.io.PrintStream ps = new java.io.PrintStream(os);
            ps.print(heading);
            printHeader(ps, str);

            int requestId = str.readInt();
            ps.print("\nrequest id = " + requestId);

            byte replyStatus = str.readByte();
            ps.print("\nreply status = " + (int)replyStatus + ' ');

            switch(replyStatus)
            {
                case ReplyStatus.replyOK:
                {
                    ps.print("(ok)");
                    break;
                }

                case ReplyStatus.replyUserException:
                {
                    ps.print("(user exception)");
                    break;
                }

                case ReplyStatus.replyObjectNotExist:
                case ReplyStatus.replyFacetNotExist:
                case ReplyStatus.replyOperationNotExist:
                {
		    switch(replyStatus)
		    {
			case ReplyStatus.replyObjectNotExist:
			{
			    ps.print("(object not exist)");
			    break;
			}
			
			case ReplyStatus.replyFacetNotExist:
			{
			    ps.print("(facet not exist)");
			    break;
			}
			
			case ReplyStatus.replyOperationNotExist:
			{
			    ps.print("(operation not exist)");
			    break;
			}
			
			default:
			{
			    if(IceUtil.Debug.ASSERT)
			    {
				IceUtil.Debug.Assert(false);
			    }
			    break;
			}
		    }
		    
		    printIdentityFacetOperation(ps, str);
		    break;
                }

		case ReplyStatus.replyUnknownException:
		case ReplyStatus.replyUnknownLocalException:
		case ReplyStatus.replyUnknownUserException:
		{
		    switch(replyStatus)
		    {
			case ReplyStatus.replyUnknownException:
			{
			    ps.print("(unknown exception)");
			    break;
			}

			case ReplyStatus.replyUnknownLocalException:
			{
			    ps.print("(unknown local exception)");
			    break;
			}
			
			case ReplyStatus.replyUnknownUserException:
			{
			    ps.print("(unknown user exception)");
			    break;
			}
			
			default:
			{
			    if(IceUtil.Debug.ASSERT)
			    {
				IceUtil.Debug.Assert(false);
			    }
			    break;
			}
		    }

		    String unknown = str.readString();
		    ps.print("\nunknown = " + unknown);
		    break;
		}

                default:
                {
                    ps.print("(unknown)");
                    break;
                }
            }

            logger.trace(tl.protocolCat, os.toString());
            str.pos(p);
        }
    }

    public static void
    dumpStream(BasicStream stream)
    {
        final int inc = 8;

        int pos = stream.pos();
        stream.pos(0);

        byte[] data = stream.readBlob(stream.size());
        dumpOctets(data);

        stream.pos(pos);
    }

    public static void
    dumpOctets(byte[] data)
    {
        final int inc = 8;

        for(int i = 0; i < data.length; i += inc)
        {
            for(int j = i; j - i < inc; j++)
            {
                if(j < data.length)
                {
                    int n = (int)data[j];
                    if(n < 0)
                    {
                        n += 256;
                    }
                    String s;
                    if(n < 10)
                    {
                        s = "  " + n;
                    }
                    else if(n < 100)
                    {
                        s = " " + n;
                    }
                    else
                    {
                        s = "" + n;
                    }
                    System.out.print(s + " ");
                }
                else
                {
                    System.out.print("    ");
                }
            }

            System.out.print('"');

            for(int j = i; j < data.length && j - i < inc; j++)
            {
                if(data[j] >= (byte)32 && data[j] < (byte)127)
                {
                    System.out.print((char)data[j]);
                }
                else
                {
                    System.out.print('.');
                }
            }

            System.out.println('"');
        }
    }

    private static void
    printIdentityFacetOperation(java.io.PrintStream out, BasicStream stream)
    {
	Ice.Identity identity = new Ice.Identity();
	identity.__read(stream);
	out.print("\nidentity = " + stream.instance().identityToString(identity));

	String[] facet = stream.readStringSeq();
	out.print("\nfacet = ");
	if(facet.length > 0)
	{
	    out.print(IceUtil.StringUtil.escapeString(facet[0], ""));
	}

	String operation = stream.readString();
	out.print("\noperation = " + operation);
    }

    private static void
    printRequestHeader(java.io.PrintStream out, BasicStream stream)
    {
	printIdentityFacetOperation(out, stream);

	byte mode = stream.readByte();
	out.print("\nmode = " + (int)mode + ' ');
	switch(mode)
	{
	case Ice.OperationMode._Normal:
	{
	    out.print("(normal)");
	    break;
	}
		
	case Ice.OperationMode._Nonmutating:
	{
	    out.print("(nonmutating)");
	    break;
	}
		
	case Ice.OperationMode._Idempotent:
	{
	    out.print("(idempotent)");
	    break;
	}
		
	default:
	{
	    out.print("(unknown)");
	    break;
	}
	}

	int sz = stream.readSize();
	out.print("\ncontext = ");
	while(sz-- > 0)
	{
	    String key = stream.readString();
	    String value = stream.readString();
	    out.print(key + '/'+ value);
	    if(sz > 0)
	    {
		out.print(", ");
	    }
	}
    }

    private static void
    printHeader(java.io.PrintStream out, BasicStream stream)
    {
	byte magic;
	magic = stream.readByte();	// Don't bother printing the magic number
	magic = stream.readByte();
	magic = stream.readByte();
	magic = stream.readByte();

	byte pMajor = stream.readByte();
	byte pMinor = stream.readByte();
//            out.print("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);

	byte eMajor = stream.readByte();
	byte eMinor = stream.readByte();
//            out.print("\nencoding version = " + (int)eMajor + "." + (int)eMinor);

	byte type = stream.readByte();
	out.print("\nmessage type = " + (int)type + ' ');
	switch(type)
	{
	case Protocol.requestMsg:
	{
	    out.print("(request)");
	    break;
	}

	case Protocol.requestBatchMsg:
	{
	    out.print("(batch request)");
	    break;
	}

	case Protocol.replyMsg:
	{
	    out.print("(reply)");
	    break;
	}

	case Protocol.closeConnectionMsg:
	{
	    out.print("(close connection)");
	    break;
	}

	case Protocol.validateConnectionMsg:
	{
	    out.print("(validate connection)");
	    break;
	}

	default:
	{
	    out.print("(unknown)");
	    break;
	}
	}

	byte compress = stream.readByte();
	out.print("\ncompression status = " + (int)compress + ' ');
	switch(compress)
	{
	case (byte)0:
	{
	    out.print("(not compressed; do not compress response, if any)");
	    break;
	}

	case (byte)1:
	{
	    out.print("(not compressed; compress response, if any)");
	    break;
	}

	case (byte)2:
	{
	    out.print("(compressed; compress response, if any)");
	    break;
	}

	default:
	{
	    out.print("(unknown)");
	    break;
	}
	}

	int size = stream.readInt();
	out.print("\nmessage size = " + size);
    }
}
