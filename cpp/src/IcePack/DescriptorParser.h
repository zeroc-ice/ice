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

namespace IcePack
{

class DescriptorParser
{
public:

    static ApplicationDescriptorPtr parseApplicationDescriptor(const std::string&, 
							       const Ice::StringSeq&, 
							       const std::map<std::string, std::string>&, 
							       const Ice::CommunicatorPtr&);

    static ServerDescriptorPtr parseServerDescriptor(const std::string&, 
						     const Ice::StringSeq&, 
						     const std::map<std::string, std::string>&, 
						     const Ice::CommunicatorPtr&);
    
};

}

#endif
