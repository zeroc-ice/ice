// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

module Test
{

interface TestIntf
{
    string getServerFile(string path);
    string getApplicationFile(string path);
}

}
