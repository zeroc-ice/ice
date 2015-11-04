// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

/**
 *
 * IceBT provides a Bluetooth transport for Ice.
 *
 **/
["objc:prefix:ICEBT"]
module IceBT
{

/**
 *
 * Indicates a failure in the Bluetooth plug-in.
 *
 **/
["cpp:ice_print"]
local exception BluetoothException
{
    /** Provides more information about the failure. */
    string reason;
};

/**
 *
 * Provides information about a Bluetooth device.
 *
 **/
local struct DeviceInfo
{
    /** The device's Bluetooth address. */
    string address;
    /** The device's display name, or an empty string if no name is available. */
    string name;
};
/** A list of device information structures. */
local sequence<DeviceInfo> DeviceInfoSeq;

/**
 *
 * Provides information about a service offered by a Bluetooth device.
 *
 **/
local struct ServiceInfo
{
    /** The Bluetooth address of the device on which the service is running. */
    string address;
    /** The UUID of the service. */
    string uuid;
    /** The display name of the service, or an empty string if no name is available. */
    string name;
    /** The RFCOMM channel on which the service is running. */
    int channel;
};
/** A list of service information structures. */
local sequence<ServiceInfo> ServiceInfoSeq;

};
