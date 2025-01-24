// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_DESCRIPTOR_PARSER_H
#define ICEGRID_DESCRIPTOR_PARSER_H

#include "IceGrid/Admin.h"
#include "IceGrid/Descriptor.h"

namespace IceGrid::DescriptorParser
{
    ApplicationDescriptor parseDescriptor(
        const std::string&,
        const Ice::StringSeq&,
        const std::map<std::string, std::string>&,
        const Ice::CommunicatorPtr&,
        IceGrid::AdminPrx);

    ApplicationDescriptor parseDescriptor(const std::string&, const Ice::CommunicatorPtr&);
}

#endif
