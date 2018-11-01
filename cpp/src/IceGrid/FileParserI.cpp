// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
