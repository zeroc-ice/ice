// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

module Ice
{

/**
 *
 * A version structure for the protocol version.
 *
 **/
struct ProtocolVersion
{
    byte major;
    byte minor;
};

/**
 *
 * A version structure for the encoding version.
 *
 **/
struct EncodingVersion
{
    byte major;
    byte minor;
};

};
