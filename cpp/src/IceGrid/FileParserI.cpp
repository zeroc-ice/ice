// Copyright (c) ZeroC, Inc.

#include "FileParserI.h"
#include "DescriptorParser.h"
#include "Ice/ObjectAdapter.h"
#include "XMLParser.h"

using namespace std;
using namespace IceGrid;

ApplicationDescriptor
FileParserI::parse(string file, optional<AdminPrx> admin, const Ice::Current& current)
{
    try
    {
        return DescriptorParser::parseDescriptor(
            file,
            Ice::StringSeq(),
            map<string, string>(),
            current.adapter->getCommunicator(),
            std::move(admin));
    }
    catch (const XMLParserException& e)
    {
        throw ParseException(e.what());
    }
}
