// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

["objc:prefix:TestSlicingObjectsServer"]
module Test
{

class Forward;

class Hidden
{
    Forward f;
};

class Forward
{
    Hidden h;
};

};
