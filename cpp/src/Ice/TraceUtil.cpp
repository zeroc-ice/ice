// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TraceUtil.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/BasicStream.h>
#include <Ice/Protocol.h>
#include <Ice/IdentityUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static void
printHeader(ostream& s, BasicStream& stream)
{
    Byte protVer;
    stream.read(protVer);
//    s << "\nprotocol version = " << static_cast<int>(protVer);
    Byte encVer;
    stream.read(encVer);
//    s << "\nencoding version = " << static_cast<int>(encVer);
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
	case compressedRequestMsg:
	{
	    s << "(compressed request)";
	    break;
	}
	case compressedRequestBatchMsg:
	{
	    s << "(compressed batch request)";
	    break;
	}
	case compressedReplyMsg:
	{
	    s << "(compressed reply)";
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
    Int size;
    stream.read(size);
    s << "\nmessage size = " << size;
}

static void
printRequestHeader(ostream& s, BasicStream& stream)
{
    Identity identity;
    identity.__read(&stream);
    s << "\nidentity = " << identity;
    vector<string> facet;
    stream.read(facet);
    s << "\nfacet = ";
    vector<string>::const_iterator p = facet.begin();
    while(p != facet.end())
    {
	//
	// TODO: Escape for whitespace and slashes.
	//
	s << *p++;
	if(p != facet.end())
	{
	    s << '/';
	}
    }
    string operation;
    stream.read(operation);
    s << "\noperation = " << operation;
    bool nonmutating;
    stream.read(nonmutating);
    s << "\nnonmutating = " << (nonmutating ? "true" : "false");
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

#if 0
static void
dumpOctets(const char* cat, const BasicStream& stream, const ::Ice::LoggerPtr& logger)
{
    ostringstream s;
    s << endl;

    const BasicStream::Container::size_type inc = 8;

    for(BasicStream::Container::size_type i = 0; i < stream.b.size(); i += inc)
    {
        for(BasicStream::Container::size_type j = i; j - i < inc; j++)
        {
            if(j < stream.b.size())
            {
                int n = stream.b[j];
                if(n < 0)
                {
                    n += 256;
                }
                if(n < 10)
                {
                    s << "  " << n;
                }
                else if(n < 100)
                {
                    s << " " << n;
                }
                else
                {
                    s << n;
                }
                s << " ";
            }
            else
            {
                s << "    ";
            }
        }

        s << '"';

        for(BasicStream::Container::size_type j = i; j < stream.b.size() && j - i < inc; j++)
        {
            if(stream.b[j] >= 32 && stream.b[j] < 127)
            {
                s << (char)stream.b[j];
            }
            else
            {
                s << '.';
            }
        }

        s << '"' << endl;
    }

    logger->trace(cat, s.str());
}
#endif

void
IceInternal::traceHeader(const char* heading, const BasicStream& str, const ::Ice::LoggerPtr& logger,
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
IceInternal::traceRequest(const char* heading, const BasicStream& str, const ::Ice::LoggerPtr& logger,
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
IceInternal::traceBatchRequest(const char* heading, const BasicStream& str, const ::Ice::LoggerPtr& logger,
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
	int cnt = 0;
	while(stream.i != stream.b.end())
	{
	    s << "\nrequest #" << cnt++ << ':';
	    printRequestHeader(s, stream);
	    stream.skipEncaps();
	}
	logger->trace(tl->protocolCat, s.str());
	stream.i = p;
    }
}

void
IceInternal::traceReply(const char* heading, const BasicStream& str, const ::Ice::LoggerPtr& logger,
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
	Byte status;
	stream.read(status);
	s << "\nreply status = " << static_cast<int>(status) << ' ';
	switch(static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		s << "(ok)";
		break;
	    }
	    case DispatchUserException:
	    {
		s << "(user exception)";
		break;
	    }
	    case DispatchLocationForward:
	    {
		s << "(location forward)";
		break;
	    }
	    case DispatchObjectNotExist:
	    {
		s << "(object not exist)";
		break;
	    }
	    case DispatchFacetNotExist:
	    {
		s << "(facet not exist)";
		break;
	    }
	    case DispatchOperationNotExist:
	    {
		s << "(operation not exist)";
		break;
	    }
	    case DispatchUnknownLocalException:
	    {
		s << "(unknown local exception)";
		break;
	    }
	    case DispatchUnknownUserException:
	    {
		s << "(unknown user exception)";
		break;
	    }
	    case DispatchUnknownException:
	    {
		s << "(unknown exception)";
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
