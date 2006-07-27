# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# The test cases for TAO 
#

taoTPConf = " -ORBSvcConf svc.threadPool.conf"
taoTCConf = " -ORBSvcConf svc.threadPerConnection.conf"
taoReactiveConf = " -ORBSvcConf svc.reactive.conf"
taoBlockingConf = " -ORBSvcConf svc.blocking.conf"

productName = "TAO"

# Form
# (productName, (group name, directory, (cases...)))
#
# case form (simple descriptor, unknown, client configuration, server configuration)

testDefinitions = [
	(productName, "latency twoway", "", [
		("1tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " latency twoway", taoTCConf),
		("tpc blocking", taoBlockingConf + " latency twoway", taoTCConf),
		]
	    ),
	(productName, "latency oneway", "", [
		("1tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " latency oneway", taoTCConf),
		("tpc blocking", taoBlockingConf + " latency oneway", taoTCConf),
		]
	    ),
	(productName, "latency twoway AMI", "", [
		("1tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " latency twoway ami", taoTCConf),
		#("tpc blocking", taoBlockingConf + " latency twoway ami", taoTCConf),
		]
	    ),
	(productName, "throughput byte", "", [
		("1tp", taoReactiveConf + " throughput byte", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " throughput byte", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " throughput byte", taoTCConf),
		("tpc blocking", taoBlockingConf + " latency throughput byte", taoTCConf),
		]
	    ),
	(productName, "throughput string seq", "", [
		("1tp", taoReactiveConf + " throughput string", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " throughput string", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " throughput string", taoTCConf),
		("tpc blocking", taoBlockingConf + " throughput string", taoTCConf),
		]
	    ),
	(productName, "throughput long string seq", "", [
		("1tp", taoReactiveConf + " throughput longString", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " throughput longString", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " throughput longString", taoTCConf),
		("tpc blocking", taoBlockingConf + " throughput longString", taoTCConf),
		]
	    ),
	(productName, "throughput struct seq", "", [
		("1tp", taoReactiveConf + " throughput struct", taoTPConf + " threadPool 1"),
		("4tp", taoReactiveConf + " throughput struct", taoTPConf + " threadPool 4"),
		("tpc", taoReactiveConf + " throughput struct", taoTCConf),
		("tpc blocking", taoBlockingConf + " throughput struct", taoTCConf),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
