# Copyright (c) ZeroC, Inc.

class InitializationData:
    """
    The attributes of this class are used to initialize a new communicator instance.

    Attributes
    ----------
    properties : Ice.Properties
        You can use the Ice.createProperties function to create a new property set.
    logger : Ice.Logger
        The logger to use for the communicator.
    threadStart : callable
        A callable that is invoked for each new Ice thread that is started.
    threadStop : callable
        A callable that is invoked when an Ice thread is stopped.
    executor : callable
        A callable that is invoked when Ice needs to execute an activity. The callable
        receives two arguments: a callable and an Ice.Connection object. The executor must
        eventually invoke the callable with no arguments.
    batchRequestInterceptor : callable
        A callable that will be invoked when a batch request is queued. The callable receives
        three arguments: a BatchRequest object, an integer representing the number of requests
        in the queue, and an integer representing the number of bytes consumed by the requests
        in the queue. The interceptor must eventually invoke the enqueue method on the BatchRequest object.
    eventLoopAdapter : Ice.EventLoopAdapter
        An event loop adapter used to run coroutines and wrap futures. If provided. This adapter is responsible for
        executing coroutines returned by Ice asynchronous dispatch methods and for wrapping Ice futures (from Ice
        Async APIs) into futures that can be awaited in the application's event loop.
    sliceLoader : callable
        A callable that creates class and exception instances from Slice type IDs. The callable receives a type ID
        or compact type ID as a string argument and returns a new instance of the class or exception identified by this
        ID. The implementation returns None when it cannot find the corresponding class.
    """

    def __init__(self):
        self.properties = None
        self.logger = None
        self.threadStart = None
        self.threadStop = None
        self.executor = None
        self.batchRequestInterceptor = None
        self.eventLoopAdapter = None
        self.sliceLoader = None
