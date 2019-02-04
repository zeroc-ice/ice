#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def testExceptions(obj):

    try:
        obj.requestFailedException()
        test(False)
    except Ice.ObjectNotExistException as ex:
        test(ex.id == obj.ice_getIdentity())
        test(ex.facet == obj.ice_getFacet())
        test(ex.operation == "requestFailedException")
    except:
        test(False)

    try:
        obj.unknownUserException()
        test(False)
    except Ice.UnknownUserException as ex:
        test(ex.unknown == "reason")
    except:
        test(False)

    try:
        obj.unknownLocalException()
        test(False)
    except Ice.UnknownLocalException as ex:
        test(ex.unknown == "reason")
    except:
        test(False)

    try:
        obj.unknownException()
        test(False)
    except Ice.UnknownException as ex:
        test(ex.unknown == "reason")
        pass

    try:
        obj.userException()
        test(False)
    except Ice.UnknownUserException as ex:
        test(ex.unknown.find("::Test::TestIntfUserException") >= 0)
    except Ice.OperationNotExistException:
        pass
    except AttributeError:
        pass
    except:
        test(False)

    try:
        obj.localException()
        test(False)
    except Ice.UnknownLocalException as ex:
        test(ex.unknown.find("Ice.SocketException") >= 0 or ex.unknown.find("Ice::SocketException") >= 0)
    except:
        test(False)

    try:
        obj.pythonException()
        test(False)
    except Ice.UnknownException as ex:
        test(ex.unknown.find("RuntimeError: message") >= 0)
    except Ice.OperationNotExistException:
        pass
    except AttributeError:
        pass
    except:
        test(False)

    try:
        obj.unknownExceptionWithServantException()
        test(False)
    except Ice.UnknownException as ex:
        test(ex.unknown == "reason")
    except:
        test(False)

    try:
        obj.impossibleException(False)
        test(False)
    except Ice.UnknownUserException:
        # Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
        pass
    except:
        test(False)

    try:
        obj.impossibleException(True)
        test(False)
    except Ice.UnknownUserException:
        # Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
        pass
    except:
        test(False)

    try:
        obj.intfUserException(False)
        test(False)
    except Test.TestImpossibleException:
        # Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
        pass
    except:
        test(False)

    try:
        obj.intfUserException(True)
        test(False)
    except Test.TestImpossibleException:
        # Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
        pass
    except:
        test(False)

def allTests(helper, communicator):
    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("asm:{0}".format(helper.getTestEndpoint()))
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    obj = Test.TestIntfPrx.checkedCast(base)
    test(obj)
    test(obj == base)
    print("ok")

    sys.stdout.write("testing ice_ids... ")
    sys.stdout.flush()
    try:
        obj = communicator.stringToProxy("category/locate:{0}".format(helper.getTestEndpoint()))
        obj.ice_ids()
        test(False)
    except Ice.UnknownUserException as ex:
        test(ex.unknown == "::Test::TestIntfUserException")
    except:
        test(False)

    try:
        obj = communicator.stringToProxy("category/finished:{0}".format(helper.getTestEndpoint()))
        obj.ice_ids()
        test(False)
    except Ice.UnknownUserException as ex:
        test(ex.unknown == "::Test::TestIntfUserException")
    except:
        test(False)
    print("ok")

    sys.stdout.write("testing servant locator... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("category/locate:{0}".format(helper.getTestEndpoint()))
    obj = Test.TestIntfPrx.checkedCast(base)
    try:
        Test.TestIntfPrx.checkedCast(
            communicator.stringToProxy("category/unknown:{0}".format(helper.getTestEndpoint())))
    except Ice.ObjectNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing default servant locator... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("anothercat/locate:{0}".format(helper.getTestEndpoint()))
    obj = Test.TestIntfPrx.checkedCast(base)
    base = communicator.stringToProxy("locate:{0}".format(helper.getTestEndpoint()))
    obj = Test.TestIntfPrx.checkedCast(base)
    try:
        Test.TestIntfPrx.checkedCast(
            communicator.stringToProxy("anothercat/unknown:{0}".format(helper.getTestEndpoint())))
    except Ice.ObjectNotExistException:
        pass
    try:
        Test.TestIntfPrx.checkedCast(communicator.stringToProxy("unknown:{0}".format(helper.getTestEndpoint())))
    except Ice.ObjectNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing locate exceptions... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("category/locate:{0}".format(helper.getTestEndpoint()))
    obj = Test.TestIntfPrx.checkedCast(base)
    testExceptions(obj)
    print("ok")

    sys.stdout.write("testing finished exceptions... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("category/finished:{0}".format(helper.getTestEndpoint()))
    obj = Test.TestIntfPrx.checkedCast(base)
    testExceptions(obj)
    print("ok")

    sys.stdout.write("testing servant locator removal... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("test/activation:{0}".format(helper.getTestEndpoint()))
    activation = Test.TestActivationPrx.checkedCast(base)
    activation.activateServantLocator(False)
    try:
        obj.ice_ping()
        test(False)
    except Ice.ObjectNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing servant locator addition... ")
    sys.stdout.flush()
    activation.activateServantLocator(True)
    try:
        obj.ice_ping()
    except:
        test(False)
    print("ok")

    return obj
