"""
Ice module
"""

import sys, exceptions, string, imp
import IcePy

#
# Add some symbols to the Ice module.
#
ObjectPrx = IcePy.ObjectPrx
identityToString = IcePy.identityToString
stringToIdentity = IcePy.stringToIdentity
loadSlice = IcePy.loadSlice

#
# Core Ice types.
#
class Object(object):
    def ice_isA(self, id, current=None):
        return id in self.ice_ids()

    def ice_ping(self, current=None):
        pass

    def ice_ids(self, current=None):
        return [ self.ice_id() ]

    def ice_id(self, current=None):
        return '::Ice::Object'

class LocalObject(object):
    pass

#
# Exceptions.
#
class Exception(exceptions.Exception):
    def __str__(self):
        return self.__class__.__name__

class LocalException(Exception):
    def __init__(self, args=''):
        self.args = args

class UserException(Exception):
    pass

#
# Utilities.
#
def openModule(name):
    if sys.modules.has_key(name):
        result = sys.modules[name]
    else:
        result = createModule(name)

    return result

def createModule(name):
    l = string.split(name, ".")
    curr = ''
    mod = None

    for s in l:
        curr = curr + s

        if sys.modules.has_key(curr):
            mod = sys.modules[curr]
        else:
            nmod = imp.new_module(curr)
            if mod:
                setattr(mod, s, nmod)
            sys.modules[curr] = nmod
            mod = nmod

        curr = curr + "."

    return mod

def createTempClass():
    class __temp: pass
    return __temp

#
# Forward declarations.
#
IcePy.declareClass('::Ice::Object')
IcePy.declareProxy('::Ice::Object')

#
# Import Ice types.
#
import BuiltinSequences_ice
import CommunicatorImpl
import Current_ice
import Identity_ice
import LocalException_ice
import ObjectAdapterImpl
import ObjectFactory_ice
import PropertiesImpl
import ServantLocator_ice
import Router_ice
import Locator_ice
import Logger_ice
import ApplicationImpl

#
# Define Ice::Object and Ice::ObjectPrx.
#
IcePy.defineClass('::Ice::Object', Object, False, '', (), ())
IcePy.defineProxy('::Ice::Object', ObjectPrx)

Object._op_ice_isA = IcePy.Operation('ice_isA', OperationMode.Nonmutating, (IcePy.T_STRING,), (), IcePy.T_BOOL, ())
Object._op_ice_ping = IcePy.Operation('ice_ping', OperationMode.Nonmutating, (), (), None, ())
Object._op_ice_ids = IcePy.Operation('ice_ids', OperationMode.Nonmutating, (), (), '::Ice::StringSeq', ())
Object._op_ice_id = IcePy.Operation('ice_id', OperationMode.Nonmutating, (), (), IcePy.T_STRING, ())

#
# Annotate Ice::Identity.
#
def Identity__str__(self):
    return IcePy.identityToString(self)
Identity.__str__ = Identity__str__
del Identity__str__

def Identity__cmp__(self, other):
    n = cmp(self.category, other.category)
    if n != 0:
        return n
    return cmp(self.name, other.name)
Identity.__cmp__ = Identity__cmp__
del Identity__cmp__

#
# Proxy comparison functions.
#
def proxyIdentityEqual(lhs, rhs):
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return True
    elif not lhs and rhs:
        return False
    elif lhs and not rhs:
        return False
    else:
        return lhs.ice_getIdentity() == rhs.ice_getIdentity()

def proxyIdentityAndFacetEqual(lhs, rhs):
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return True
    elif not lhs and rhs:
        return False
    elif lhs and not rhs:
        return False
    else:
        return lhs.ice_getIdentity() == rhs.ice_getIdentity() and lhs.ice_getFacet() == rhs.ice_getFacet()
