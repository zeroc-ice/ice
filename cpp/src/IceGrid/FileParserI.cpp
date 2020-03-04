//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/FileParserI.h>
#include <IceGrid/DescriptorParser.h>
#include <IceXML/Parser.h>
#include <Ice/ObjectAdapter.h>

using namespace std;
using namespace IceGrid;

ApplicationDescriptor
FileParserI::parse(string file, shared_ptr<AdminPrx> admin, const Ice::Current& current)
{
    try
    {
        return DescriptorParser::parseDescriptor(move(file), Ice::StringSeq(), map<string, string>(),
                                                 current.adapter->getCommunicator(), move(admin));
    }
    catch(const IceXML::ParserException& e)
    {
        throw ParseException(e.reason());
    }
}
