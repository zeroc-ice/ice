//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

namespace IceInternal
{
    internal static class TraceUtil
    {
        internal static void TraceSend(Communicator communicator,
            byte[] buffer, ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                var iss = new InputStream(communicator, buffer);
                iss.Pos = 0;

                using var s = new System.IO.StringWriter(CultureInfo.CurrentCulture);
                Ice1Definitions.MessageType type = PrintMessage(s, iss);

                logger.Trace(tl.ProtocolCat, "sending " + GetMessageTypeAsString(type) + " " + s.ToString());
            }
        }

        internal static void TraceRecv(Ice.InputStream str, Ice.ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                int p = str.Pos;
                str.Pos = 0;

                using (var s = new System.IO.StringWriter(CultureInfo.CurrentCulture))
                {
                    Ice1Definitions.MessageType type = PrintMessage(s, str);

                    logger.Trace(tl.ProtocolCat, "received " + GetMessageTypeAsString(type) + " " + s.ToString());
                }
                str.Pos = p;
            }
        }

        internal static void Trace(string heading, Ice.InputStream str, Ice.ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                int p = str.Pos;
                str.Pos = 0;

                using (var s = new System.IO.StringWriter(CultureInfo.CurrentCulture))
                {
                    s.Write(heading);
                    PrintMessage(s, str);

                    logger.Trace(tl.ProtocolCat, s.ToString());
                }
                str.Pos = p;
            }
        }

        private static readonly HashSet<string> _slicingIds = new HashSet<string>();

        internal static void TraceSlicing(string kind, string typeId, string slicingCat, Ice.ILogger logger)
        {
            lock (_mutex)
            {
                if (_slicingIds.Add(typeId))
                {
                    using var s = new System.IO.StringWriter(CultureInfo.CurrentCulture);
                    s.Write("unknown " + kind + " type `" + typeId + "'");
                    logger.Trace(slicingCat, s.ToString());
                }
            }
        }

        private static void PrintIdentityFacetOperation(System.IO.StringWriter s, Ice.InputStream str)
        {
            try
            {
                Ice.ToStringMode toStringMode = str.Communicator.ToStringMode;

                var identity = new Ice.Identity(str);
                s.Write("\nidentity = " + identity.ToString(toStringMode));

                string facet = str.ReadFacet();
                s.Write("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Write(IceUtilInternal.StringUtil.EscapeString(facet, "", toStringMode));
                }

                string operation = str.ReadString();
                s.Write("\noperation = " + operation);
            }
            catch (System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }

        private static void PrintRequest(System.IO.StringWriter s, Ice.InputStream str)
        {
            int requestId = str.ReadInt();
            s.Write("\nrequest id = " + requestId);
            if (requestId == 0)
            {
                s.Write(" (oneway)");
            }

            PrintRequestHeader(s, str);
        }

        private static void PrintBatchRequest(System.IO.StringWriter s, Ice.InputStream str)
        {
            int batchRequestNum = str.ReadInt();
            s.Write("\nnumber of requests = " + batchRequestNum);

            for (int i = 0; i < batchRequestNum; ++i)
            {
                s.Write("\nrequest #" + i + ':');
                PrintRequestHeader(s, str);
            }
        }

        private static void PrintReply(System.IO.StringWriter s, Ice.InputStream str)
        {
            int requestId = str.ReadInt();
            s.Write("\nrequest id = " + requestId);

            var replyStatus = (ReplyStatus)str.ReadByte();
            s.Write($"\nreply status = {replyStatus}");

            if (replyStatus == ReplyStatus.OK || replyStatus == ReplyStatus.UserException)
            {
                _ = str.SkipEncapsulation();
                s.Write("\nencoding = ");
            }
        }

        private static void PrintRequestHeader(System.IO.StringWriter s, Ice.InputStream str)
        {
            PrintIdentityFacetOperation(s, str);

            try
            {
                byte mode = str.ReadByte();
                s.Write("\noperation mode = " + (int)mode + ' ');
                switch (mode)
                {
                    case 0:
                        {
                            s.Write("(non-idempotent)");
                            break;
                        }

                    case 1:
                        {
                            s.Write("(idempotent/nonmutating)");
                            break;
                        }

                    case 2:
                        {
                            s.Write("(idempotent)");
                            break;
                        }

                    default:
                        {
                            s.Write("(unknown)");
                            break;
                        }
                }

                int sz = str.ReadSize();
                s.Write("\ncontext = ");
                while (sz-- > 0)
                {
                    string key = str.ReadString();
                    string val = str.ReadString();
                    s.Write(key + '/' + val);
                    if (sz > 0)
                    {
                        s.Write(", ");
                    }
                }

                Ice.Encoding v = str.SkipEncapsulation();
                s.Write("\nencoding = ");
                s.Write(v.ToString());
            }
            catch (System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }

        private static Ice1Definitions.MessageType PrintHeader(System.IO.StringWriter s, Ice.InputStream str)
        {
            try
            {
                str.ReadByte(); // Don't bother printing the magic number
                str.ReadByte();
                str.ReadByte();
                str.ReadByte();

                /* byte pMajor = */
                str.ReadByte();
                /* byte pMinor = */
                str.ReadByte();
                //s.Write("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);

                /* byte eMajor = */
                str.ReadByte();
                /* byte eMinor = */
                str.ReadByte();
                //s.Write("\nencoding version = " + (int)eMajor + "." + (int)eMinor);

                var type = (Ice1Definitions.MessageType)str.ReadByte();
                s.Write("\nmessage type = " + (int)type + " (" + GetMessageTypeAsString(type) + ')');

                byte compress = str.ReadByte();
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

                int size = str.ReadInt();
                s.Write("\nmessage size = " + size);
                return type;
            }
            catch (System.IO.IOException)
            {
                Debug.Assert(false);
                return 0;
            }
        }

        private static Ice1Definitions.MessageType PrintMessage(System.IO.StringWriter s, InputStream str)
        {
            var type = (Ice1Definitions.MessageType) PrintHeader(s, str);

            switch (type)
            {
                case Ice1Definitions.MessageType.CloseConnectionMessage:
                case Ice1Definitions.MessageType.ValidateConnectionMessage:
                    {
                        // We're done.
                        break;
                    }

                case Ice1Definitions.MessageType.RequestMessage:
                    {
                        PrintRequest(s, str);
                        break;
                    }

                case Ice1Definitions.MessageType.RequestBatchMessage:
                    {
                        PrintBatchRequest(s, str);
                        break;
                    }

                case Ice1Definitions.MessageType.ReplyMessage:
                    {
                        PrintReply(s, str);
                        break;
                    }

                default:
                    {
                        s.Write("(unknown)");
                        break;
                    }
            }

            return type;
        }

        internal static void TraceHeader(string heading, Ice.InputStream str, Ice.ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                int p = str.Pos;
                str.Pos = 0;

                using (var s = new System.IO.StringWriter(CultureInfo.CurrentCulture))
                {
                    s.Write(heading);
                    PrintHeader(s, str);

                    logger.Trace(tl.ProtocolCat, s.ToString());
                }
                str.Pos = p;
            }
        }

        private static string GetMessageTypeAsString(Ice1Definitions.MessageType type)
        {
            return type switch
            {
                Ice1Definitions.MessageType.RequestMessage => "request",
                Ice1Definitions.MessageType.RequestBatchMessage => "batch request",
                Ice1Definitions.MessageType.ReplyMessage => "reply",
                Ice1Definitions.MessageType.CloseConnectionMessage => "close connection",
                Ice1Definitions.MessageType.ValidateConnectionMessage => "validate connection",
                _ => "unknown",
            };
        }

        private static readonly object _mutex = new object();
    }
}
