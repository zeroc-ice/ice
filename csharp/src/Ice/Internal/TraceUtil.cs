// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;

namespace Ice.Internal;

internal sealed class TraceUtil
{
    internal static void traceSend(
        OutputStream str,
        Instance instance,
        ConnectionI connection,
        Logger logger,
        TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            int p = str.pos();
            var iss = new Ice.InputStream(instance, str.getEncoding(), str.getBuffer(), false);
            iss.pos(0);

            using (var s = new System.IO.StringWriter(CultureInfo.CurrentCulture))
            {
                byte type = printMessage(s, iss, connection);

                logger.trace(tl.protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.ToString());
            }
            str.pos(p);
        }
    }

    internal static void traceRecv(Ice.InputStream str, ConnectionI connection, Ice.Logger logger, TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

            using (var s = new System.IO.StringWriter(CultureInfo.CurrentCulture))
            {
                byte type = printMessage(s, str, connection);

                logger.trace(tl.protocolCat, "received " + getMessageTypeAsString(type) + " " + s.ToString());
            }
            str.pos(p);
        }
    }

    internal static void trace(string heading, InputStream str, ConnectionI connection, Logger logger, TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            int p = str.pos();
            str.pos(0);

            using (var s = new StringWriter(CultureInfo.CurrentCulture))
            {
                s.Write(heading);
                printMessage(s, str, connection);

                logger.trace(tl.protocolCat, s.ToString());
            }
            str.pos(p);
        }
    }

    private static readonly HashSet<string> slicingIds = new();

    internal static void traceSlicing(string kind, string typeId, string slicingCat, Ice.Logger logger)
    {
        lock (_globalMutex)
        {
            if (slicingIds.Add(typeId))
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write("unknown " + kind + " type `" + typeId + "'");
                logger.trace(slicingCat, s.ToString());
            }
        }
    }

    private static void printIdentityFacetOperation(System.IO.StringWriter s, Ice.InputStream str)
    {
        try
        {
            Ice.ToStringMode toStringMode = str.instance().toStringMode();

            var identity = new Ice.Identity(str);
            s.Write("\nidentity = " + Ice.Util.identityToString(identity, toStringMode));

            string[] facet = str.readStringSeq();
            s.Write("\nfacet = ");
            if (facet.Length > 0)
            {
                s.Write(Ice.UtilInternal.StringUtil.escapeString(facet[0], "", toStringMode));
            }

            string operation = str.readString();
            s.Write("\noperation = " + operation);
        }
        catch (System.IO.IOException)
        {
            Debug.Assert(false);
        }
    }

    private static void printRequest(System.IO.StringWriter s, Ice.InputStream str)
    {
        int requestId = str.readInt();
        s.Write("\nrequest id = " + requestId);
        if (requestId == 0)
        {
            s.Write(" (oneway)");
        }

        printRequestHeader(s, str);
    }

    private static void printBatchRequest(System.IO.StringWriter s, Ice.InputStream str)
    {
        int batchRequestNum = str.readInt();
        s.Write("\nnumber of requests = " + batchRequestNum);

        for (int i = 0; i < batchRequestNum; ++i)
        {
            s.Write("\nrequest #" + i + ':');
            printRequestHeader(s, str);
        }
    }

    private static void printReply(System.IO.StringWriter s, Ice.InputStream str)
    {
        int requestId = str.readInt();
        s.Write("\nrequest id = " + requestId);

        // We can't (shouldn't) use the generated code to unmarshal a possibly unknown reply status.
        var replyStatus = (ReplyStatus)str.readByte();
        s.Write("\nreply status = ");
        s.Write(replyStatus);

        switch (replyStatus)
        {
            case ReplyStatus.Ok:
            case ReplyStatus.UserException:
                EncodingVersion v = str.skipEncapsulation();
                if (v != Ice.Util.Encoding_1_0)
                {
                    s.Write("\nencoding = ");
                    s.Write(Ice.Util.encodingVersionToString(v));
                }
                break;

            case ReplyStatus.ObjectNotExist:
            case ReplyStatus.FacetNotExist:
            case ReplyStatus.OperationNotExist:
                printIdentityFacetOperation(s, str);
                break;

            default:
                string message = str.readString();
                s.Write("\nmessage = ");
                s.Write(message);
                break;
        }
    }

    private static void printRequestHeader(System.IO.StringWriter s, Ice.InputStream str)
    {
        printIdentityFacetOperation(s, str);

        try
        {
            byte mode = str.readByte();
            s.Write("\nmode = " + ((OperationMode)mode).ToString());

            int sz = str.readSize();
            s.Write("\ncontext = ");
            while (sz-- > 0)
            {
                string key = str.readString();
                string val = str.readString();
                s.Write(key + '/' + val);
                if (sz > 0)
                {
                    s.Write(", ");
                }
            }

            Ice.EncodingVersion v = str.skipEncapsulation();
            if (!v.Equals(Ice.Util.Encoding_1_0))
            {
                s.Write("\nencoding = ");
                s.Write(Ice.Util.encodingVersionToString(v));
            }
        }
        catch (System.IO.IOException)
        {
            Debug.Assert(false);
        }
    }

    private static byte printHeader(System.IO.StringWriter s, Ice.InputStream str)
    {
        try
        {
            str.readByte(); // Don't bother printing the magic number
            str.readByte();
            str.readByte();
            str.readByte();

            /* byte pMajor = */
            str.readByte();
            /* byte pMinor = */
            str.readByte();
            // s.Write("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);

            /* byte eMajor = */
            str.readByte();
            /* byte eMinor = */
            str.readByte();
            // s.Write("\nencoding version = " + (int)eMajor + "." + (int)eMinor);

            byte type = str.readByte();
            s.Write("\nmessage type = " + (int)type + " (" + getMessageTypeAsString(type) + ')');

            byte compress = str.readByte();
            s.Write("\ncompression status = " + (int)compress + ' ');
            switch (compress)
            {
                case 0:
                {
                    s.Write("(not compressed; do not compress response, if any)");
                    break;
                }

                case 1:
                {
                    s.Write("(not compressed; compress response, if any)");
                    break;
                }

                case 2:
                {
                    s.Write("(compressed; compress response, if any)");
                    break;
                }

                default:
                {
                    s.Write("(unknown)");
                    break;
                }
            }

            int size = str.readInt();
            s.Write("\nmessage size = " + size);
            return type;
        }
        catch (System.IO.IOException)
        {
            Debug.Assert(false);
            return 0;
        }
    }

    private static byte printMessage(System.IO.StringWriter s, Ice.InputStream str, ConnectionI connection)
    {
        byte type = printHeader(s, str);

        switch (type)
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
                s.Write("(unknown)");
                break;
            }
        }

        if (connection is not null)
        {
            s.Write("\ntransport = " + connection.type() + "\n");
            string connectionId = connection.endpoint().connectionId();
            if (connectionId.Length > 0)
            {
                s.Write("connection ID = " + connectionId + "\n");
            }
            s.Write(connection.ToString());
        }
        else
        {
            s.Write("\ncollocated = true");
        }

        return type;
    }

    private static string getMessageTypeAsString(byte type)
    {
        return type switch
        {
            Protocol.requestMsg => "request",
            Protocol.requestBatchMsg => "batch request",
            Protocol.replyMsg => "reply",
            Protocol.closeConnectionMsg => "close connection",
            Protocol.validateConnectionMsg => "validate connection",
            _ => "unknown",
        };
    }

    private static readonly object _globalMutex = new object();
}
