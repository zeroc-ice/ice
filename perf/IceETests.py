# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

productName = "IceE"

blocking = " --Ice.Blocking "

testDefinitions = [
	(productName, "latency twoway", "latency", [
		("tpc", "twoway", "", [], ["tpc"]),
		("tpc blocking", "twoway" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway", "latency", [
		("tpc", "oneway", "", [] , ["tpc"]),
		("tpc blocking", "oneway" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway batch", "latency", [
		("tpc", "batch", "", [], ["tpc"]),
		("tpc blocking", "batch" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency twoway with 2k payload", "latency", [
		("tpc", "twoway", "", [("payload", "2000")], ["tpc"]),
		("tpc blocking", "twoway" + blocking, "", [("payload", "2000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway with 2k payload", "latency", [
		("tpc", "oneway", "", [("payload", "2000")], ["tpc"]),
		("tpc blocking", "oneway" + blocking, "", [("payload", "2000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency twoway with 10k payload", "latency", [
		("tpc", "twoway", "", [("payload", "10000")], ["tpc"]),
		("tpc blocking", "twoway" + blocking, "", [("payload", "10000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "latency oneway with 10k payload", "latency", [
		("tpc", "oneway", "", [("payload", "10000")], ["tpc"]),
		("tpc blocking", "oneway" + blocking, "", [("payload", "10000")], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "throughput byte", "throughput", [
		("tpc", "byte", "", [], ["tpc"]),
		("tpc blocking", "byte" + blocking, "", [], ["tpcb"]),
		("tpc (w/o zero copy)", "byte noZeroCopy", "", [], ["tpc", "tpcnz"]),
		("tpc blocking (w/o zero copy)", "byte noZeroCopy" + blocking, "", [], ["tpc", "tpcb", "tpcnz"]),
		]
	    ),
	(productName, "throughput string sequence", "throughput", [
		("tpc", "stringSeq", "", [], ["tpc"]),
		("tpc blocking", "stringSeq" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "throughput long string sequence", "throughput", [
		("tpc", "longStringSeq", "", [], ["tpc"]),
		("tpc blocking", "longStringSeq" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	(productName, "throughput struct sequence", "throughput", [
		("tpc", "structSeq", "", [], ["tpcb"]),
		("tpc blocking", "structSeq" + blocking, "", [], ["tpc", "tpcb"]),
		]
	    ),
	]

#
# These tests have been removed from the main run because there is no
# corresponding equivalent in TAO.
#
unusedTests = [
	(productName, "latency oneway batch with 2k payload", "latency", [
		("tpc", "batch", "", [("payload", "2000")]),
		("tpc blocking", "batch" + blocking, "", [("payload", "2000")]),
		]
	    ),
	(productName, "latency oneway batch with 10k payload", "latency", [
		("tpc", "batch", "", [("payload", "10000")]),
		("tpc blocking", "batch" + blocking, "", [("payload", "10000")]),
		]
	    ),
	]

def getDefinitions():
    return testDefinitions
