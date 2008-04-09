// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
                    
                    string operation = str.readString();
                    s.Write("\noperation = " + operation);

                    Ice.ObjectPrx proxy = str.readProxy();
                    s.Write("\nproxy = " + proxy.ice_toString());
                
                    try
                    {
                        byte mode = str.readByte();
                        s.Write("\nmode = " + (int)mode + ' ');
                        switch((Ice.OperationMode)mode)
                        {
                            case Ice.OperationMode.Normal:
                            {
                                s.Write("(normal)");
                                break;
                            }

                            case Ice.OperationMode.Nonmutating:
                            {
                                s.Write("(nonmutating)");
                                break;
                            }

                            case Ice.OperationMode.Idempotent:
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

                        int sz = str.readSize();
                        s.Write("\ncontext = ");
                        while(sz-- > 0)
                        {
                            string key = str.readString();
                            string val = str.readString();
                            s.Write(key + '/' + val);
                            if(sz > 0)
                            {
                                s.Write(", ");
                            }
                        }
                    }
                    catch(System.IO.IOException)
                    {
                        Debug.Assert(false);
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
