#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, pickle, platform

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

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

class ValuesMeanAndBest :

    def calcMeanAndBest(self, values):

        if len(values) == 0:
            return (0, 0)

        values.sort()
        values = values[0:len(values) / 2 + 1]
        best = values[0]
        
        mean = 0.0
        for r in values:
            mean += r
            
        mean /= len(values)
            
        return (mean, best)


def splitName(name):
    return (name.split()[0], name[len(name.split()[0]):].strip())

def compTest((k1, v1), (k2, v2)):

    m1 = 0.0
    for m in v1.itervalues():
        m1 += m
    m1 /= len(v1)
        
    m2 = 0.0
    for m in v2.itervalues():
        m2 += m
    m2 /= len(v2)

    if m1 < m2:
        return -1
    elif m2 > m1:
        return 1
    else:
        return 0
     
class TestResults :

    def __init__(self, test):

        self.test = test
        self.results = { }
        self.names = [ ]
        self.products = [ ]
        self.reference = ""

    def add(self, product, name, result):

        if not product in self.products:
            self.products.append(product)

        if not name in self.names:
            self.names.append(name)

        if not self.results.has_key(name):
            self.results[name] = { }

        if not self.results[name].has_key(product):
            self.results[name][product] = [ ]
            
        self.results[name][product].append(result)

    def merge(self, result):

        for n in result.names:
            for p in result.products:
                for i in result.results[n][p]:
                    self.add(p, n, i)

    def remove(self, expr):

        for n in self.names:
            for p in self.products:
                if expr.match(p + " " + self.test + " " + n):
                    if self.results.has_key(n) and self.results[n].has_key(p):
                        self.results[n].pop(p)
                        if len(self.results[n]) == 0:
                            self.results.pop(n)
                        print "removed " + p + " " + self.test + " " + n

    def convert(self, function):

        for x in self.results.iterkeys():
            for y in self.results[x].iterkeys():
                for i in range(0, len(self.results[x][y])):
                    self.results[x][y][i] = function.convert(self.results[x][y][i])

    def addToResults(self, results, tests, names, products, id, function):

        if len(self.results) == 0:
            return

        if not self.test in tests:
            tests.append(self.test)

        if not results.has_key(self.test):
            results[self.test] = { } 

        for n in self.names:
 
            (mn, sn) = splitName(n);
            if not results[self.test].has_key(mn):
                results[self.test][mn] = { }

            if not results[self.test][mn].has_key(sn):
                results[self.test][mn][sn] = { }
                    
            if not n in names:
                names.append(n)

            if self.results.has_key(n):
                
                for p in self.products:
                    
                    if self.results[n].has_key(p):
                        
                        if not results[self.test][mn][sn].has_key(p):
                            results[self.test][mn][sn][p] = { }

                        if not p in products:
                            products.append(p)

                        (mean, best) = function.calcMeanAndBest(self.results[n][p])

                        results[self.test][mn][sn][p][id] = mean

class HostResults :

    def __init__(self, hostname, outputFile):

        self.id = hostname + " " + platform.system()
        self.outputFile = outputFile
        self.tests = [ ]
        self.results = { }

    def add(self, product, test, name, result):

        if not self.results.has_key(test):
            self.results[test] = TestResults(test)

        if not test in self.tests:
            self.tests.append(test)

        self.results[test].add(product, name, result)

        f = file(self.outputFile, 'w')
        pickle.dump(self, f);
        f.close()

    def merge(self, results):

        for t in results.tests:

            if not t in self.tests:
                self.tests.append(t)

            if not self.results.has_key(t):
                self.results[t] = TestResults(t)

            self.results[t].merge(results.results[t])

    def remove(self, expr):

        for r in self.results.itervalues():
            r.remove(expr)

    def save(self, outputFile):

        if outputFile == "":
            outputFile = self.outputFile

        f = file(outputFile, 'w')
        pickle.dump(self, f);
        f.close()        

    def convert(self, function):

        for t in self.tests:
            self.results[t].convert(function)
        
    def addToResults(self, results, tests, names, products, hosts, function):

        if not self.id in hosts:
            hosts.append(self.id)

        for t in self.tests:
            self.results[t].addToResults(results, tests, names, products, self.id, function)
        
class AllResults :

    def __init__(self):
        self.results = { }

    def add(self, results):

        if self.results.has_key(results.id):
            self.results[results.id].merge(results)
        else:
            self.results[results.id] = results

    def remove(self, expr):

        for result in self.results.itervalues():
            result.remove(expr)

    def saveAll(self, outputFile):

        if outputFile != "" and len(self.results) > 1:
            print "You can't use --output with input file from different host/operating system"
            return

        for result in self.results.itervalues():
            result.save(outputFile)

    def convert(self, function):

        for r in self.results.itervalues():
            r.convert(function)
    
    def printAll(self, function, csv):

        results = { }
        tests = [ ]
        names = [ ]
        hosts = [ ]
        products = [ ]
        for r in self.results.itervalues():
            r.addToResults(results, tests, names, products, hosts, function)

        if csv:
            self.printAllAsCsv(results, tests, names, hosts, products)
        else:
            self.printAllAsText(results, tests, names, hosts, products)

    def printAllAsCsv(self, results, tests, names, hosts, products):

        print "Test, Configuration, ",
        for product in products:
            for host in hosts:
                print product + " " + host + ",",
        print ""

        for test in tests:
            for n in names:
                (name, subname) = splitName(n)
                if results[test].has_key(name) and results[test][name].has_key(subname):
                    print test + "," + name + "," + subname + ",",
                    for product in products:
                        if results[test][name][subname].has_key(product):
                            for host in hosts:
                                if not results[test][name][subname][product].has_key(host):
                                    print ",",
                                else:
                                    m = results[test][name][subname][product][host]
                                    print  str(m) + ",",
                        else:
                            print ",",

                    print ""

    def printAllAsText(self, results, tests, names, hosts, products):

        for test in tests:
            print test + ": "
            sep = "==========================="
            print sep[0:len(test)]
            print ""
            
            maxLen = 0
            for name in names:
                for product in products:
                    if maxLen < len(product + " " + name):
                        maxLen = len(product + " " + name)

            print (" %-" + str(maxLen) + "s") % "Test",
            for host in hosts:
                print "| %-18s" % host[0:18],
            print ""

            print (" %-" + str(maxLen) + "s") % ("-------------------------------------------"[0:maxLen]),
            for host in hosts:
                print "+-------------------",
            print ""

            namesWithSubnames = { }
            for n in names:
                (name, subname) = splitName(n)
                if not namesWithSubnames.has_key(name):
                    namesWithSubnames[name] = [ ]
                namesWithSubnames[name].append(subname)

            sortedNames = namesWithSubnames.keys();
            sortedNames.sort()
            for name in sortedNames:
                t = [ ]
                bestResults = { }
                for subname in namesWithSubnames[name]:
                    if results[test].has_key(name) and results[test][name].has_key(subname):
                        for product in products:
                            if results[test][name][subname].has_key(product):
                                k = product + " " + name + " " + subname
                                v = results[test][name][subname][product]
                                t.append((k,v))
                                for (h, r) in v.iteritems():
                                    if not bestResults.has_key(h):
                                        bestResults[h] = r
                                    elif r < bestResults[h]:
                                        bestResults[h] = r
                                    
                if len(t) > 0:

                    t.sort(compTest)
                    for (k, v) in t:
                        print (" %-" + str(maxLen) + "s") % k,
                        for host in hosts:
                            if not v.has_key(host):
                                print "| %-18s" % "",
                            elif v[host] / bestResults[host] >= 10.0:
                                print "| %10.6f (%-#1.2f)" % (v[host], v[host] / bestResults[host]),
                            else:
                                print "| %10.6f (%-#1.2f) " % (v[host], v[host] / bestResults[host]),
                        print ""

                    print (" %-" + str(maxLen) + "s") % ("-------------------------------------------"[0:maxLen]),
                    for host in hosts:
                        print "+-------------------",
                    print ""
            
            print ""
        
class Test :

    def __init__(self, expr, results, i, product, test, directory = ""):
        self.expr = expr
        self.results = results
        self.iteration = i
        self.product = product
        self.test = test
        if directory != "":
            self.directory = directory
        else:
            self.directory  = product

    def run(self, name, options):

        match = len(self.expr) == 0
        for e in self.expr:
            if e.match(self.product + " " + self.test + " " + name):
                match = True
                break;

        if not match:
            return

        print str(self.iteration) + ": " + self.product + " " + self.test + " " + name + "...",
        sys.stdout.flush()

        result = self.execute(options)
        if result > 0.0:
            self.results.add(self.product, self.test, name, result)
            try:
                (m1, m5, m15) = os.getloadavg()
                print "(load = " + str(m1) + ") " + str(result)
            except:
                print result
        else:
            print "invalid"
    
    def execute(self, options):

        return

    
