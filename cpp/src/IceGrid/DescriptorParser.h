//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_DESCRIPTOR_PARSER_H
#define ICEGRID_DESCRIPTOR_PARSER_H

namespace IceGrid
{

class DescriptorParser
{
public:

    static ApplicationDescriptor parseDescriptor(const std::string&,
                                                 const Ice::StringSeq&,
                                                 const std::map<std::string, std::string>&,
                                                 const Ice::CommunicatorPtr&,
                                                 const IceGrid::AdminPrx&);

    static ApplicationDescriptor parseDescriptor(const std::string&, const Ice::CommunicatorPtr&);

};

}

#endif
