// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_DB_ICE
#define FREEZE_DB_ICE

/**
 *
 * &Freeze; provides automatic persistence for &Ice;
 * servants. &Freeze; provides a binary data format for maximum speed,
 * as well as an XML data format for maximum flexibility. Freeze
 * databases using the XML data format can be migrated when the
 * &Slice; description of the persistent data changes.
 *
 **/
module Freeze
{

/**
 *
 * A database key, represented as a sequence of bytes.
 *
 **/
sequence<byte> Key;

/**
 *
 * A database value, represented as a sequence of bytes.
 *
 **/
sequence<byte> Value;

};

#endif
