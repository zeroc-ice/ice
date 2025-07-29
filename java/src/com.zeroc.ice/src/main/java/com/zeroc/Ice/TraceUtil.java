// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.HashSet;
import java.util.Set;

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

            StringWriter s = new StringWriter();
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

            StringWriter s = new StringWriter();
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

            StringWriter s = new StringWriter();
            s.write(heading);
            printMessage(s, str, connection);

            logger.trace(tl.protocolCat, s.toString());
            str.pos(p);
        }
    }

    private static final Set<String> slicingIds = new HashSet<>();

    public static synchronized void traceSlicing(
            String kind, String typeId, String slicingCat, Logger logger) {
        if (slicingIds.add(typeId)) {
            StringWriter s = new StringWriter();
            s.write("unknown " + kind + " type `" + typeId + "'");
            logger.trace(slicingCat, s.toString());
        }
    }

    private static void printIdentityFacetOperation(Writer out, InputStream stream) {
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
        } catch (IOException ex) {
            assert false;
        }
    }

    private static void printRequest(StringWriter s, InputStream str) {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);
        if (requestId == 0) {
            s.write(" (oneway)");
        }

        printRequestHeader(s, str);
    }

    private static void printBatchRequest(StringWriter s, InputStream str) {
        int batchRequestNum = str.readInt();
        s.write("\nnumber of requests = " + batchRequestNum);

        for (int i = 0; i < batchRequestNum; i++) {
            s.write("\nrequest #" + i + ':');
            printRequestHeader(s, str);
        }
    }

    private static void printReply(StringWriter s, InputStream str) {
        int requestId = str.readInt();
        s.write("\nrequest id = " + requestId);

        s.write("\nreply status = ");
        // convert the signed byte into a positive int
        int replyStatusInt = str.readByte() & 0xFF;
        var replyStatus = ReplyStatus.valueOf(replyStatusInt);
        if (replyStatus != null) {
            s.write(replyStatus.toString());

            switch (replyStatus) {
                case Ok:
                case UserException:
                    EncodingVersion v = str.skipEncapsulation();
                    if (!v.equals(Util.Encoding_1_0)) {
                        s.write("\nencoding = ");
                        s.write(Util.encodingVersionToString(v));
                    }
                    break;

                case ObjectNotExist:
                case FacetNotExist:
                case OperationNotExist:
                    printIdentityFacetOperation(s, str);
                    break;

                default:
                    s.write("\nmessage = " + str.readString());
                    break;
            }
        } else {
            s.write(Integer.toString(replyStatusInt));
            // Same as default case above:
            s.write("\nmessage = " + str.readString());
        }
    }

    private static void printRequestHeader(Writer out, InputStream stream) {
        printIdentityFacetOperation(out, stream);

        try {
            byte mode = stream.readByte();
            out.write("\nmode = " + (int) mode + ' ');
            switch (OperationMode.values()[mode]) {
                case Normal -> out.write("(normal)");
                case Nonmutating -> out.write("(nonmutating)");
                case Idempotent -> out.write("(idempotent)");
                default -> out.write("(unknown)");
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
        } catch (IOException ex) {
            assert false;
        }
    }

    private static byte printHeader(Writer out, InputStream stream) {
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
                case (byte) 0 -> out.write("(not compressed; do not compress response, if any)");
                case (byte) 1 -> out.write("(not compressed; compress response, if any)");
                case (byte) 2 -> out.write("(compressed; compress response, if any)");
                default -> out.write("(unknown)");
            }

            int size = stream.readInt();
            out.write("\nmessage size = " + size);
            return type;
        } catch (IOException ex) {
            assert false;
            return 0;
        }
    }

    private static byte printMessage(StringWriter s, InputStream str, ConnectionI connection) {
        byte type = printHeader(s, str);

        switch (type) {
            case Protocol.closeConnectionMsg, Protocol.validateConnectionMsg -> { /* we're done */ }
            case Protocol.requestMsg -> printRequest(s, str);
            case Protocol.requestBatchMsg -> printBatchRequest(s, str);
            case Protocol.replyMsg -> printReply(s, str);
            default -> {}
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
        return switch (type) {
            case Protocol.requestMsg -> "request";
            case Protocol.requestBatchMsg -> "batch request";
            case Protocol.replyMsg -> "reply";
            case Protocol.closeConnectionMsg -> "close connection";
            case Protocol.validateConnectionMsg -> "validate connection";
            default -> "unknown";
        };
    }

    private TraceUtil() {}
}
