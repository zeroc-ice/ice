# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class InitializationData(object):
    """The attributes of this class are used to initialize a new
    communicator instance. The supported attributes are as follows:

    properties: An instance of Ice.Properties. You can use the
        Ice.createProperties function to create a new property set.

    logger: An instance of Ice.Logger.

    threadStart: A callable that is invoked for each new Ice thread that is started.

    threadStop: A callable that is invoked when an Ice thread is stopped.

    dispatcher: A callable that is invoked when Ice needs to dispatch an activity. The callable
        receives two arguments: a callable and an Ice.Connection object. The dispatcher must
        eventually invoke the callable with no arguments.

    batchRequestInterceptor: A callable that will be invoked when a batch request is queued.
        The callable receives three arguments: a BatchRequest object, an integer representing
        the number of requests in the queue, and an integer representing the number of bytes
        consumed by the requests in the queue. The interceptor must eventually invoke the
        enqueue method on the BatchRequest object.

    valueFactoryManager: An object that implements ValueFactoryManager."""

    def __init__(self):
        self.properties = None
        self.logger = None
        self.threadStart = None
        self.threadStop = None
        self.dispatcher = None
        self.batchRequestInterceptor = None
        self.valueFactoryManager = None
