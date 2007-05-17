// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, str);

            logger.trace(tl.protocolCat, s.toString());
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

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, str);

            int requestId = str.readInt();
            s.write("\nrequest id = " + requestId);
            if(requestId == 0)
            {
                s.write(" (oneway)");
            }

            printRequestHeader(s, str);

            logger.trace(tl.protocolCat, s.toString());
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

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, str);

            int batchRequestNum = str.readInt();
            s.write("\nnumber of requests = " + batchRequestNum);
                
            for(int i = 0; i < batchRequestNum; ++i)
            {
                s.write("\nrequest #" + i + ':');
                printRequestHeader(s, str);
                str.skipEncaps();
            }

            logger.trace(tl.protocolCat, s.toString());
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

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, str);

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

            logger.trace(tl.protocolCat, s.toString());
            str.pos(p);
        }
    }

    private static java.util.Set slicingIds = new java.util.HashSet();

    synchronized static void
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
    printIdentityFacetOperation(java.io.Writer out, BasicStream stream)
    {
        try
        {
            Ice.Identity identity = new Ice.Identity();
            identity.__read(stream);
            out.write("\nidentity = " + stream.instance().identityToString(identity));

            String[] facet = stream.readStringSeq();
            out.write("\nfacet = ");
            if(facet.length > 0)
            {
                out.write(IceUtil.StringUtil.escapeString(facet[0], ""));
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
    printRequestHeader(java.io.Writer out, BasicStream stream)
    {
        printIdentityFacetOperation(out, stream);

        try
        {
            byte mode = stream.readByte();
            out.write("\nmode = " + (int)mode + ' ');
            switch(mode)
            {
                case Ice.OperationMode._Normal:
                {
                    out.write("(normal)");
                    break;
                }
                
                case Ice.OperationMode._Nonmutating:
                {
                    out.write("(nonmutating)");
                    break;
                }
                
                case Ice.OperationMode._Idempotent:
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
                out.write(key + '/'+ value);
                if(sz > 0)
                {
                    out.write(", ");
                }
            }
        }
        catch(java.io.IOException ex)
        {
            assert(false);
        }
    }

    private static void
    printHeader(java.io.Writer out, BasicStream stream)
    {
        try
        {
            byte magic;
            magic = stream.readByte();  // Don't bother printing the magic number
            magic = stream.readByte();
            magic = stream.readByte();
            magic = stream.readByte();

            byte pMajor = stream.readByte();
            byte pMinor = stream.readByte();
//            out.write("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);

            byte eMajor = stream.readByte();
            byte eMinor = stream.readByte();
//            out.write("\nencoding version = " + (int)eMajor + "." + (int)eMinor);

            byte type = stream.readByte();
            out.write("\nmessage type = " + (int)type + ' ');
            switch(type)
            {
                case Protocol.requestMsg:
                {
                    out.write("(request)");
                    break;
                }

                case Protocol.requestBatchMsg:
                {
                    out.write("(batch request)");
                    break;
                }

                case Protocol.replyMsg:
                {
                    out.write("(reply)");
                    break;
                }

                case Protocol.closeConnectionMsg:
                {
                    out.write("(close connection)");
                    break;
                }

                case Protocol.validateConnectionMsg:
                {
                    out.write("(validate connection)");
                    break;
                }

                default:
                {
                    out.write("(unknown)");
                    break;
                }
            }

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
        }
        catch(java.io.IOException ex)
        {
            assert(false);
        }
    }
}
