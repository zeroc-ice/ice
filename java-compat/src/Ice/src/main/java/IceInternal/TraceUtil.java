// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public final class TraceUtil
{
    public static void
    traceSend(Ice.OutputStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            Ice.InputStream is = new Ice.InputStream(str.instance(), str.getEncoding(), str.getBuffer(), false);
            is.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            byte type = printMessage(s, is);

            logger.trace(tl.protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.toString());

            str.pos(p);
        }
    }

    public static void
    traceRecv(Ice.InputStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            byte type = printMessage(s, str);

            logger.trace(tl.protocolCat, "received " + getMessageTypeAsString(type) + " " + s.toString());

            str.pos(p);
        }
    }

    public static void
    trace(String heading, Ice.OutputStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            Ice.InputStream is = new Ice.InputStream(str.instance(), str.getEncoding(), str.getBuffer(), false);
            is.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printMessage(s, is);

            logger.trace(tl.protocolCat, s.toString());
            str.pos(p);
        }
    }

    public static void
    trace(String heading, Ice.InputStream str, Ice.Logger logger, TraceLevels tl)
    {
        if(tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printMessage(s, str);

            logger.trace(tl.protocolCat, s.toString());
            str.pos(p);
        }
    }

    private static java.util.Set<String> slicingIds = new java.util.HashSet<String>();

    public synchronized static void
    traceSlicing(String kind, String typeId, String slicingCat, Ice.Logger logger)
    {
        if(slicingIds.add(typeId))
        {
            java.io.StringWriter s = new java.io.StringWriter();
            s.write("unknown " + kind + " type `" + typeId + "'");
            logger.trace(slicingCat, s.toString());
        }
    }

    public static void
    dumpStream(Ice.InputStream stream)
    {
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
                    int n = data[j];
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
    printIdentityFacetOperation(java.io.Writer out, Ice.InputStream stream)
    {
        try
        {
            Ice.ToStringMode toStringMode = Ice.ToStringMode.Unicode;
            if(stream.instance() != null)
            {
                toStringMode = stream.instance().toStringMode();
            }

            Ice.Identity identity = new Ice.Identity();
            identity.ice_readMembers(stream);
            out.write("\nidentity = " + Ice.Util.identityToString(identity, toStringMode));

            String[] facet = stream.readStringSeq();
            out.write("\nfacet = ");
            if(facet.length > 0)
            {
                out.write(IceUtilInternal.StringUtil.escapeString(facet[0], "", toStringMode));
            }

            String operation = stream.readString();
            out.write("\noperation = " + operation);
        }
        catch(java.io.IOException ex)
        {
            assert(false);
        }
    }

    private static void
    printRequest(java.io.StringWriter s, Ice.InputStream str)
    {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);
        if(requestId == 0)
        {
            s.write(" (oneway)");
        }

        printRequestHeader(s, str);
    }

    private static void
    printBatchRequest(java.io.StringWriter s, Ice.InputStream str)
    {
        int batchRequestNum = str.readInt();
        s.write("\nnumber of requests = " + batchRequestNum);

        for(int i = 0; i < batchRequestNum; ++i)
        {
            s.write("\nrequest #" + i + ':');
            printRequestHeader(s, str);
        }
    }

    private static void
    printReply(java.io.StringWriter s, Ice.InputStream str)
    {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);

        byte replyStatus = str.readByte();
        s.write("\nreply status = " + (int)replyStatus + ' ');

        switch(replyStatus)
        {
        case ReplyStatus.replyOK:
        {
            s.write("(ok)");
            break;
        }

        case ReplyStatus.replyUserException:
        {
            s.write("(user exception)");
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
                s.write("(object not exist)");
                break;
            }

            case ReplyStatus.replyFacetNotExist:
            {
                s.write("(facet not exist)");
                break;
            }

            case ReplyStatus.replyOperationNotExist:
            {
                s.write("(operation not exist)");
                break;
            }

            default:
            {
                assert(false);
                break;
            }
            }

            printIdentityFacetOperation(s, str);
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
                s.write("(unknown exception)");
                break;
            }

            case ReplyStatus.replyUnknownLocalException:
            {
                s.write("(unknown local exception)");
                break;
            }

            case ReplyStatus.replyUnknownUserException:
            {
                s.write("(unknown user exception)");
                break;
            }

            default:
            {
                assert(false);
                break;
            }
            }

            String unknown = str.readString();
            s.write("\nunknown = " + unknown);
            break;
        }

        default:
        {
            s.write("(unknown)");
            break;
        }
        }

        if(replyStatus == ReplyStatus.replyOK || replyStatus == ReplyStatus.replyUserException)
        {
            Ice.EncodingVersion v = str.skipEncapsulation();
            if(!v.equals(Ice.Util.Encoding_1_0))
            {
                s.write("\nencoding = ");
                s.write(Ice.Util.encodingVersionToString(v));
            }
        }
    }

    private static void
    printRequestHeader(java.io.Writer out, Ice.InputStream stream)
    {
        printIdentityFacetOperation(out, stream);

        try
        {
            byte mode = stream.readByte();
            out.write("\nmode = " + (int) mode + ' ');
            switch(Ice.OperationMode.values()[mode])
            {
                case Normal:
                {
                    out.write("(normal)");
                    break;
                }

                case Nonmutating:
                {
                    out.write("(nonmutating)");
                    break;
                }

                case Idempotent:
                {
                    out.write("(idempotent)");
                    break;
                }

                default:
                {
                    out.write("(unknown)");
                    break;
                }
            }

            int sz = stream.readSize();
            out.write("\ncontext = ");
            while(sz-- > 0)
            {
                String key = stream.readString();
                String value = stream.readString();
                out.write(key + '/' + value);
                if(sz > 0)
                {
                    out.write(", ");
                }
            }

            Ice.EncodingVersion v = stream.skipEncapsulation();
            if(!v.equals(Ice.Util.Encoding_1_0))
            {
                out.write("\nencoding = ");
                out.write(Ice.Util.encodingVersionToString(v));
            }
        }
        catch(java.io.IOException ex)
        {
            assert(false);
        }
    }

    private static byte
    printHeader(java.io.Writer out, Ice.InputStream stream)
    {
        stream.readByte();  // Don't bother printing the magic number
        stream.readByte();
        stream.readByte();
        stream.readByte();

//        byte pMajor = stream.readByte();
//        byte pMinor = stream.readByte();
//        out.write("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);
        stream.readByte(); // major
        stream.readByte(); // minor

//        byte eMajor = stream.readByte();
//        byte eMinor = stream.readByte();
//        out.write("\nencoding version = " + (int)eMajor + "." + (int)eMinor);
        stream.readByte(); // major
        stream.readByte(); // minor

        byte type = stream.readByte();

        try
        {
            out.write("\nmessage type = " + (int)type + " (" + getMessageTypeAsString(type) + ')');
            byte compress = stream.readByte();
            out.write("\ncompression status = " + (int)compress + ' ');
            switch(compress)
            {
                case (byte)0:
                {
                    out.write("(not compressed; do not compress response, if any)");
                    break;
                }

                case (byte)1:
                {
                    out.write("(not compressed; compress response, if any)");
                    break;
                }

                case (byte)2:
                {
                    out.write("(compressed; compress response, if any)");
                    break;
                }

                default:
                {
                    out.write("(unknown)");
                    break;
                }
            }

            int size = stream.readInt();
            out.write("\nmessage size = " + size);
            return type;
        }
        catch(java.io.IOException ex)
        {
            assert(false);
            return 0;
        }
    }

    static private byte
    printMessage(java.io.StringWriter s, Ice.InputStream str)
    {
        byte type = printHeader(s, str);

        switch(type)
        {
        case Protocol.closeConnectionMsg:
        case Protocol.validateConnectionMsg:
        {
            // We're done.
            break;
        }

        case Protocol.requestMsg:
        {
            printRequest(s, str);
            break;
        }

        case Protocol.requestBatchMsg:
        {
            printBatchRequest(s, str);
            break;
        }

        case Protocol.replyMsg:
        {
            printReply(s, str);
            break;
        }

        default:
        {
            break;
        }
        }

        return type;
    }

    static private String
    getMessageTypeAsString(byte type)
    {
        switch(type)
        {
        case Protocol.requestMsg:
            return "request";
        case Protocol.requestBatchMsg:
            return "batch request";
        case Protocol.replyMsg:
            return "reply";
        case Protocol.closeConnectionMsg:
            return "close connection";
        case Protocol.validateConnectionMsg:
            return  "validate connection";
        default:
            return "unknown";
        }
    }
}
