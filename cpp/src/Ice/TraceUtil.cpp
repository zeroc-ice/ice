// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StaticMutex.h>
#include <IceUtil/StringUtil.h>
#include <Ice/TraceUtil.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/BasicStream.h>
#include <Ice/Protocol.h>
#include <Ice/ReplyStatus.h>
#include <set>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static void
printIdentityFacetOperation(ostream& s, BasicStream& stream)
{
    Identity identity;
    identity.__read(&stream);
    s << "\nidentity = " << stream.instance()->identityToString(identity);

    vector<string> facet;
    stream.read(facet);
    s << "\nfacet = ";
    if(!facet.empty())
    {
        s << IceUtil::escapeString(facet[0], "");
    }

    string operation;
    stream.read(operation, false);
    s << "\noperation = " << operation;
}

static void
printRequestHeader(ostream& s, BasicStream& stream)
{
    printIdentityFacetOperation(s, stream);

    Byte mode;
    stream.read(mode);
    s << "\nmode = " << static_cast<int>(mode) << ' ';
    switch(mode)
    {
        case Normal:
        {
            s << "(normal)";
            break;
        }

        case Nonmutating:
        {
            s << "(nonmutating)";
            break;
        }

        case Idempotent:
        {
            s << "(idempotent)";
            break;
        }

        default:
        {
            s << "(unknown)";
            break;
        }
    }

    Int sz;
    stream.readSize(sz);
    s << "\ncontext = ";
    while(sz--)
    {
        pair<string, string> pair;
        stream.read(pair.first);
        stream.read(pair.second);
        s << pair.first << '/' << pair.second;
        if(sz)
        {
            s << ", ";
        }
    }
}

static void
printHeader(ostream& s, BasicStream& stream)
{
    Byte magicNumber;
    stream.read(magicNumber);   // Don't bother printing the magic number
    stream.read(magicNumber);
    stream.read(magicNumber);
    stream.read(magicNumber);

    Byte pMajor;
    Byte pMinor;
    stream.read(pMajor);
    stream.read(pMinor);
//    s << "\nprotocol version = " << static_cast<unsigned>(pMajor)
//      << "." << static_cast<unsigned>(pMinor);

    Byte eMajor;
    Byte eMinor;
    stream.read(eMajor);
    stream.read(eMinor);
//    s << "\nencoding version = " << static_cast<unsigned>(eMajor)
//      << "." << static_cast<unsigned>(eMinor);

    Byte type;
    stream.read(type);
    s << "\nmessage type = "  << static_cast<int>(type) << ' ';

    switch(type)
    {
        case requestMsg:
        {
            s << "(request)";
            break;
        }

        case requestBatchMsg:
        {
            s << "(batch request)";
            break;
        }

        case replyMsg:
        {
            s << "(reply)";
            break;
        }

        case closeConnectionMsg:
        {
            s << "(close connection)";
            break;
        }

        case validateConnectionMsg:
        {
            s << "(validate connection)";
            break;
        }

        default:
        {
            s << "(unknown)";
            break;
        }
    }

    Byte compress;
    stream.read(compress);
    s << "\ncompression status = "  << static_cast<int>(compress) << ' ';

    switch(compress)
    {
        case 0:
        {
            s << "(not compressed; do not compress response, if any)";
            break;
        }

        case 1:
        {
            s << "(not compressed; compress response, if any)";
            break;
        }

        case 2:
        {
            s << "(compressed; compress response, if any)";
            break;
        }

        default:
        {
            s << "(unknown)";
            break;
        }
    }

    Int size;
    stream.read(size);
    s << "\nmessage size = " << size;
}

void
IceInternal::traceHeader(const char* heading, const BasicStream& str, const LoggerPtr& logger,
                         const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        s << heading;
        printHeader(s, stream);

        logger->trace(tl->protocolCat, s.str());
        stream.i = p;
    }
}

void
IceInternal::traceRequest(const char* heading, const BasicStream& str, const LoggerPtr& logger,
                          const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        s << heading;
        printHeader(s, stream);

        Int requestId;
        stream.read(requestId);
        s << "\nrequest id = " << requestId;
        if(requestId == 0)
        {
            s << " (oneway)";
        }

        printRequestHeader(s, stream);

        logger->trace(tl->protocolCat, s.str());
        stream.i = p;
    }
}

void
IceInternal::traceBatchRequest(const char* heading, const BasicStream& str, const LoggerPtr& logger,
                               const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        s << heading;
        printHeader(s, stream);

        int batchRequestNum;
        stream.read(batchRequestNum);
        s << "\nnumber of requests = " << batchRequestNum;

        for(int i = 0; i < batchRequestNum; ++i)
        {
            s << "\nrequest #" << i << ':';
            printRequestHeader(s, stream);
            stream.skipEncaps();
        }

        logger->trace(tl->protocolCat, s.str());
        stream.i = p;
    }
}

void
IceInternal::traceReply(const char* heading, const BasicStream& str, const LoggerPtr& logger,
                        const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        s << heading;
        printHeader(s, stream);

        Int requestId;
        stream.read(requestId);
        s << "\nrequest id = " << requestId;

        Byte replyStatus;
        stream.read(replyStatus);
        s << "\nreply status = " << static_cast<int>(replyStatus) << ' ';
        switch(replyStatus)
        {
            case replyOK:
            {
                s << "(ok)";
                break;
            }

            case replyUserException:
            {
                s << "(user exception)";
                break;
            }

            case replyObjectNotExist:
            case replyFacetNotExist:
            case replyOperationNotExist:
            {
                switch(replyStatus)
                {
                    case replyObjectNotExist:
                    {
                        s << "(object not exist)";
                        break;
                    }

                    case replyFacetNotExist:
                    {
                        s << "(facet not exist)";
                        break;
                    }

                    case replyOperationNotExist:
                    {
                        s << "(operation not exist)";
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                printIdentityFacetOperation(s, stream);
                break;
            }

            case replyUnknownException:
            case replyUnknownLocalException:
            case replyUnknownUserException:
            {
                switch(replyStatus)
                {
                    case replyUnknownException:
                    {
                        s << "(unknown exception)";
                        break;
                    }

                    case replyUnknownLocalException:
                    {
                        s << "(unknown local exception)";
                        break;
                    }

                    case replyUnknownUserException:
                    {
                        s << "(unknown user exception)";
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }
                
                string unknown;
                stream.read(unknown, false);
                s << "\nunknown = " << unknown;
                break;
            }

            default:
            {
                s << "(unknown)";
                break;
            }
        }

        logger->trace(tl->protocolCat, s.str());
        stream.i = p;
    }
}

static IceUtil::StaticMutex slicingMutex = ICE_STATIC_MUTEX_INITIALIZER;

void
IceInternal::traceSlicing(const char* kind, const string& typeId, const char* slicingCat, const LoggerPtr& logger)
{
    IceUtil::StaticMutex::Lock lock(slicingMutex);
    static set<string> slicingIds;
    if(slicingIds.insert(typeId).second)
    {
        string s("unknown ");
        s += kind;
        s += " type `" + typeId + "'";
        logger->trace(slicingCat, s);
    }
}
