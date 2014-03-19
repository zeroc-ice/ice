// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Demo
{

//
// A new, improved version of ContactData
//
// Typically we would keep the same type name (ContactData).
// For this Java demo we pick a different names to be able to
// build both old and new versions in the same directory.
//

struct NewContactData
{
    string phoneNumber;
    string emailAddress;
};

};
