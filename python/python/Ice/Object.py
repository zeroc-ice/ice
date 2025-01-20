# Copyright (c) ZeroC, Inc.

import inspect
import sys
import IcePy
import Ice.OperationMode_ice
import Ice.BuiltinSequences_ice
from .Future import Future

class Object(object):
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

    def ice_ids(self, current=None):
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

    def ice_id(self, current=None):
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

    def _iceDispatch(self, cb, method, args):
        # Invoke the given servant method. Exceptions can propagate to the caller.
        result = method(*args)

        # Check for a future.
        if isinstance(result, Future) or callable(
            getattr(result, "add_done_callback", None)
        ):

            def handler(future):
                try:
                    cb.response(future.result())
                except Exception:
                    cb.exception(sys.exc_info()[1])

            result.add_done_callback(handler)
        elif inspect.iscoroutine(result):
            self._iceDispatchCoroutine(cb, result)
        else:
            cb.response(result)

    def _iceDispatchCoroutine(self, cb, coro, value=None, exception=None):
        try:
            if exception:
                result = coro.throw(exception)
            else:
                result = coro.send(value)

            if result is None:
                # The result can be None if the coroutine performs a bare yield (such as asyncio.sleep(0))
                cb.response(None)
            elif isinstance(result, Future) or callable(
                getattr(result, "add_done_callback", None)
            ):
                # If we've received a future from the coroutine setup a done callback to continue the dispatching
                # when the future completes.
                def handler(future):
                    try:
                        self._iceDispatchCoroutine(cb, coro, value=future.result())
                    except BaseException:
                        self._iceDispatchCoroutine(
                            cb, coro, exception=sys.exc_info()[1]
                        )

                result.add_done_callback(handler)
            else:
                raise RuntimeError(
                    "unexpected value of type "
                    + str(type(result))
                    + " provided by coroutine"
                )
        except StopIteration as ex:
            # StopIteration is raised when the coroutine completes.
            cb.response(ex.value)
        except BaseException:
            cb.exception(sys.exc_info()[1])

Object._op_ice_isA = IcePy.Operation(
    "ice_isA",
    Ice.OperationMode.Idempotent,
    False,
    None,
    (),
    (((), IcePy._t_string, False, 0),),
    (),
    ((), IcePy._t_bool, False, 0),
    (),
)
Object._op_ice_ping = IcePy.Operation(
    "ice_ping",
    Ice.OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    None,
    (),
)
Object._op_ice_ids = IcePy.Operation(
    "ice_ids",
    Ice.OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    ((), Ice._t_StringSeq, False, 0),
    (),
)
Object._op_ice_id = IcePy.Operation(
    "ice_id",
    Ice.OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    ((), IcePy._t_string, False, 0),
    (),
)
