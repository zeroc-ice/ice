// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
printIdentityFacetOperation(ostream& s, BasicStream& stream)
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
}

static void
printRequestHeader(ostream& s, BasicStream& stream)
{
    printIdentityFacetOperation(s, stream);

    bool idempotent;
    stream.read(idempotent);
    s << "\nidempotent = " << (idempotent ? "true" : "false");

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
	    case DispatchFacetNotExist:
	    case DispatchOperationNotExist:
	    {
		switch(static_cast<DispatchStatus>(status))
		{
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

		    default:
		    {
			assert(false);
			break;
		    }
		}

		printIdentityFacetOperation(s, stream);
		break;
	    }

	    case DispatchUnknownException:
	    case DispatchUnknownLocalException:
	    case DispatchUnknownUserException:
	    {
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchUnknownException:
		    {
			s << "(unknown exception)";
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

		    default:
		    {
			assert(false);
			break;
		    }
		}
		
		string unknown;
		stream.read(unknown);
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
