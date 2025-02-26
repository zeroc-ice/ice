# Copyright (c) ZeroC, Inc.

class InitializationData(object):
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
    coroutineExecutor : callable
        A callable used to execute coroutines returned by servant dispatch methods.
        It takes a single argument—the coroutine to execute—and must return a Future-like object to track its
        completion.

        .. code-block:: python

        import asyncio
        import Ice

        loop = asyncio.get_event_loop()

        def coroutineExecutor(coroutine):
            return asyncio.run_coroutine_threadsafe(coroutine, loop)

        initData = Ice.InitializationData()
        initData.coroutineExecutor = coroutineExecutor

        with Ice.initialize(initData) as communicator:
            # ...
    """

    def __init__(self):
        self.properties = None
        self.logger = None
        self.threadStart = None
        self.threadStop = None
        self.executor = None
        self.batchRequestInterceptor = None
        self.coroutineExecutor = None

    __module__ = "Ice"
