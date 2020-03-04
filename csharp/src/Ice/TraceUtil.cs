//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

namespace IceInternal
{
    internal sealed class TraceUtil
    {
        internal static void TraceSend(Ice.OutputStream str, Ice.ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                var buffer = new Buffer(str.ToArray()); // TODO avoid copy the data
                var iss = new Ice.InputStream(str.Communicator, str.Encoding, buffer, false);
                iss.Pos = 0;

                using var s = new System.IO.StringWriter(CultureInfo.CurrentCulture);
                byte type = PrintMessage(s, iss);

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
                    byte type = PrintMessage(s, str);

                    logger.Trace(tl.ProtocolCat, "received " + GetMessageTypeAsString(type) + " " + s.ToString());
                }
                str.Pos = p;
            }
        }

        internal static void Trace(string heading, Ice.OutputStream str, Ice.ILogger logger, TraceLevels tl)
        {
            if (tl.Protocol >= 1)
            {
                var buffer = new Buffer(str.ToArray()); // TODO avoid copy the data
                var iss = new Ice.InputStream(str.Communicator, str.Encoding, buffer, false);
                iss.Pos = 0;

                using var s = new System.IO.StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                PrintMessage(s, iss);

                logger.Trace(tl.ProtocolCat, s.ToString());
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

        public static void DumpStream(Ice.InputStream stream)
        {
            int pos = stream.Pos;
            stream.Pos = 0;

            byte[] data = new byte[stream.Size];
            stream.ReadBlob(data);
            DumpOctets(data);

            stream.Pos = pos;
        }

        public static void DumpOctets(byte[] data)
        {
            const int inc = 8;

            for (int i = 0; i < data.Length; i += inc)
            {
                for (int j = i; j - i < inc; j++)
                {
                    if (j < data.Length)
                    {
                        int n = data[j];
                        if (n < 0)
                        {
                            n += 256;
                        }
                        string s;
                        if (n < 10)
                        {
                            s = "  " + n;
                        }
                        else if (n < 100)
                        {
                            s = " " + n;
                        }
                        else
                        {
                            s = "" + n;
                        }
                        System.Console.Out.Write(s + " ");
                    }
                    else
                    {
                        System.Console.Out.Write("    ");
                    }
                }

                System.Console.Out.Write('"');

                for (int j = i; j < data.Length && j - i < inc; j++)
                {
                    // TODO: this needs fixing
                    if (data[j] >= 32 && data[j] < 127)
                    {
                        System.Console.Out.Write((char)data[j]);
                    }
                    else
                    {
                        System.Console.Out.Write('.');
                    }
                }

                System.Console.Out.WriteLine('"');
            }
        }

        private static void PrintIdentityFacetOperation(System.IO.StringWriter s, Ice.InputStream str)
        {
            try
            {
                Ice.ToStringMode toStringMode = str.Communicator.ToStringMode;

                var identity = new Ice.Identity(str);
                s.Write("\nidentity = " + identity.ToString(toStringMode));

                string[] facet = str.ReadStringArray();
                s.Write("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Write(IceUtilInternal.StringUtil.EscapeString(facet[0], "", toStringMode));
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
                Ice.Encoding v = str.SkipEncapsulation();
                if (!v.Equals(Ice.Util.Encoding_1_0))
                {
                    s.Write("\nencoding = ");
                    s.Write(v.ToString());
                }
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
                if (!v.Equals(Ice.Util.Encoding_1_0))
                {
                    s.Write("\nencoding = ");
                    s.Write(v.ToString());
                }
            }
            catch (System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }

        private static byte PrintHeader(System.IO.StringWriter s, Ice.InputStream str)
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

                byte type = str.ReadByte();
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

        private static byte PrintMessage(System.IO.StringWriter s, Ice.InputStream str)
        {
            byte type = PrintHeader(s, str);

            switch (type)
            {
                case Ice1Definitions.CloseConnectionMessage:
                case Ice1Definitions.ValidateConnectionMessage:
                    {
                        // We're done.
                        break;
                    }

                case Ice1Definitions.RequestMessage:
                    {
                        PrintRequest(s, str);
                        break;
                    }

                case Ice1Definitions.RequestBatchMessage:
                    {
                        PrintBatchRequest(s, str);
                        break;
                    }

                case Ice1Definitions.ReplyMessage:
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

        private static string GetMessageTypeAsString(byte type)
        {
            return type switch
            {
                Ice1Definitions.RequestMessage => "request",
                Ice1Definitions.RequestBatchMessage => "batch request",
                Ice1Definitions.ReplyMessage => "reply",
                Ice1Definitions.CloseConnectionMessage => "close connection",
                Ice1Definitions.ValidateConnectionMessage => "validate connection",
                _ => "unknown",
            };
        }

        private static readonly object _mutex = new object();
    }
}
