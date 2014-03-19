# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

threadPoolOne = " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=1 --Ice.ThreadPool.Server.SizeWarn=2"
threadPoolFour = " --Ice.ThreadPool.Server.Size=4 --Ice.ThreadPool.Server.SizeMax=4 --Ice.ThreadPool.Server.SizeWarn=5"
threadPerConnection = " --Ice.ThreadPerConnection"
blocking = " --Ice.Blocking"

productName = "Ice"

testDefinitions = [
	(productName, "latency twoway", "latency", [
	        ("1tp", "twoway", threadPoolOne, [], ["1tp"]),
		("4tp", "twoway", threadPoolFour, [], ["4tp"]),
		("tpc", "twoway " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "latency twoway with 2k payload", "latency", [
	        ("1tp", "twoway", threadPoolOne, [("payload", "2000")], ["1tp"]),
		("4tp", "twoway", threadPoolFour, [("payload", "2000")], ["4tp"]),
		("tpc", "twoway " + threadPerConnection, threadPerConnection, [("payload", "2000")], ["tpc"]),
		]
	    ),
	(productName, "latency twoway with 10k payload", "latency", [
	        ("1tp", "twoway", threadPoolOne, [("payload", "10000")], ["1tp"]),
		("4tp", "twoway", threadPoolFour, [("payload", "10000")], ["4tp"]),
		("tpc", "twoway " + threadPerConnection, threadPerConnection, [("payload", "10000")], ["tpc"]),
		]
	    ),
	(productName, "latency oneway", "latency", [
		("1tp", "oneway", threadPoolOne, [], ["1tp"]),
		("4tp", "oneway", threadPoolFour, [], ["4tp"]), 
		("tpc", "oneway " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "latency oneway with 2k payload", "latency", [
	        ("1tp", "oneway", threadPoolOne, [("payload", "2000")], ["1tp"]),
		("4tp", "oneway", threadPoolFour, [("payload", "2000")], ["4tp"]),
		("tpc", "oneway " + threadPerConnection, threadPerConnection, [("payload", "2000")], ["tpc"]),
		]
	    ),
	(productName, "latency oneway with 10k payload", "latency", [
	        ("1tp", "oneway", threadPoolOne, [("payload", "10000")], ["1tp"]),
		("4tp", "oneway", threadPoolFour, [("payload", "10000")], ["4tp"]),
		("tpc", "oneway " + threadPerConnection, threadPerConnection, [("payload", "10000")], ["tpc"]),
		]
	    ),
	(productName, "latency oneway batch", "latency", [
		("1tp", "batch", threadPoolOne, [], ["1tp"]),
		("4tp", "batch", threadPoolFour, [], ["4tp"]), 
		("tpc", "batch " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "latency twoway AMI", "latency", [
		("1tp", "twoway ami", threadPoolOne, [], ["1tp"]),
		("4tp", "twoway ami", threadPoolFour, [], ["4tp"]), 
		("tpc", "twoway ami " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "latency twoway AMI with 2k payload", "latency", [
		("1tp", "twoway ami", threadPoolOne, [("payload", "2000")], ["1tp"]),
		("4tp", "twoway ami", threadPoolFour, [("payload", "2000")], ["4tp"]), 
		("tpc", "twoway ami " + threadPerConnection, threadPerConnection, [("payload", "2000")], ["tpc"]),
		]
	    ),
	(productName, "latency twoway AMI with 10k payload", "latency", [
		("1tp", "twoway ami", threadPoolOne, [("payload", "10000")], ["1tp"]),
		("4tp", "twoway ami", threadPoolFour, [("payload", "10000")], ["4tp"]), 
		("tpc", "twoway ami " + threadPerConnection, threadPerConnection, [("payload", "10000")], ["tpc"]),
		]
	    ),
	(productName, "throughput byte", "throughput", [
		("1tp", "byte", threadPoolOne, [], ["1tp"]),
		("4tp", "byte", threadPoolFour, [], ["4tp"]), 
		("tpc", "byte " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		("1tp (w/o zero copy)", "byte noZeroCopy", threadPoolOne, [], ["1tpnz"]),
		("4tp (w/o zero copy)", "byte noZeroCopy", threadPoolFour, [], ["4tpnz"]), 
		("tpc (w/o zero copy)", "byte noZeroCopy" + threadPerConnection, threadPerConnection, [], ["tpcnz"]),
		]
	    ),
	(productName, "throughput string sequence", "throughput", [
		("1tp", "stringSeq", threadPoolOne, [], ["1tp"]),
		("4tp", "stringSeq", threadPoolFour, [], ["4tp"]), 
		("tpc", "stringSeq " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "throughput long string sequence", "throughput", [
		("1tp", "longStringSeq", threadPoolOne, [], ["1tp"]),
		("4tp", "longStringSeq", threadPoolFour, [], ["4tp"]), 
		("tpc", "longStringSeq " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	(productName, "throughput struct sequence", "throughput", [
		("1tp", "structSeq", threadPoolOne, [], ["1tp"]),
		("4tp", "structSeq", threadPoolFour, [], ["4tp"]), 
		("tpc", "structSeq " + threadPerConnection, threadPerConnection, [], ["tpc"]),
		]
	    ),
	]

#
# These tests have been removed from the main run because there is no
# corresponding equivalent in TAO.
#
unusedTests = [
	(productName, "latency oneway batch with 2k payload", "latency", [
		("1tp", "batch", threadPoolOne, [("payload", "2000")]),
		("4tp", "batch", threadPoolFour, [("payload", "2000")]), 
		("tpc", "batch " + threadPerConnection, threadPerConnection, [("payload", "2000")]),
		]
	    ),
	(productName, "latency oneway batch with 10k payload", "latency", [
		("1tp", "batch", threadPoolOne, [("payload", "10000")]),
		("4tp", "batch", threadPoolFour, [("payload", "10000")]), 
		("tpc", "batch " + threadPerConnection, threadPerConnection, [("payload", "10000")]),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
