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

    #
    # Do not define these here. They will be invoked if defined by a subclass.
    #
    #def ice_preMarshal(self):
    #    pass
    #
    #def ice_postUnmarshal(self):
    #    pass

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

def Identity__hash__(self):
    return 5 * hash(self.category) + hash(self.name)
Identity.__hash__ = Identity__hash__
del Identity__hash__

def Identity__lt__(self, other):
    if self.category < other.category:
        return True
    elif self.category == other.category:
        return self.name < other.name
    return False
Identity.__lt__ = Identity__lt__
del Identity__lt__

def Identity__le__(self, other):
    return self.__lt__(other) or self.__eq__(other)
Identity.__le__ = Identity__le__
del Identity__le__

def Identity__eq__(self, other):
    return self.category == other.category and self.name == other.name
Identity.__eq__ = Identity__eq__
del Identity__eq__

def Identity__ne__(self, other):
    return not self.__eq__(other)
Identity.__ne__ = Identity__ne__
del Identity__ne__

def Identity__gt__(self, other):
    if self.category > other.category:
        return True
    elif self.category == other.category:
        return self.name > other.name
    return False
Identity.__gt__ = Identity__gt__
del Identity__gt__

def Identity__ge__(self, other):
    return self.__gt__(other) or self.__eq__(other)
Identity.__ge__ = Identity__ge__
del Identity__ge__

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
