// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using IceUtil;

    sealed class TraceUtil
    {
        internal static void traceHeader(string heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
        {
            if(tl.protocol >= 1)
            {
                int p = str.pos();
                str.pos(0);
                
                using(System.IO.StringWriter s = new System.IO.StringWriter())
                {
                    s.Write(heading);
                    printHeader(s, str);
                    
                    logger.trace(tl.protocolCat, s.ToString());
                }
                str.pos(p);
            }
        }
        
        internal static void traceRequest(string heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
        {
            if(tl.protocol >= 1)
            {
                int p = str.pos();
                str.pos(0);
                
                using(System.IO.StringWriter s = new System.IO.StringWriter())
                {
                    s.Write(heading);
                    printHeader(s, str);
                    
                    int requestId = str.readInt();
                    s.Write("\nrequest id = " + requestId);
                    if(requestId == 0)
                    {
                        s.Write(" (oneway)");
                    }
                    
                    printRequestHeader(s, str);
                    
                    logger.trace(tl.protocolCat, s.ToString());
                }
                str.pos(p);
            }
        }
        
        internal static void traceBatchRequest(string heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
        {
            if(tl.protocol >= 1)
            {
                int p = str.pos();
                str.pos(0);
                
                using(System.IO.StringWriter s = new System.IO.StringWriter())
                {
                    s.Write(heading);
                    printHeader(s, str);
                    
                    int batchRequestNum = str.readInt();
                    s.Write("\nnumber of requests = " + batchRequestNum);
                    
                    for(int i = 0; i < batchRequestNum; ++i)
                    {
                        s.Write("\nrequest #" + i + ':');
                        printRequestHeader(s, str);
                        str.skipEncaps();
                    }
                    
                    logger.trace(tl.protocolCat, s.ToString());
                }
                str.pos(p);
            }
        }
        
        internal static void traceReply(string heading, BasicStream str, Ice.Logger logger, TraceLevels tl)
        {
            if(tl.protocol >= 1)
            {
                int p = str.pos();
                str.pos(0);
                
                using(System.IO.StringWriter s = new System.IO.StringWriter())
                {
                    s.Write(heading);
                    printHeader(s, str);
                    
                    int requestId = str.readInt();
                    s.Write("\nrequest id = " + requestId);
                    
                    byte replyStatus = str.readByte();
                    s.Write("\nreply status = " + (int)replyStatus + ' ');
                    
                    switch(replyStatus)
                    {
                        case ReplyStatus.replyOK: 
                        {
                            s.Write("(ok)");
                            break;
                        }
                        
                        case ReplyStatus.replyUserException: 
                        {
                            s.Write("(user exception)");
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
                                    s.Write("(object not exist)");
                                    break;
                                }
                                
                                case ReplyStatus.replyFacetNotExist: 
                                {
                                    s.Write("(facet not exist)");
                                    break;
                                }
                                
                                case ReplyStatus.replyOperationNotExist: 
                                {
                                    s.Write("(operation not exist)");
                                    break;
                                }
                                
                                default: 
                                {
                                    Debug.Assert(false);
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
                                    s.Write("(unknown exception)");
                                    break;
                                }
                                
                                case ReplyStatus.replyUnknownLocalException: 
                                {
                                    s.Write("(unknown local exception)");
                                    break;
                                }
                                
                                case ReplyStatus.replyUnknownUserException: 
                                {
                                    s.Write("(unknown user exception)");
                                    break;
                                }
                                
                                default: 
                                {
                                    Debug.Assert(false);
                                    break;
                                }
                            }
                            
                            string unknown = str.readString();
                            s.Write("\nunknown = " + unknown);
                            break;
                        }
                        
                        default: 
                        {
                            s.Write("(unknown)");
                            break;
                        }
                        
                    }
                    
                    logger.trace(tl.protocolCat, s.ToString());
                }
                str.pos(p);
            }
        }
        
        private static Set slicingIds;
        
        internal static void traceSlicing(string kind, string typeId, string slicingCat, Ice.Logger logger)
        {
            lock(typeof(IceInternal.TraceUtil))
            {
                if(slicingIds.Add(typeId))
                {
                    using(System.IO.StringWriter s = new System.IO.StringWriter())
                    {
                        s.Write("unknown " + kind + " type `" + typeId + "'");
                        logger.trace(slicingCat, s.ToString());
                    }
                }
            }
        }
        
        public static void dumpStream(BasicStream stream)
        {       
            int pos = stream.pos();
            stream.pos(0);
            
            byte[] data = new byte[stream.size()];
            stream.readBlob(data);
            dumpOctets(data);
            
            stream.pos(pos);
        }
        
        public static void dumpOctets(byte[] data)
        {
            const int inc = 8;
            
            for(int i = 0; i < data.Length; i += inc)
            {
                for(int j = i; j - i < inc; j++)
                {
                    if(j < data.Length)
                    {
                        int n = (int)data[j];
                        if(n < 0)
                        {
                            n += 256;
                        }
                        string s;
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
                        System.Console.Out.Write(s + " ");
                    }
                    else
                    {
                        System.Console.Out.Write("    ");
                    }
                }
                
                System.Console.Out.Write('"');
                
                for(int j = i; j < data.Length && j - i < inc; j++)
                {
                    // TODO: this needs fixing
                    if(data[j] >= (byte)32 && data[j] < (byte)127)
                    {
                        System.Console.Out.Write((char) data[j]);
                    }
                    else
                    {
                        System.Console.Out.Write('.');
                    }
                }
                
                System.Console.Out.WriteLine('"');
            }
        }
        
        private static void printIdentityFacetOperation(System.IO.StringWriter o, BasicStream stream)
        {
            try
            {
                Ice.Identity identity = new Ice.Identity();
                identity.read__(stream);
                o.Write("\nidentity = " + stream.instance().identityToString(identity));
                
                string[] facet = stream.readStringSeq();
                o.Write("\nfacet = ");
                if(facet.Length > 0)
                {
                    o.Write(IceUtil.StringUtil.escapeString(facet[0], ""));
                }
                
                string operation = stream.readString();
                o.Write("\noperation = " + operation);
            }
            catch(System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }
        
        private static void printRequestHeader(System.IO.StringWriter o, BasicStream stream)
        {
            printIdentityFacetOperation(o, stream);
            
            try
            {
                byte mode = stream.readByte();
                o.Write("\nmode = " + (int)mode + ' ');
                switch((Ice.OperationMode)mode)
                {
                    case Ice.OperationMode.Normal:
                    {
                        o.Write("(normal)");
                        break;
                    }
                    
                    case Ice.OperationMode.Nonmutating:
                    {
                        o.Write("(nonmutating)");
                        break;
                    }
                    
                    case Ice.OperationMode.Idempotent:
                    {
                        o.Write("(idempotent)");
                        break;
                    }
                    
                    default:
                    {
                        o.Write("(unknown)");
                        break;
                    }
                }
                
                int sz = stream.readSize();
                o.Write("\ncontext = ");
                while(sz-- > 0)
                {
                    string key = stream.readString();
                    string val = stream.readString();
                    o.Write(key + '/' + val);
                    if(sz > 0)
                    {
                        o.Write(", ");
                    }
                }
            }
            catch(System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }
        
        private static void printHeader(System.IO.StringWriter o, BasicStream stream)
        {
            try
            {
                stream.readByte(); // Don't bother printing the magic number
                stream.readByte();
                stream.readByte();
                stream.readByte();
                
                /* byte pMajor = */ stream.readByte();
                /* byte pMinor = */ stream.readByte();
                //o.Write("\nprotocol version = " + (int)pMajor + "." + (int)pMinor);
                
                /* byte eMajor = */ stream.readByte();
                /* byte eMinor = */ stream.readByte();
                //o.Write("\nencoding version = " + (int)eMajor + "." + (int)eMinor);
                
                byte type = stream.readByte();
                o.Write("\nmessage type = " + (int)type + ' ');
                switch(type)
                {
                    case Protocol.requestMsg: 
                    {
                        o.Write("(request)");
                        break;
                    }
                    
                    case Protocol.requestBatchMsg: 
                    {
                        o.Write("(batch request)");
                        break;
                    }
                    
                    case Protocol.replyMsg: 
                    {
                        o.Write("(reply)");
                        break;
                    }
                    
                    case Protocol.closeConnectionMsg: 
                    {
                        o.Write("(close connection)");
                        break;
                    }
                    
                    case Protocol.validateConnectionMsg: 
                    {
                        o.Write("(validate connection)");
                        break;
                    }
                    
                    default: 
                    {
                        o.Write("(unknown)");
                        break;
                    }
                }
                
                byte compress = stream.readByte();
                o.Write("\ncompression status = " + (int)compress + ' ');
                switch(compress)
                {
                    case (byte)0: 
                    {
                        o.Write("(not compressed; do not compress response, if any)");
                        break;
                    }
                    
                    case (byte)1: 
                    {
                        o.Write("(not compressed; compress response, if any)");
                        break;
                    }
                    
                    case (byte)2: 
                    {
                        o.Write("(compressed; compress response, if any)");
                        break;
                    }
                    
                    default: 
                    {
                        o.Write("(unknown)");
                        break;
                    }
                }
                
                int size = stream.readInt();
                o.Write("\nmessage size = " + size);
            }
            catch(System.IO.IOException)
            {
                Debug.Assert(false);
            }
        }

        static TraceUtil()
        {
            slicingIds = new Set();
        }
    }

}
