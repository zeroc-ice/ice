# Copyright (c) ZeroC, Inc.

import IcePy
import Ice.OperationMode_ice
import Ice.BuiltinSequences_ice

class Object:
    """
    The base class for servants.
    """

    def ice_isA(self, id, current):
        """
        Determine whether the target object supports the interface denoted by the given Slice type ID.

        Parameters
        ----------
        id : str
            The Slice type ID.
        current : Ice.Current
            The current context.

        Returns
        -------
        bool
            True if the target object supports the interface, False otherwise.
        """
        return id in self.ice_ids(current)

    def ice_ping(self, current):
        """
        A reachability test for the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.
        """
        pass

    def ice_ids(self, current):
        """
        Obtain the type IDs corresponding to the Slice interfaces that are supported by the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.

        Returns
        -------
        list of str
            A list of type IDs.
        """
        return [self.ice_id(current)]

    def ice_id(self, current):
        """
        Obtain the type ID corresponding to the most-derived Slice interface supported by the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.

        Returns
        -------
        str
            The type ID.
        """
        return "::Ice::Object"

    @staticmethod
    def ice_staticId():
        """
        Obtain the type ID of this Slice class or interface.

        Returns
        -------
        str
            The type ID.
        """
        return "::Ice::Object"

Object._op_ice_isA = IcePy.Operation(
    "ice_isA",
    "ice_isA",
    Ice.OperationMode.Idempotent,
    None,
    (),
    (((), IcePy._t_string, False, 0),),
    (),
    ((), IcePy._t_bool, False, 0),
    (),
)

Object._op_ice_ping = IcePy.Operation(
    "ice_ping",
    "ice_ping",
    Ice.OperationMode.Idempotent,
    None,
    (),
    (),
    (),
    None,
    (),
)

Object._op_ice_ids = IcePy.Operation(
    "ice_ids",
    "ice_ids",
    Ice.OperationMode.Idempotent,
    None,
    (),
    (),
    (),
    ((), Ice._t_StringSeq, False, 0),
    (),
)

Object._op_ice_id = IcePy.Operation(
    "ice_id",
    "ice_id",
    Ice.OperationMode.Idempotent,
    None,
    (),
    (),
    (),
    ((), IcePy._t_string, False, 0),
    (),
)
