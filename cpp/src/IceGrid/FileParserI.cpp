//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "FileParserI.h"
#include "DescriptorParser.h"
#include "../IceXML/Parser.h"
#include "Ice/ObjectAdapter.h"

using namespace std;
using namespace IceGrid;

ApplicationDescriptor
FileParserI::parse(string file, optional<AdminPrx> admin, const Ice::Current& current)
{
    Ice::checkNotNull(admin, __FILE__, __LINE__, current);
    try
    {
        return DescriptorParser::parseDescriptor(
            std::move(file),
            Ice::StringSeq(),
            map<string, string>(),
            current.adapter->getCommunicator(),
            *std::move(admin));
    }
    catch (const IceXML::ParserException& e)
    {
        throw ParseException(e.reason());
    }
}
