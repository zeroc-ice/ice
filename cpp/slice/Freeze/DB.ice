// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
