# Copyright (c) ZeroC, Inc.

# Avoid evaluating annotations at function definition time.
from __future__ import annotations

from collections.abc import Awaitable, Sequence
from typing import TYPE_CHECKING

import IcePy

from .OperationMode import OperationMode
from .StringSeq import _Ice_StringSeq_t

if TYPE_CHECKING:
    from .Current import Current


class Object:
    """
    The base class for servants.
    """

    _ice_ids: Sequence[str] = ("::Ice::Object",)

    _op_ice_isA = IcePy.Operation(
        "ice_isA",
        "ice_isA",
        OperationMode.Idempotent,
        None,
        (),
        (((), IcePy._t_string, False, 0),),
        (),
        ((), IcePy._t_bool, False, 0),
        (),
    )

    _op_ice_ping = IcePy.Operation(
        "ice_ping",
        "ice_ping",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        None,
        (),
    )

    _op_ice_ids = IcePy.Operation(
        "ice_ids",
        "ice_ids",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        ((), _Ice_StringSeq_t, False, 0),
        (),
    )

    _op_ice_id = IcePy.Operation(
        "ice_id",
        "ice_id",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        ((), IcePy._t_string, False, 0),
        (),
    )

    def ice_isA(self, id: str, current: Current) -> bool | Awaitable[bool]:
        """
        Tests whether this object supports a specific Slice interface.

        Parameters
        ----------
        id : str
            The type ID of the Slice interface to test against.
        current : Current
            The Current object of the incoming request.

        Returns
        -------
        bool | Awaitable[bool]
            ``True`` if this object implements the Slice interface specified by ``typeId`` or implements a derived
            interface, ``False`` otherwise.
        """
        return id in self._ice_ids

    def ice_ping(self, current: Current) -> None | Awaitable[None]:
        """
        Tests whether this object can be reached.

        Parameters
        ----------
        current : Current
            The Current object of the incoming request.

        Returns
        -------
        None | Awaitable[None]
        """
        pass

    def ice_ids(self, current: Current) -> Sequence[str] | Awaitable[Sequence[str]]:
        """
        Returns the Slice interfaces supported by this object as a list of Slice type IDs.

        Parameters
        ----------
        current : Current
            The Current object of the incoming request.

        Returns
        -------
        Sequence[str] | Awaitable[Sequence[str]]
            The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        """
        return self._ice_ids

    def ice_id(self, current: Current) -> str | Awaitable[str]:
        """
        Returns the type ID of the most-derived Slice interface supported by this object.

        Parameters
        ----------
        current : Current
            The Current object of the incoming request.

        Returns
        -------
        str | Awaitable[str]
            The Slice type ID of the most-derived interface.
        """
        # Call ice_staticId() on self to get the value from the most-derived class.
        return self.ice_staticId()

    @staticmethod
    def ice_staticId() -> str:
        """
        Returns the type ID of the associated Slice interface.

        Returns
        -------
        str
            The return value is always ``"::Ice::Object"``.
        """
        return "::Ice::Object"
