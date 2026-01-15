# Copyright (c) ZeroC, Inc.

from __future__ import annotations

from dataclasses import dataclass
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Callable

    from .EventLoopAdapter import EventLoopAdapter
    from .IcePyTypes import BatchRequest, Connection
    from .Logger import Logger
    from .Properties import Properties
    from .UserException import UserException
    from .Value import Value


@dataclass
class InitializationData:
    """
    Represents a set of options that you can specify when initializing a communicator.

    Attributes
    ----------
    properties : Ice.Properties | None
        The properties for the communicator.
        If not ``None``, this corresponds to the object returned by the :func:`Communicator.getProperties` function.
    logger : Ice.Logger | None
        The logger for the communicator.
    threadStart : Callable[[], None] | None
        A :class:`Callable` that is invoked whenever the communicator starts a new thread.
    threadStop : Callable[[], None] | None
        A :class:`Callable` that is invoked whenever a thread created by the communicator is about to be destroyed.
    executor : Callable[[Callable[[], None], Connection], None] | None
        A :class:`Callable` that the communicator invokes to execute dispatches and async invocation callbacks.
        The callable receives two arguments: a callable and an Ice.Connection object.
        The executor must eventually invoke the callable with no arguments.
    batchRequestInterceptor : Callable[[Ice.BatchRequest, int, int], None] | None
        A :class:`Callable` that is invoked by the Ice runtime to enqueue a batch request.
        The callable receives three arguments: a BatchRequest object, an integer representing the number of requests
        currently in the queue, and an integer representing the number of bytes consumed by the requests in the queue.
        The interceptor must eventually invoke the enqueue function on the BatchRequest object.
    eventLoopAdapter : Ice.EventLoopAdapter | None
        An event loop adapter used to run coroutines and wrap futures. If provided, this adapter is responsible for
        executing coroutines returned by Ice asynchronous dispatch functions and for wrapping Ice futures (from Ice
        Async APIs) into futures that can be awaited in the application's event loop.
    sliceLoader : Callable[[str], Ice.Value | Ice.UserException | None] | None
        A :class:`Callable` used to create instances of Slice classes and user exceptions.
        Applications can supply a custom slice loader that the Ice runtime will use during unmarshaling.
        The callable receives one argument: a type ID or compact type ID (as a string) and returns a new instance of the
        corresponding class or exception, or ``None`` if no such class or exception could be found.
    """

    properties: Properties | None = None
    logger: Logger | None = None
    threadStart: Callable[[], None] | None = None
    threadStop: Callable[[], None] | None = None
    executor: Callable[[Callable[[], None], Connection], None] | None = None
    batchRequestInterceptor: Callable[[BatchRequest, int, int], None] | None = None
    eventLoopAdapter: EventLoopAdapter | None = None
    sliceLoader: Callable[[str], Value | UserException | None] | None = None


__all__ = ["InitializationData"]
