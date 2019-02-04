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
FileParserI::parse(const string& file, const AdminPrx& admin, const Ice::Current& current)
{
    try
    {
        return DescriptorParser::parseDescriptor(file, Ice::StringSeq(), map<string, string>(),
                                                 current.adapter->getCommunicator(), admin);
    }
    catch(const IceXML::ParserException& e)
    {
        throw ParseException(e.reason());
    }
}
