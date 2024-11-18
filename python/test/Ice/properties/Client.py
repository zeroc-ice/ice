#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys
import Ice
from TestHelper import TestHelper


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Client(TestHelper):
    def run(sef, args):
        sys.stdout.write("testing load properties from UTF-8 path... ")
        sys.stdout.flush()
        properties = Ice.createProperties()
        properties.load("./config/中国_client.config")
        test(properties.getIceProperty("Ice.Trace.Network") == "1")
        test(properties.getIceProperty("Ice.Trace.Protocol") == "1")
        test(properties.getProperty("Config.Path") == "./config/中国_client.config")
        test(properties.getIceProperty("Ice.ProgramName") == "PropertiesClient")
        print("ok")

        sys.stdout.write("testing using Ice.Config with multiple config files... ")
        sys.stdout.flush()
        properties = Ice.createProperties(
            ["--Ice.Config=config/config.1, config/config.2, config/config.3"]
        )
        test(properties.getProperty("Config1") == "Config1")
        test(properties.getProperty("Config2") == "Config2")
        test(properties.getProperty("Config3") == "Config3")
        print("ok")

        sys.stdout.write("testing configuration file escapes... ")
        sys.stdout.flush()
        properties = Ice.createProperties(["--Ice.Config=config/escapes.cfg"])

        props = {
            "Foo\tBar": "3",
            "Foo\\tBar": "4",
            "Escape\\ Space": "2",
            "Prop1": "1",
            "Prop2": "2",
            "Prop3": "3",
            "My Prop1": "1",
            "My Prop2": "2",
            "My.Prop1": "a property",
            "My.Prop2": "a     property",
            "My.Prop3": "  a     property  ",
            "My.Prop4": "  a     property  ",
            "My.Prop5": "a \\ property",
            "foo=bar": "1",
            "foo#bar": "2",
            "foo bar": "3",
            "A": "1",
            "B": "2 3 4",
            "C": "5=#6",
            "AServer": "\\\\server\\dir",
            "BServer": "\\server\\dir",
        }

        for k in props.keys():
            test(properties.getProperty(k) == props[k])
        print("ok")

        sys.stdout.write("testing ice properties with set default values...")
        sys.stdout.flush()
        properties = Ice.createProperties()

        toStringMode = properties.getIceProperty("Ice.ToStringMode")
        test(toStringMode == "Unicode")

        closeTimeout = properties.getIcePropertyAsInt(
            "Ice.Connection.Client.CloseTimeout"
        )
        test(closeTimeout == 10)

        retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals")
        test(retryIntervals == ["0"])

        print("ok")

        sys.stdout.write("testing ice properties with unset default values...")
        sys.stdout.flush()
        properties = Ice.createProperties()

        stringValue = properties.getIceProperty("Ice.Admin.Router")
        test(stringValue == "")

        intValue = properties.getIcePropertyAsInt("Ice.Admin.Router")
        test(intValue == 0)

        listValue = properties.getIcePropertyAsList("Ice.Admin.Router")
        test(listValue == [])

        print("ok")

        sys.stdout.write("testing load properties exception... ")
        sys.stdout.flush()
        try:
            properties = Ice.createProperties()
            properties.load("./config/xxx_client.config")
            test(False)
        except Ice.LocalException as ex:
            test("error while accessing file './config/xxx_client.config'" in str(ex))
        print("ok")

        sys.stdout.write(
            "testing that getting an unknown ice property throws an exception..."
        )
        sys.stdout.flush()
        try:
            properties = Ice.createProperties()
            properties.getIceProperty("Ice.UnknownProperty")
            test(False)
        except Ice.PropertyException:
            pass
        print("ok")

        sys.stdout.write(
            "testing that trying to read a non-numeric value as an int throws... "
        )
        sys.stdout.flush()

        properties = Ice.createProperties()
        properties.setProperty("Test", "foo")
        try:
            properties.getPropertyAsInt("Test")
            test(False)
        except Ice.PropertyException:
            pass
        print("ok")
