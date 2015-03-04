#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, IceMX

#
# Formatting information for metrics maps. The tupple defines the
# field name, title, alignment ('<' for left alignement and '>' for
# right aligment) and width.
#
maps = {
    "Connection" : [
        ("id", "Connections", '<', 35), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("receivedBytes", "RxBytes", '>', 10),
        ("sentBytes", "TxBytes", '>', 10),
        ("averageLifetime", "Avg (s)", '>', 8) 
    ],
    "Invocation" : [
        ("id", "Invocations", '<', 39), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("retry", "Rtrs", '>', 5),
        ("", "Sz", '>', 5),
        ("", "RepSz", '>', 5),
        ("averageLifetime", "Avg (ms)", '>', 8) 
    ],
    "Dispatch" : [
        ("id", "Dispatch", '<', 40), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("size", "Sz", '>', 5),
        ("replySize", "RepSz", '>', 5),
        ("averageLifetime", "Avg (ms)", '>', 8) 
    ],
    "Thread" : [ 
        ("id", "Threads", '<', 25), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("inUseForIO", "IO", '>', 6),
        ("inUseForUser", "User", '>', 6),
        ("inUseForOther", "Other", '>', 6),
        ("averageLifetime", "Avg (s)", '>', 8) 
    ],
    "ConnectionEstablishment" : [ 
        ("id", "Connection Establishments", '<', 30), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("averageLifetime", "Avg (ms)", '>', 8) 
    ],
    "EndpointLookup" : [
        ("id", "Endpoint Lookups", '<', 30), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("averageLifetime", "Avg (ms)",'>', 8) 
    ],
    "Remote" : [
        ("id", "Invocations", '>', 39), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("", "", '>', 5),
        ("size", "Sz", '>', 5),
        ("replySize", "RepSz", '>', 5),
        ("averageLifetime", "Avg (ms)", '>', 8)
    ],
    "Session" : [
        ("id", "Sessions", '<', 15), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("forwardedClient", "FdCli", '>', 5),
        ("forwardedServer", "FdSrv", '>', 5),
        ("queuedClient", "QdCli", '>', 5),
        ("queuedServer", "QdSrv", '>', 5),
        ("overriddenClient", "OdCli", '>', 5),
        ("overriddenServer", "OdSrv", '>', 5),
        ("routingTableSize", "RT Sz", '>', 5),
        ("averageLifetime", "Avg (s)", '>', 8)
    ],
    "Topic" : [
        ("id", "Topics", '<', 30), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("published", "Published", '>', 9),
        ("forwarded", "Forwarded", '>', 9),
        ("averageLifetime", "Avg (s)", '>', 7)
    ],
    "Subscriber" : [
        ("id", "Subscribers", '<', 32), 
        ("current", "#", '>', 3),
        ("total", "Total", '>', 5),
        ("queued", "Queued", '>', 6),
        ("outstanding", "Pending", '>', 7),
        ("delivered", "Delivered", '>', 9),
        ("averageLifetime", "Avg (s)", '>', 8)
    ],
}

def metricsTitle():
    # Returns a lambda which returns the title of the field.
    return lambda mapName, field, title, align, width: ("{0:" + align + str(width) + "}").format(title)

def metricsSeparator(fill):
    # Returns a lambda which returns a separator of the size of the
    # field and using the specified fill character.
    return lambda mapName, field, title, align, width: ("{0:" + fill + align + str(width) + "}").format("")

def metricsField(metrics):
    def getField(mapName, field, title, width):
        if field == "":
            return ""
        elif field == "averageLifetime":
            #
            # Compute the average lifetime.
            #
            if metrics.total <= metrics.current:
                return "0.000"

            avg = metrics.totalLifetime / 1000.0 / (metrics.total - metrics.current)
            if title.find("(s)") > 0: # Display in seconds
                avg /= 1000.0
            return "%.3f" % avg

        else:
            v = str(metrics.__dict__[field])
            if len(v) > width:
                v = v[0:width - 3] + '...' # Truncates the field if it's too large.
            return v

    # Return a lambda which returns the formated metric value for the
    # given metrics object.
    return lambda mapName, field, title, align, width: \
        ("{0:" + align + str(width) + "}").format(getField(mapName, field, title, width))

def printMetrics(mapName, getValue, sep = '|', prefix = " "):
    #
    # This method prints a metrics, a line in the table. 
    #
    # It concatenates all the metric fields according to the format
    # described in the `maps' table defined above. The value of each
    # column is obtained by calling the provided getValue function.
    #
    fieldStr = [ prefix + sep ]
    for (field, title, align, width) in maps[mapName]:
        fieldStr.append(getValue(mapName, field, title, align, width) + sep)
    print("".join(fieldStr))

def printMetricsMap(admin, viewName, mapName, map):

    #
    # Print the table header.
    # 
    printMetrics(mapName, metricsSeparator('='), '+')
    printMetrics(mapName, metricsTitle())
    printMetrics(mapName, metricsSeparator('='), '+')

    #
    # Print the metrics map.
    #
    # For the invocation metrics map we also print the "Remote"
    # submaps embedded with each invocation metrics.
    #
    if mapName != "Invocation":
        # Print each metrics from the map
        for o in map:
            printMetrics(mapName, metricsField(o))
    else:
        first = True
        for o in map:
            if not first:
                printMetrics(mapName, metricsSeparator('-'), '+')
            printMetrics(mapName, metricsField(o))
            for so in o.remotes:
                printMetrics("Remote", metricsField(so))

    #
    # Print the table footer.
    #
    printMetrics(mapName, metricsSeparator('='), '+')

    #
    # Print metrics failure if any after the table.
    #
    failures = admin.getMapMetricsFailures(viewName, mapName)
    if len(failures) > 0:
        print("")
        print(" Failures:")
        for k in failures:
            for (exception, count) in k.failures.items():
                if len(k.id) > 0:
                    print("  - " + str(count) + ' ' + exception + " for `" + k.id + "'")
                else:
                    print("  - " + str(count) + ' ' + exception)
    print("")

def printMetricsView(admin, viewName, viewAndRefreshTime):

    (view, refreshTime) = viewAndRefreshTime

    #
    # Print each metrics map from the metrics view.
    #
    print("View: " + viewName)
    print("")
    for mapName, map in view.items():
        if mapName in maps and len(map) > 0:
            printMetricsMap(admin, viewName, mapName, map)
    print("")

class Client(Ice.Application):

    def usage(self):
        print("usage: " + sys.argv[0] + " dump | enable | disable [<view-name> [<map-name>]]" + """
To connect to the Ice administrative facility of an Ice process, you
should specify its endpoint(s) and instance name with the
`InstanceName' and `Endpoints' properties. For example:
    
 $ ./Metrics.py --Endpoints="tcp -p 12345 -h localhost" --InstanceName=Server dump

Commands:

  dump    Dump all the IceMX metrics views configured for the 
          process or if a specific view or map is specified,
          print only this view or map.

  enable  Enable all the IceMX metrics views configured for 
          the process or the provided view or map if specified.

  disable Disable all the IceMX metrics views configured for 
          the process or the provided view or map if specified.
""")

    def run(self, args):

        props = self.communicator().getProperties()
        args = props.parseCommandLineOptions("", args);

        if len(args) < 2 or len(args) > 6:
            self.usage()
            return 2

        command = args[1]
        viewName = None
        mapName = None
        if len(args) > 2:
            viewName = args[2]
            if len(args) > 3:
                mapName = args[3]

        try:
            #
            # Create the proxy for the Metrics admin facet.
            #
            proxyStr = "%s/admin -f Metrics:%s" % (props.getProperty("InstanceName"), props.getProperty("Endpoints"))
            metrics = IceMX.MetricsAdminPrx.checkedCast(self.communicator().stringToProxy(proxyStr));
            if not metrics:
                print(sys.argv[0] + ": invalid proxy `" + proxyStr + "'")
                return 1

            if command == "dump":
                (views, disabledViews) = metrics.getMetricsViewNames()
                if not viewName:
                    # Print all the enabled metrics views.
                    for v in views:
                        printMetricsView(metrics, v, metrics.getMetricsView(v))
                else:
                    # Ensure the view exists
                    if not viewName in views and not viewName in disabledViews:
                        print("unknown view `" + viewName + "', available views:")
                        print("enabled = " + str(views) + ", disabled = " + str(disabledViews))
                        return 0

                    # Ensure the view is enabled
                    if viewName in disabledViews:
                        print("view `" + viewName + "' is disabled")
                        return 0
                    
                    # Retrieve the metrics view and print it.
                    (view, refresh) = metrics.getMetricsView(viewName);                
                    if mapName:
                        if not mapName in view:
                            print("no map `" + mapName + "' in `" + viewName + "' view, available maps:")
                            print(str(view.keys()))
                            return 0
                        printMetricsMap(metrics, viewName, mapName, view[mapName])
                    else:
                        printMetricsView(metrics, viewName, (view, refresh))

            elif command == "enable":
                metrics.enableMetricsView(viewName)
            elif command == "disable":
                metrics.disableMetricsView(viewName)
            else:
                print("unknown command `" + command + "'")
                self.usage()
                return 2

        except Ice.ObjectNotExistException as ex:
            print("failed to get metrics from `%s':\n(the admin object doesn't exist, " + 
                  "are you sure to use the correct instance name?)") % (proxyStr)
            return 1
        except Ice.Exception as ex:
            print("failed to get metrics from `%s':\n%s") % (proxyStr, ex)
            return 1

        return 0

app = Client()

sys.exit(app.main(sys.argv))
