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
	(productName, "latency twoway 2k", "latency", [
		("tpc", "twoway", "", [("payload", "2000")]),
		("tpc blocking", "twoway" + blocking, "", [("payload", "2000")]),
		]
	    ),
	(productName, "latency oneway 2k", "latency", [
		("tpc", "oneway", "", [("payload", "2000")]),
		("tpc blocking", "oneway" + blocking, "", [("payload", "2000")]),
		]
	    ),
	(productName, "latency oneway 2k (batch)", "latency", [
		("tpc (batch)", "batch", "", [("payload", "2000")]),
		("tpc blocking (batch)", "batch" + blocking, "", [("payload", "2000")]),
		]
	    ),
	(productName, "latency twoway 10k", "latency", [
		("tpc", "twoway", "", [("payload", "10000")]),
		("tpc blocking", "twoway" + blocking, "", [("payload", "10000")]),
		]
	    ),
	(productName, "latency oneway 10k", "latency", [
		("tpc", "oneway", "", [("payload", "10000")]),
		("tpc blocking", "oneway" + blocking, "", [("payload", "10000")]),
		]
	    ),
	(productName, "latency oneway 10k (batch)", "latency", [
		("tpc (batch)", "batch", "", [("payload", "10000")]),
		("tpc blocking (batch)", "batch" + blocking, "", [("payload", "10000")]),
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
