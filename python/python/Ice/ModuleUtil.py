#  Copyright (c) ZeroC, Inc.

import sys
import types

_pendingModules = {}


def openModule(name):
    global _pendingModules
    if name in sys.modules:
        result = sys.modules[name]
    elif name in _pendingModules:
        result = _pendingModules[name]
    else:
        result = createModule(name)

    return result


def createModule(name):
    global _pendingModules
    parts = name.split(".")
    curr = ""
    mod = None

    for s in parts:
        curr = curr + s

        if curr in sys.modules:
            mod = sys.modules[curr]
        elif curr in _pendingModules:
            mod = _pendingModules[curr]
        else:
            nmod = types.ModuleType(curr)
            _pendingModules[curr] = nmod
            mod = nmod

        curr = curr + "."

    return mod


def updateModule(name):
    global _pendingModules
    if name in _pendingModules:
        pendingModule = _pendingModules[name]
        mod = sys.modules[name]
        mod.__dict__.update(pendingModule.__dict__)
        del _pendingModules[name]


def updateModules():
    global _pendingModules
    for name in _pendingModules.keys():
        if name in sys.modules:
            sys.modules[name].__dict__.update(_pendingModules[name].__dict__)
        else:
            sys.modules[name] = _pendingModules[name]
    _pendingModules = {}
