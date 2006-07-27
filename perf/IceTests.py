# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
		("1tp", "twoway", threadPoolOne),
		("4tp", "twoway", threadPoolFour),
		("tpc", "twoway " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "latency oneway", "latency", [
		("1tp", "oneway", threadPoolOne),
		("4tp", "oneway", threadPoolFour), 
		("tpc", "oneway " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "latency oneway batch", "latency", [
		("1tp (batch)", "batch", threadPoolOne),
		("4tp (batch)", "batch", threadPoolFour), 
		("tpc (batch)", "batch " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "latency twoway AMI", "latency", [
		("1tp (batch)", "twoway ami", threadPoolOne),
		("4tp (batch)", "twoway ami", threadPoolFour), 
		("tpc (batch)", "twoway ami " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "throughput byte", "throughput", [
		("1tp", "byte", threadPoolOne),
		("4tp", "byte", threadPoolFour), 
		("tpc", "byte " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "throughput string sequence", "throughput", [
		("1tp", "stringSeq", threadPoolOne),
		("4tp", "stringSeq", threadPoolFour), 
		("tpc", "stringSeq " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "throughput long string sequence", "throughput", [
		("1tp", "longStringSeq", threadPoolOne),
		("4tp", "longStringSeq", threadPoolFour), 
		("tpc", "longStringSeq " + threadPerConnection, threadPerConnection),
		]
	    ),
	(productName, "throughput struct sequence", "throughput", [
		("1tp", "structSeq", threadPoolOne),
		("4tp", "structSeq", threadPoolFour), 
		("tpc", "structSeq " + threadPerConnection, threadPerConnection),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
