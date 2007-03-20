// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ITEM_INFO_ICE
#define ITEM_INFO_ICE

//
// Data types persisted in the Freeze database
//
module Warehouse
{

struct ItemInfo
{
    string description;
    float unitPrice;
    int quantityInStock;
    string filler;
};

};

#endif
