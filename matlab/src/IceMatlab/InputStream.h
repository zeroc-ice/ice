// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/InputStream.h>

namespace IceMatlab
{

struct InputStreamData
{
    std::vector<Ice::Byte> data;
    Ice::InputStream* in;
};

//
// Consumes the vector.
//
void* createInputStream(const std::shared_ptr<Ice::Communicator>&, const Ice::EncodingVersion&,
                        std::vector<Ice::Byte>&);

}
