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
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Stream.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static void
printHeader(ostream& s, Stream& stream)
{
    bool bigendian;
    stream.read(bigendian);
    s << "\nbigendian = " << boolalpha << bigendian;
    Byte protVer;
    stream.read(protVer);
    s << "\nprotocol version = " << static_cast<int>(protVer);
    Byte encVer;
    stream.read(encVer);
    s << "\nencoding version = " << static_cast<int>(encVer);
    Byte type;
    stream.read(type);
    s << "\nmessage type = " << static_cast<int>(type);
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
	    s << " (oneway)";
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
	s << "\nreply status = " << static_cast<int>(status);
	logger->trace(tl->protocolCat, s.str());
	stream.i = p;
    }
}
