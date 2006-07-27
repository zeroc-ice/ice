# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

productName = "IceE"

blocking = " --Ice.Blocking "

testDefinitions = [
	(productName, "latency twoway", "latency", [
		("tpc", "twoway", ""),
		("tpc blocking", "twoway" + blocking, ""),
		]
	    ),
	(productName, "latency oneway", "latency", [
		("tpc", "oneway", ""),
		("tpc blocking", "oneway" + blocking, ""),
		]
	    ),
	(productName, "latency oneway (batch)", "latency", [
		("tpc (batch)", "batch", ""),
		("tpc blocking (batch)", "batch" + blocking, ""),
		]
	    ),
	(productName, "throughput byte", "throughput", [
		("tpc", "byte", ""),
		("tpc blocking", "byte" + blocking, ""),
		]
	    ),
	(productName, "throughput string seq", "throughput", [
		("tpc", "stringSeq", ""),
		("tpc blocking", "stringSeq" + blocking, ""),
		]
	    ),
	(productName, "throughput long string seq", "throughput", [
		("tpc", "longStringSeq", ""),
		("tpc blocking", "longStringSeq" + blocking, ""),
		]
	    ),
	(productName, "throughput struct seq", "throughput", [
		("tpc", "structSeq", ""),
		("tpc blocking", "structSeq" + blocking, ""),
		]
	    ),
	(productName, "throughput byte (receive)", "throughput", [
		("tpc", "receive byte", ""),
		("tpc blocking", "receive byte" + blocking, ""),
		]
	    ),
	(productName, "throughput string seq (receive)", "throughput", [
		("tpc", "receive stringSeq", ""),
		("tpc blocking", "receive stringSeq" + blocking, ""),
		]
	    ),
	(productName, "throughput long string seq (receive)", "throughput", [
		("tpc", "receive longStringSeq", ""),
		("tpc blocking", "receive longStringSeq" + blocking, ""),
		]
	    ),
	(productName, "throughput struct seq (receive)", "throughput", [
		("tpc", "receive structSeq", ""),
		("tpc blocking", "receive structSeq" + blocking, ""),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
