# Copyright (c) ZeroC, Inc.

from dataclasses import dataclass

from IcePy import Connection

from .EncodingVersion import EncodingVersion
from .Identity import Identity
from .ObjectAdapter import ObjectAdapter
from .OperationMode import OperationMode


@dataclass(frozen=True)
class Current:
    """
    Provides information about an incoming request being dispatched.

    Attributes
    ----------
    adapter : Ice.ObjectAdapter
        The object adapter that received the request.
    con : IcePy.Connection | None
        The connection that received the request. It's None when the invocation and dispatch are collocated.
    id : Ice.Identity
        The identity of the target Ice object.
    facet : str
        The facet of the target Ice object.
    operation : str
        The name of the operation.
    mode : Ice.OperationMode
        The operation mode (idempotent or not).
    ctx : dict[str, str]
        The request context.
    requestId : int
        The request ID. 0 means the request is a one-way request.
    encoding : Ice.EncodingVersion
        The encoding of the request payload.
    """

    adapter: ObjectAdapter
    con: Connection | None
    id: Identity
    facet: str
    operation: str
    mode: OperationMode
    ctx: dict[str, str]
    requestId: int
    encoding: EncodingVersion
