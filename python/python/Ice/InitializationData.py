# Copyright (c) ZeroC, Inc.

from collections.abc import Callable
from dataclasses import dataclass

from IcePy import BatchRequest, Connection
from .EventLoopAdapter import EventLoopAdapter
from .Logger import Logger
from .Properties import Properties
from .UserException import UserException
from .Value import Value

@dataclass
class InitializationData:
    """
    The attributes of this class are used to initialize a new communicator instance.

    Attributes
    ----------
    properties : Ice.Properties | None
        You can use the Ice.createProperties function to create a new property set.
    logger : Ice.Logger | None
        The logger to use for the communicator.
    threadStart : Callable[[], None] | None
        A callable that is invoked for each new Ice thread that is started.
    threadStop : Callable[[], None] | None
        A callable that is invoked when an Ice thread is stopped.
    executor : Callable[[Callable[[], None], Connection], None] | None
        A callable that is invoked when Ice needs to execute an activity. The callable
        receives two arguments: a callable and an Ice.Connection object. The executor must
        eventually invoke the callable with no arguments.
    batchRequestInterceptor : Callable[[BatchRequest, int, int], None] | None
        A callable that will be invoked when a batch request is queued. The callable receives
        three arguments: a BatchRequest object, an integer representing the number of requests
        in the queue, and an integer representing the number of bytes consumed by the requests
        in the queue. The interceptor must eventually invoke the enqueue method on the BatchRequest object.
    eventLoopAdapter : Ice.EventLoopAdapter | None
        An event loop adapter used to run coroutines and wrap futures. If provided. This adapter is responsible for
        executing coroutines returned by Ice asynchronous dispatch methods and for wrapping Ice futures (from Ice
        Async APIs) into futures that can be awaited in the application's event loop.
    sliceLoader : Callable[[str], Value | UserException | None] | None
        A callable that creates class and exception instances from Slice type IDs. The callable receives a type ID
        or compact type ID as a string argument and returns a new instance of the class or exception identified by this
        ID. The implementation returns None when it cannot find the corresponding class.
    """

    properties: Properties | None = None
    logger: Logger | None = None
    threadStart: Callable[[], None] | None = None
    threadStop: Callable[[], None] | None = None
    executor: Callable[[Callable[[], None], Connection], None] | None = None
    batchRequestInterceptor: Callable[[BatchRequest, int, int], None] | None = None
    eventLoopAdapter: EventLoopAdapter | None = None
    sliceLoader: Callable[[str], Value | UserException | None] | None = None
