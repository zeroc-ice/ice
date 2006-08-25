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
		("1tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway", taoTCConf, [], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency twoway", taoTCConf, [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency twoway with 2k payload", "", [
		("1tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 1", [("payload", "2000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 4", [("payload", "2000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway", taoTCConf,  [("payload", "2000")], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency twoway", taoTCConf,  [("payload", "2000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency twoway with 10k payload", "", [
		("1tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 1", [("payload", "10000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway", taoTPConf + " threadPool 4",  [("payload", "10000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway", taoTCConf,  [("payload", "10000")], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency twoway", taoTCConf,  [("payload", "10000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway", "", [
		("1tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " latency oneway", taoTCConf, [], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency oneway", taoTCConf, [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway batch", "", [
		("1tp (no batching avail)", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 1", [], ["1tp"]),
		("4tp (no batching avail)", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 4", [], ["4tp"]),
		("tpc (no batching avail)", taoReactiveConf + " latency oneway", taoTCConf, [], ["tpc"]),
		("tpc blocking (no batching avail)", taoBlockingConf + " latency oneway", taoTCConf, [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway with 2k payload", "", [
		("1tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 1", [("payload", "2000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 4",  [("payload", "2000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency oneway", taoTCConf,  [("payload", "2000")], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency oneway", taoTCConf,  [("payload", "2000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway with 10k payload", "", [
		("1tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 1", [("payload", "10000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency oneway", taoTPConf + " threadPool 4",  [("payload", "10000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency oneway", taoTCConf,  [("payload", "10000")], ["tpc"]),
		("tpc blocking", taoBlockingConf + " latency oneway", taoTCConf,  [("payload", "10000")], ["tpcb", "tpc"]),
		]
	    ),
	(productName, "latency twoway AMI", "", [
		("1tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway ami", taoTCConf, [], ["tpc"]),
		#("tpc blocking", taoBlockingConf + " latency twoway ami", taoTCConf, [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency twoway AMI with 2k payload", "", [
		("1tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 1", [("payload", "2000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 4", [("payload", "2000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway ami", taoTCConf, [("payload", "2000")], ["tpcb", "tpc"]),
		]
	    ),
	(productName, "latency twoway AMI with 10k payload", "", [
		("1tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 1", [("payload", "10000")], ["1tp"]),
		("4tp", taoReactiveConf + " latency twoway ami", taoTPConf + " threadPool 4", [("payload", "10000")], ["4tp"]),
		("tpc", taoReactiveConf + " latency twoway ami", taoTCConf, [("payload", "10000")], ["tpc"]),
		]
	    ),
	(productName, "throughput byte", "", [
		("1tp", taoReactiveConf + " throughput byte", taoTPConf + " threadPool 1", [], ["1tp"]),
		("1tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput byte",
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " throughput byte", taoTPConf + " threadPool 4", [], ["4tp"]),
		("4tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput byte", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " throughput byte", taoTCConf, [], ["tpc"]),
		("tpc blocking no memory pool", taoBlockingConf + " -ORBUseLocalMemoryPool 0 throughput byte", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "throughput string sequence", "", [
		("1tp", taoReactiveConf + " throughput string", taoTPConf + " threadPool 1", [], ["1tp"]),
		("1tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput string", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " throughput string", taoTPConf + " threadPool 4", [], ["4tp"]),
		("4tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput string", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " throughput string", taoTCConf, [], ["tpc"]),
		("tpc no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput string", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc"]),
		("tpc blocking", taoBlockingConf + " throughput string", taoTCConf, [], ["tpcb", "tpc"]),
		("tpc blocking no memory pool", taoBlockingConf + " -ORBUseLocalMemoryPool 0 throughput string", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpcb", "tpc"]),
		]
	    ),
	(productName, "throughput long string sequence", "", [
		("1tp", taoReactiveConf + " throughput longString", taoTPConf + " threadPool 1", [], ["1tp"]),
		("1tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput longString", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " throughput longString", taoTPConf + " threadPool 4", [], ["4tp"]),
		("4tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput longString", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " throughput longString", taoTCConf, [], ["tpc"]),
		("tpc no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput longString", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc"]),
		("tpc blocking", taoBlockingConf + " throughput longString", taoTCConf, [], ["tpc", "tpcb"]),
		("tpc blocking no memory pool", taoBlockingConf + " -ORBUseLocalMemoryPool 0 throughput longString", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "throughput struct sequence", "", [
		("1tp", taoReactiveConf + " throughput struct", taoTPConf + " threadPool 1", [], ["1tp"]),
		("1tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput struct", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 1", [], ["1tp"]),
		("4tp", taoReactiveConf + " throughput struct", taoTPConf + " threadPool 4", [], ["4tp"]),
		("4tp no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput struct", 
		    taoTPConf + " -ORBUseLocalMemoryPool 0 threadPool 4", [], ["4tp"]),
		("tpc", taoReactiveConf + " throughput struct", taoTCConf, [], ["tpc"]),
		("tpc no memory pool", taoReactiveConf + " -ORBUseLocalMemoryPool 0 throughput struct", 
		    taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc"]),
		("tpc blocking", taoBlockingConf + " throughput struct", taoTCConf, [], ["tpc", "tpcb"]),
		("tpc blocking no memory pool", taoBlockingConf + " -ORBUseLocalMemoryPool 0 throughput struct", taoTCConf + " -ORBUseLocalMemoryPool 0", [], ["tpc", "tpcb"]),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
