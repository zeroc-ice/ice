// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TraceUtil.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Stream.h>
#include <Ice/Protocol.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static void
printHeader(ostream& s, Stream& stream)
{
    Byte protVer;
    stream.read(protVer);
    s << "\nprotocol version = " << static_cast<int>(protVer);
    Byte encVer;
    stream.read(encVer);
    s << "\nencoding version = " << static_cast<int>(encVer);
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
	    s << "(request batch)";
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
IceInternal::traceHeader(const char* heading, const Stream& str, const ::Ice::LoggerPtr& logger,
			 const TraceLevelsPtr& tl)
{
    if (tl->protocol >= 1)
    {
	Stream& stream = const_cast<Stream&>(str);
	Stream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();
	ostringstream s;
	s << heading;
	printHeader(s, stream);
	logger->trace(tl->protocolCat, s.str());
	stream.i = p;
    }
}

void
IceInternal::traceRequest(const char* heading, const Stream& str, const ::Ice::LoggerPtr& logger,
			  const TraceLevelsPtr& tl)
{
    if (tl->protocol >= 1)
    {
	Stream& stream = const_cast<Stream&>(str);
	Stream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();
	ostringstream s;
	s << heading;
	printHeader(s, stream);
	Int requestId;
	stream.read(requestId);
	s << "\nrequest id = " << requestId;
	if (requestId == 0)
	{
	    s << " (oneway)";
	}
	string identity;
	stream.read(identity);
	s << "\nidentity = " << identity;
	string operation;
	stream.read(operation);
	s << "\noperation name = " << operation;
	logger->trace(tl->protocolCat, s.str());
	stream.i = p;
    }
}

void
IceInternal::traceBatchRequest(const char* heading, const Stream& str, const ::Ice::LoggerPtr& logger,
			       const TraceLevelsPtr& tl)
{
    if (tl->protocol >= 1)
    {
	Stream& stream = const_cast<Stream&>(str);
	Stream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();
	ostringstream s;
	s << heading;
	printHeader(s, stream);
	stream.i = p;
    }
}

void
IceInternal::traceReply(const char* heading, const Stream& str, const ::Ice::LoggerPtr& logger,
			const TraceLevelsPtr& tl)
{
    if (tl->protocol >= 1)
    {
	Stream& stream = const_cast<Stream&>(str);
	Stream::Container::iterator p = stream.i;
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
	    case DispatchException:
	    {
		s << "(exception)";
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
	    case DispatchOperationNotExist:
	    {
		s << "(operation not exist)";
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
