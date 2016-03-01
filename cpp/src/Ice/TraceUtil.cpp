// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
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
    stream.read(identity);
    s << "\nidentity = " << stream.instance()->identityToString(identity);

    vector<string> facet;
    stream.read(facet);
    s << "\nfacet = ";
    if(!facet.empty())
    {
        s << IceUtilInternal::escapeString(facet[0], "");
    }

    string operation;
    stream.read(operation, false);
    s << "\noperation = " << operation;
}

static string
getMessageTypeAsString(Byte type)
{
    switch(type)
    {
        case requestMsg:
            return "request";
        case requestBatchMsg:
            return "batch request";
        case replyMsg:
            return "reply";
        case closeConnectionMsg:
            return "close connection";
        case validateConnectionMsg:
            return "validate connection";
        default:
            return "unknown";
    }
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

    Int sz = stream.readSize();
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

    Ice::EncodingVersion v = stream.skipEncaps();
    if(v > Ice::Encoding_1_0)
    {
        s << "\nencoding = " << v;
    }
}

static Byte
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
    s << "\nmessage type = "  << static_cast<int>(type) << " (" << getMessageTypeAsString(type) << ')';

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

    return type;
}

static void
printRequest(ostream& s, BasicStream& stream)
{
    Int requestId;
    stream.read(requestId);
    s << "\nrequest id = " << requestId;
    if(requestId == 0)
    {
        s << " (oneway)";
    }

    printRequestHeader(s, stream);
}

static void
printBatchRequest(ostream& s, BasicStream& stream)
{
    int batchRequestNum;
    stream.read(batchRequestNum);
    s << "\nnumber of requests = " << batchRequestNum;

    for(int i = 0; i < batchRequestNum; ++i)
    {
        s << "\nrequest #" << i << ':';
        printRequestHeader(s, stream);
    }
}

static void
printReply(ostream& s, BasicStream& stream)
{
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

    if(replyStatus == replyOK || replyStatus == replyUserException)
    {
        Ice::EncodingVersion v = stream.skipEncaps();
        if(v > Ice::Encoding_1_0)
        {
            s << "\nencoding = " << v;
        }
    }
}

static Byte
printMessage(ostream& s, BasicStream& stream)
{
    Byte type = printHeader(s, stream);

    switch(type)
    {
    case closeConnectionMsg:
    case validateConnectionMsg:
    {
        // We're done.
        break;
    }

    case requestMsg:
    {
        printRequest(s, stream);
        break;
    }

    case requestBatchMsg:
    {
        printBatchRequest(s, stream);
        break;
    }

    case replyMsg:
    {
        printReply(s, stream);
        break;
    }

    default:
    {
        break;
    }
    }

    return type;
}

namespace
{

IceUtil::Mutex* slicingMutex = 0;

class Init
{
public:

    Init()
    {
        slicingMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete slicingMutex;
        slicingMutex = 0;
    }
};

Init init;

}

void
IceInternal::traceSlicing(const char* kind, const string& typeId, const char* slicingCat, const LoggerPtr& logger)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(slicingMutex);
    static set<string> slicingIds;
    if(slicingIds.insert(typeId).second)
    {
        string s("unknown ");
        s += kind;
        s += " type `" + typeId + "'";
        logger->trace(slicingCat, s);
    }
}

void
IceInternal::traceSend(const BasicStream& str, const LoggerPtr& logger, const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        Byte type = printMessage(s, stream);

        logger->trace(tl->protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.str());
        stream.i = p;
    }
}

void
IceInternal::traceRecv(const BasicStream& str, const LoggerPtr& logger, const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        Byte type = printMessage(s, stream);

        logger->trace(tl->protocolCat, "received " + getMessageTypeAsString(type) + " " + s.str());
        stream.i = p;
    }
}

void
IceInternal::trace(const char* heading, const BasicStream& str, const LoggerPtr& logger, const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
        BasicStream& stream = const_cast<BasicStream&>(str);
        BasicStream::Container::iterator p = stream.i;
        stream.i = stream.b.begin();

        ostringstream s;
        s << heading;
        printMessage(s, stream);

        logger->trace(tl->protocolCat, s.str());
        stream.i = p;
    }
}

