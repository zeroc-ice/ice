// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPACK_DESCRIPTOR_PARSER_H
#define ICEPACK_DESCRIPTOR_PARSER_H

namespace IceGrid
{

class DescriptorParser
{
public:

    static ApplicationDescriptor parseDescriptor(const std::string&, 
						 const Ice::StringSeq&, 
						 const std::map<std::string, std::string>&, 
						 const Ice::CommunicatorPtr&);
    
};

}

#endif
