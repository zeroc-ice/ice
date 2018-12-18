# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, Test, sys, TestI

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def testFacets(com, builtInFacets = True):

    if builtInFacets:
        test(com.findAdminFacet("Properties") != None)
        test(com.findAdminFacet("Process") != None)
        test(com.findAdminFacet("Logger") != None)
        test(com.findAdminFacet("Metrics") != None)

    f1 = TestI.TestFacetI()
    f2 = TestI.TestFacetI()
    f3 = TestI.TestFacetI()

    com.addAdminFacet(f1, "Facet1")
    com.addAdminFacet(f2, "Facet2")
    com.addAdminFacet(f3, "Facet3")

    test(com.findAdminFacet("Facet1") == f1)
    test(com.findAdminFacet("Facet2") == f2)
    test(com.findAdminFacet("Facet3") == f3)
    test(com.findAdminFacet("Bogus") == None)

    facetMap = com.findAllAdminFacets()
    if builtInFacets:
        test(len(facetMap) == 7)
        test("Properties" in facetMap)
        test(isinstance(facetMap["Properties"], Ice.NativePropertiesAdmin))
        test("Process" in facetMap)
        test("Logger" in facetMap)
        test("Metrics" in facetMap)

    test(len(facetMap) >=3)

    test("Facet1" in facetMap)
    test("Facet2" in facetMap)
    test("Facet3" in facetMap)

    try:
        com.addAdminFacet(f1, "Facet1")
        test(False)
    except Ice.AlreadyRegisteredException:
        pass # Expected

    try:
        com.removeAdminFacet("Bogus")
        test(False)
    except Ice.NotRegisteredException:
        pass # Expected

    com.removeAdminFacet("Facet1")
    com.removeAdminFacet("Facet2")
    com.removeAdminFacet("Facet3")

    try:
        com.removeAdminFacet("Facet1")
        test(False)
    except Ice.NotRegisteredException:
        pass # Expected

def allTests(helper, communicator):
    sys.stdout.write("testing communicator operations... ")
    sys.stdout.flush()

    #
    # Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
    #
    init = Ice.InitializationData()
    init.properties = Ice.createProperties()
    init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1")
    init.properties.setProperty("Ice.Admin.InstanceName", "Test")
    init.properties.setProperty("Ice.ProgramName", "MyTestProgram")
    com = Ice.initialize(init)
    testFacets(com)
    test(com.getLogger().getPrefix() == "MyTestProgram")
    com.destroy()

    #
    # Test: Verify that the operations work correctly in the presence of facet filters.
    #
    init = Ice.InitializationData()
    init.properties = Ice.createProperties()
    init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1")
    init.properties.setProperty("Ice.Admin.InstanceName", "Test")
    init.properties.setProperty("Ice.Admin.Facets", "Properties")
    com = Ice.initialize(init)
    testFacets(com, False)
    com.destroy()

    #
    # Test: Verify that the operations work correctly with the Admin object disabled.
    #
    com = Ice.initialize()
    testFacets(com, False)
    com.destroy()

    #
    # Test: Verify that the operations work correctly when Ice.Admin is enabled.
    #
    init = Ice.InitializationData()
    init.properties = Ice.createProperties()
    init.properties.setProperty("Ice.Admin.Enabled", "1")
    com = Ice.initialize(init)
    test(com.getAdmin() == None)
    identity = Ice.stringToIdentity("test-admin")
    try:
        com.createAdmin(None, identity)
        test(False)
    except Ice.InitializationException:
        pass

    adapter = com.createObjectAdapter("")
    test(com.createAdmin(adapter, identity) != None)
    test(com.getAdmin() != None)

    testFacets(com)
    com.destroy()

    #
    # Test: Verify that the operations work correctly when creation of the Admin object is delayed.
    #
    init = Ice.InitializationData()
    init.properties = Ice.createProperties()
    init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1")
    init.properties.setProperty("Ice.Admin.InstanceName", "Test")
    init.properties.setProperty("Ice.Admin.DelayCreation", "1")
    com = Ice.initialize(init)
    testFacets(com)
    com.getAdmin()
    testFacets(com)
    com.destroy()
    print("ok")

    ref = "factory:{0} -t 10000".format(helper.getTestEndpoint())
    factory = Test.RemoteCommunicatorFactoryPrx.uncheckedCast(communicator.stringToProxy(ref))

    sys.stdout.write("testing process facet... ")
    sys.stdout.flush()

    #
    # Test: Verify that Process::shutdown() operation shuts down the communicator.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()
    proc = Ice.ProcessPrx.checkedCast(obj, "Process")
    proc.shutdown()
    com.waitForShutdown()
    com.destroy()

    print("ok")

    sys.stdout.write("testing properties facet... ")
    sys.stdout.flush()

    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Prop1"] = "1"
    props["Prop2"] = "2"
    props["Prop3"] = "3"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()
    pa = Ice.PropertiesAdminPrx.checkedCast(obj, "Properties")

    #
    # Test: PropertiesAdmin::getProperty()
    #
    test(pa.getProperty("Prop2") == "2")
    test(pa.getProperty("Bogus") == "")

    #
    # Test: PropertiesAdmin::getProperties()
    #
    pd = pa.getPropertiesForPrefix("")
    test(len(pd) == 5)
    test(pd["Ice.Admin.Endpoints"] == "tcp -h 127.0.0.1")
    test(pd["Ice.Admin.InstanceName"] == "Test")
    test(pd["Prop1"] == "1")
    test(pd["Prop2"] == "2")
    test(pd["Prop3"] == "3")

    changes = {}

    #
    # Test: PropertiesAdmin::setProperties()
    #
    setProps = {}
    setProps["Prop1"] = "10" # Changed
    setProps["Prop2"] = "20" # Changed
    setProps["Prop3"] = "" # Removed
    setProps["Prop4"] = "4" # Added
    setProps["Prop5"] = "5" # Added
    pa.setProperties(setProps)
    test(pa.getProperty("Prop1") == "10")
    test(pa.getProperty("Prop2") == "20")
    test(pa.getProperty("Prop3") == "")
    test(pa.getProperty("Prop4") == "4")
    test(pa.getProperty("Prop5") == "5")
    changes = com.getChanges()
    test(len(changes) == 5)
    test(changes["Prop1"] == "10")
    test(changes["Prop2"] == "20")
    test(changes["Prop3"] == "")
    test(changes["Prop4"] == "4")
    test(changes["Prop5"] == "5")
    pa.setProperties(setProps)
    changes = com.getChanges()
    test(len(changes) == 0)

    com.destroy()

    print("ok")

    sys.stdout.write("testing custom facet... ")
    sys.stdout.flush()

    #
    # Test: Verify that the custom facet is present.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()
    tf = Test.TestFacetPrx.checkedCast(obj, "TestFacet")
    tf.op()
    com.destroy()

    print("ok")

    sys.stdout.write("testing facet filtering... ")
    sys.stdout.flush()

    #
    # Test: Set Ice.Admin.Facets to expose only the Properties facet,
    # meaning no other facet is available.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Ice.Admin.Facets"] = "Properties"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()

    proc = Ice.ProcessPrx.checkedCast(obj, "Process")
    test(proc == None)
    tf = Test.TestFacetPrx.checkedCast(obj, "TestFacet")
    test(tf == None)
    com.destroy()

    #
    # Test: Set Ice.Admin.Facets to expose only the Process facet,
    # meaning no other facet is available.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Ice.Admin.Facets"] = "Process"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()

    pa = Ice.PropertiesAdminPrx.checkedCast(obj, "Properties")
    test(pa == None)
    tf = Test.TestFacetPrx.checkedCast(obj, "TestFacet")
    test(tf == None)

    com.destroy()

    #
    # Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
    # meaning no other facet is available.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Ice.Admin.Facets"] = "TestFacet"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()

    pa = Ice.PropertiesAdminPrx.checkedCast(obj, "Properties")
    test(pa == None)

    proc = Ice.ProcessPrx.checkedCast(obj, "Process")
    test(proc == None)

    com.destroy()

    #
    # Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
    # facet names.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Ice.Admin.Facets"] = "Properties TestFacet"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()
    pa = Ice.PropertiesAdminPrx.checkedCast(obj, "Properties")
    test(pa.getProperty("Ice.Admin.InstanceName") == "Test")
    tf = Test.TestFacetPrx.checkedCast(obj, "TestFacet")
    tf.op()

    proc = Ice.ProcessPrx.checkedCast(obj, "Process")
    test(proc == None)
    com.destroy()

    #
    # Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
    # facet names.
    #
    props = {}
    props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1"
    props["Ice.Admin.InstanceName"] = "Test"
    props["Ice.Admin.Facets"] = "TestFacet, Process"
    com = factory.createCommunicator(props)
    obj = com.getAdmin()

    pa = Ice.PropertiesAdminPrx.checkedCast(obj, "Properties")
    test(pa == None)

    tf = Test.TestFacetPrx.checkedCast(obj, "TestFacet")
    tf.op()
    proc = Ice.ProcessPrx.checkedCast(obj, "Process")
    proc.shutdown()
    com.waitForShutdown()
    com.destroy()

    print("ok")

    factory.shutdown()
