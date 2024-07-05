# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class LocalException(Exception):
    """The base class for all Ice run-time exceptions."""

    def __init__(self, args=""):
        self.args = args

    __module__ = "Ice"
    __class__ = "LocalException"
