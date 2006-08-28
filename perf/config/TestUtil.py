#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, platform, pprint

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

def isCygwin():

    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    if sys.platform[:6] == "cygwin":
        return 1
    else:
        return 0

def isWin32():

    if sys.platform == "win32" or isCygwin():
        return 1
    else:
        return 0

def getAdapterReady(serverPipe):

    output = serverPipe.readline().strip()

    if not output:
        print "failed!"
        sys.exit(1)

def printOutputFromPipe(pipe):

    while 1:

        c = pipe.read(1)

        if c == "":
            break

        os.write(1, c)

class Test :
    """Encapsulates the run of a test group. Manages the running of test
    cases and captures the results."""

    def __init__(self, product, test, directory = ""):
        self.product = product
        self.test = test
        if directory != "":
            self.directory = directory
        else:
            self.directory  = product

    def run(self, name, options, topics):

        sys.stdout.write(self.product + " " + self.test + " " + name + "... ")
        sys.stdout.flush()

        result = self.execute(options)
	if result != None:
	    key = 'latency'
	    if self.test.find('latency') == -1:
		key = 'throughput'

	    if result[key] > 0.0:
		print result[key]
	    else:
		print str(result[key]) + "(invalid data)"

	    result['product'] = self.product
	    result['test'] = self.test
	    result['variant'] = name
	    result['topics'] = topics
	return result
    
    def execute(self, options):

        return

def OrganizeResultsByProduct(rawData):
    """
    Set up a structure where all of the results for a given product are
    grouped under a map's key. This is useful for determining which
    tests have been run against a specific product.  
    """
    dataTree = {}
    for f in rawData:
	if dataTree.has_key(f['product']):
	    if dataTree[f['product']].has_key(f['test']):
		dataTree[f['product']][f['test']].append(f)
	    else:
		dataTree[f['product']][f['test']] = [f]
	else:
	    dataTree[f['product']] = {f['test'] : [f]}
    return dataTree

def OrganizeResultsByTest(rawData):
    """
    The organization is along the lines of a tree. 
           test-name
            /     \
         topicA   topicB
	 /  |   \       \
    prodA prodB prodC  prodA
      |     |            |
    res 0  res 0        res 0
    res 1   .           res 1
     .      .             .
     .                    .
     .                    .
    res n               res n
    """
    dataTree = {}
    for f in rawData:
	topics = f['topics'] 
	if dataTree.has_key(f['test']):
	    #
	    # Test key is already in the tree. So we iterate through our
	    # topics that these results apply to and insert them into
	    # the relevant topic subtrees, taking care to proper
	    # initialize the subtrees if they don't already exist. 
	    # 
	    for t in topics:
		#
		# Organizing topics
		#
		if dataTree[f['test']].has_key(t):
		    if dataTree[f['test']][t].has_key(f['product']):
			dataTree[f['test']][t][f['product']].append(f)
		    else:
			dataTree[f['test']][t][f['product']] = [f]
		else:
		    dataTree[f['test']][t] = {f['product'] : [f]}
	else:
	    #
	    # Test isn't in the tree yet. We insert the data into the
	    # tree, repeating the insertion for each topic. This leads
	    # to redundancy in the tree structure, but it is useful for
	    # producing comparisons later on.
	    #
	    dataTree[f['test']] = {} 
	    for t in topics:
		dataTree[f['test']][t] = { f['product'] : [f] } 
    return dataTree

def getMinKey(t):
    return min(t[1], t[2])

def getMaxKey(t):
    return max(t[1], t[2])

#
# TODO: This should create an object instance to organize the data
# instead of using lists and hashtables.
# 
def compileAndGroupResults(keyVariant, data):

    #
    # Gather variants:
    #
    initResults = [0,           0,          0]
    #	      totalLatency totalThroughput count 

    results = {}
    for d in data:
	if not results.has_key(d['variant']):
	    results[d['variant']] = list(initResults)
	results[d['variant']][0] += d['latency']
	results[d['variant']][1] += d['throughput']
	results[d['variant']][2] += 1 

    sys.stdout.flush()

    rest = []
    r = []
    for k in results.keys():
	avgLatency = results[k][0]  / results[k][2]
	avgThroughput = results[k][1]  / results[k][2]
	if k != keyVariant:
	    rest.append((avgLatency, avgThroughput, k))
	else:
	    r = [(avgLatency, avgThroughput, "")]

    #
    # There is a weird side effect at work here. The first entry is the
    # one that is used to compare to the exact variant match. The way
    # the above code works, this should always be the case if there is a
    # match. If there *ISN'T* an exact match on the variant then the
    # first result in the sequence will be used. This actually works
    # nicely in the case of latency oneway batch in Ice VS. TAO since
    # TAO doesn't support batching and consequently there isn't an exact
    # match.
    #
    r.extend(rest)
    return r

def PrintResults(rawResults, fileroot, products):
    resultsByProduct = OrganizeResultsByProduct(rawResults)
    #
    # Uncomment the following lines to produce a fairly readable representation of the structure.
    #
    # byProduct = file('byproduct.res', 'w+b')
    # byProduct.write(pprint.pformat(resultsByProduct))
    # byProduct.close()

    resultsByTest = OrganizeResultsByTest(rawResults)
    #
    # Uncomment the following lines to produce a fairly readable representation of the structure.
    #
    # byTests = file('bytest.res', 'w+b')
    # byTests.write(pprint.pformat(resultsByTest))
    # byTests.close()

    #
    # Compare vs Ice.
    #
    for A, B in products:
	print "Creating file %s.%s_vs_%s.csv" % (fileroot, A, B) 
	outputFile = file("%s.%s_vs_%s.csv" % (fileroot, A, B), "w+b")

	# 
	# If there are no results for this product comparison, skip it.
	#
	if not (resultsByProduct.has_key(A) and resultsByProduct.has_key(B)):
	    continue

	outputFile.write("%s versus %s\n\n" % (A, B))

	#
	# In an A vs B comparison, we are only interested in comparing the
	# tests that were run on A. We do not care about the tests that were
	# run on B but non on A. As long as resultsByProduct is organized
	# correctly, keyTests will contain all of the tests that were run on
	# product A.
	#
	keyTests = resultsByProduct[A]
	tests = list(keyTests.keys())
	tests.sort()

	for t in tests:
	    #
	    # If resultsByTest is organized correctly, r will now contain
	    # all the results that apply to this test, including all of the
	    # variants (sub-tests, configurations,
	    # whatever-you-want-to-call-them). See OrganizeResultsByTest for
	    # the actual organization of this structure.
	    #
	    r = resultsByTest[t]

	    #
	    # Sorting should have the affect of grouping the variants together
	    # in semi-logical groups.
	    #
	    topics = list(r.keys())
	    topics.sort()

	    #
	    # TODO: Do we want to do matching on 'similar' keys, not just
	    # exact matches. I don't know what our output format then
	    # becomes...
	    #
	    latency = t.find('latency') != -1
	    topicResults = []
	    annotatedResults = {}
	    annotations = {} 
	    stars = ""
	    for topic in topics:

		#
		# A vs B 
		#
		if r[topic].has_key(A) and r[topic].has_key(B):
		    keyVariant = r[topic][A][0]['variant'] 
		    line = [ keyVariant ]
		    data = r[topic][A]
		    totalLatency = 0.0
		    totalThroughput = 0.0
		    count = 0
		    for d in data:
			totalLatency += d['latency']
			totalThroughput += d['throughput']
			count += 1 
		    aLatency = totalLatency / count 
		    aThroughput = totalThroughput / count 

		    bResults = compileAndGroupResults(keyVariant, r[topic][B])
		    if len(bResults) > 1:
			annotatedResults[keyVariant] = bResults[1:]

		    lineNote = ""
		    if bResults[0][2] != "": 
			note = bResults[0][2]

			#
			# If keyVariant is part of the name, remove it.
			#
			if note.find(keyVariant) != -1:
			    note = note[len(keyVariant):].strip()
			note = note.strip()

			if not annotations.has_key(note):
			    stars = "%s*" % stars
			    annotations[note] = stars
			lineNote = annotations[note]

		    if latency:
			percentDiff =  (bResults[0][0] - aLatency)/aLatency
			line.extend([aLatency, bResults[0][0], percentDiff * 100, lineNote])
		    else:
			percentDiff =  (aThroughput - bResults[0][1])/bResults[0][1]
			line.extend([aThroughput, bResults[0][1], percentDiff * 100, lineNote])
		    topicResults.append(tuple(line))
	    #
	    # Format of the column headers in CSV format.
	    #
	    if len(topicResults) > 0:
		outputFile.write('"%s", %s, %s, %s\n' % (t, A, B, '% difference'))
		if latency:
		    outputFile.write('"", (ms), (ms)\n')
		else:
		    outputFile.write('"", (MB/s), (MB/s)\n')

		if latency:
		    topicResults.sort(None, getMinKey, False)
		else:
		    topicResults.sort(None, getMaxKey, True)

		for tr in topicResults:
		    outputFile.write('"%s","%f","%f","%f","%s"\n' % tr)
		    if annotatedResults.has_key(tr[0]):
			baseValue = tr[1]
			for additional in annotatedResults[tr[0]]:
			    aValue = 0.0
			    percentDiff = 0.0
			    if latency:
				aValue = additional[0]
				percentDiff = (aValue - baseValue) / baseValue * 100
			    else:
				aValue = additional[1]
				percentDiff = (baseValue - aValue) / aValue * 100

			    #
			    # If keyVariant is part of the name, remove it.
			    #
			    note = additional[2]
			    if note.find(tr[0]) != -1:
				note = note[len(tr[0]):]
			    note = note.strip()

			    lineNote = ""
			    if not annotations.has_key(note):
				stars = "%s*" % stars
				annotations[note] = stars
				lineNote = stars

			    lineNote = annotations[note]

			    outputFile.write('"","","%f","%f","%s"\n' % (aValue, percentDiff, lineNote))
		notes = []
		for a in annotations.keys():
		    notes.append("%s %s" % (annotations[a], a))
		notes.sort()
		for n in notes:
		    outputFile.write(n + "\n")
		outputFile.write('\n')

	outputFile.close() 
