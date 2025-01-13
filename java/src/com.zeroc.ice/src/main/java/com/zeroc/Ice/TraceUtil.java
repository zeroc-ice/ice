//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class TraceUtil {
    public static void traceSend(
            OutputStream str,
            Instance instance,
            ConnectionI connection,
            Logger logger,
            TraceLevels tl) {
        if (tl.protocol >= 1) {
            int p = str.pos();
            var is = new InputStream(instance, str.getEncoding(), str.getBuffer(), false);
            is.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            byte type = printMessage(s, is, connection);

            logger.trace(
                    tl.protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.toString());

            str.pos(p);
        }
    }

    public static void traceRecv(
            InputStream str, ConnectionI connection, Logger logger, TraceLevels tl) {
        if (tl.protocol >= 1) {
            int p = str.pos();
            str.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            byte type = printMessage(s, str, connection);

            logger.trace(
                    tl.protocolCat,
                    "received " + getMessageTypeAsString(type) + " " + s.toString());

            str.pos(p);
        }
    }

    public static void trace(
            String heading,
            InputStream str,
            ConnectionI connection,
            Logger logger,
            TraceLevels tl) {
        if (tl.protocol >= 1) {
            int p = str.pos();
            str.pos(0);

            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printMessage(s, str, connection);

            logger.trace(tl.protocolCat, s.toString());
            str.pos(p);
        }
    }

    private static java.util.Set<String> slicingIds = new java.util.HashSet<>();

    public static synchronized void traceSlicing(
            String kind, String typeId, String slicingCat, Logger logger) {
        if (slicingIds.add(typeId)) {
            java.io.StringWriter s = new java.io.StringWriter();
            s.write("unknown " + kind + " type `" + typeId + "'");
            logger.trace(slicingCat, s.toString());
        }
    }

    private static void printIdentityFacetOperation(java.io.Writer out, InputStream stream) {
        try {
            ToStringMode toStringMode = stream.instance().toStringMode();

            Identity identity = Identity.ice_read(stream);
            out.write("\nidentity = " + Util.identityToString(identity, toStringMode));

            String[] facet = stream.readStringSeq();
            out.write("\nfacet = ");
            if (facet.length > 0) {
                out.write(StringUtil.escapeString(facet[0], "", toStringMode));
            }

            String operation = stream.readString();
            out.write("\noperation = " + operation);
        } catch (java.io.IOException ex) {
            assert (false);
        }
    }

    private static void printRequest(java.io.StringWriter s, InputStream str) {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);
        if (requestId == 0) {
            s.write(" (oneway)");
        }

        printRequestHeader(s, str);
    }

    private static void printBatchRequest(java.io.StringWriter s, InputStream str) {
        int batchRequestNum = str.readInt();
        s.write("\nnumber of requests = " + batchRequestNum);

        for (int i = 0; i < batchRequestNum; ++i) {
            s.write("\nrequest #" + i + ':');
            printRequestHeader(s, str);
        }
    }

    private static void printReply(java.io.StringWriter s, InputStream str) {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);

        ReplyStatus replyStatus = ReplyStatus.valueOf(str.readByte());
        s.write("\nreply status = " + (int) replyStatus.value() + ' ');

        switch (replyStatus) {
            case Ok:
                s.write("(ok)");
                break;

            case UserException:
                s.write("(user exception)");
                break;

            case ObjectNotExist:
            case FacetNotExist:
            case OperationNotExist:
                switch (replyStatus) {
                    case ObjectNotExist:
                        s.write("(object not exist)");
                        break;

                    case FacetNotExist:
                        s.write("(facet not exist)");
                        break;

                    case OperationNotExist:
                        s.write("(operation not exist)");
                        break;

                    default:
                        assert (false);
                        break;
                }

                printIdentityFacetOperation(s, str);
                break;

            case UnknownException:
            case UnknownLocalException:
            case UnknownUserException:
                switch (replyStatus) {
                    case UnknownException:
                        s.write("(unknown exception)");
                        break;

                    case UnknownLocalException:
                        s.write("(unknown local exception)");
                        break;

                    case UnknownUserException:
                        s.write("(unknown user exception)");
                        break;

                    default:
                        assert (false);
                        break;
                }

                String unknown = str.readString();
                s.write("\nunknown = " + unknown);
                break;

            default:
                s.write("(unknown)");
                break;
        }

        if (replyStatus == ReplyStatus.Ok || replyStatus == ReplyStatus.UserException) {
            EncodingVersion v = str.skipEncapsulation();
            if (!v.equals(Util.Encoding_1_0)) {
                s.write("\nencoding = ");
                s.write(Util.encodingVersionToString(v));
            }
        }
    }

    private static void printRequestHeader(java.io.Writer out, InputStream stream) {
        printIdentityFacetOperation(out, stream);

        try {
            byte mode = stream.readByte();
            out.write("\nmode = " + (int) mode + ' ');
            switch (OperationMode.values()[mode]) {
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
            while (sz-- > 0) {
                String key = stream.readString();
                String value = stream.readString();
                out.write(key + '/' + value);
                if (sz > 0) {
                    out.write(", ");
                }
            }

            EncodingVersion v = stream.skipEncapsulation();
            if (!v.equals(Util.Encoding_1_0)) {
                out.write("\nencoding = ");
                out.write(Util.encodingVersionToString(v));
            }
        } catch (java.io.IOException ex) {
            assert (false);
        }
    }

    private static byte printHeader(java.io.Writer out, InputStream stream) {
        stream.readByte(); // Don't bother printing the magic number
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

        try {
            out.write("\nmessage type = " + (int) type + " (" + getMessageTypeAsString(type) + ')');
            byte compress = stream.readByte();
            out.write("\ncompression status = " + (int) compress + ' ');
            switch (compress) {
                case (byte) 0:
                    {
                        out.write("(not compressed; do not compress response, if any)");
                        break;
                    }

                case (byte) 1:
                    {
                        out.write("(not compressed; compress response, if any)");
                        break;
                    }

                case (byte) 2:
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
        } catch (java.io.IOException ex) {
            assert (false);
            return 0;
        }
    }

    private static byte printMessage(
            java.io.StringWriter s, InputStream str, ConnectionI connection) {
        byte type = printHeader(s, str);

        switch (type) {
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

        if (connection != null) {
            s.write("\ntransport = " + connection.type() + "\n");
            String connectionId = connection.endpoint().connectionId();
            if (!connectionId.isEmpty()) {
                s.write("connection ID = " + connectionId + "\n");
            }
            s.write(connection.toString());
        } else {
            s.write("\ncollocated = true");
        }

        return type;
    }

    private static String getMessageTypeAsString(byte type) {
        switch (type) {
            case Protocol.requestMsg:
                return "request";
            case Protocol.requestBatchMsg:
                return "batch request";
            case Protocol.replyMsg:
                return "reply";
            case Protocol.closeConnectionMsg:
                return "close connection";
            case Protocol.validateConnectionMsg:
                return "validate connection";
            default:
                return "unknown";
        }
    }
}
