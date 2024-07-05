# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import inspect
import sys
from .Future import Future

class Object(object):
    def ice_isA(self, id, current=None):
        """Determines whether the target object supports the interface denoted
        by the given Slice type id.
        Arguments:
            id The Slice type id
        Returns:
            True if the target object supports the interface, or False otherwise."""
        return id in self.ice_ids(current)

    def ice_ping(self, current=None):
        """A reachability test for the target object."""
        pass

    def ice_ids(self, current=None):
        """Obtains the type ids corresponding to the Slice interface
        that are supported by the target object.
        Returns:
            A list of type ids."""
        return [self.ice_id(current)]

    def ice_id(self, current=None):
        """Obtains the type id corresponding to the most-derived Slice
        interface supported by the target object.
        Returns:
            The type id."""
        return "::Ice::Object"

    @staticmethod
    def ice_staticId():
        """Obtains the type id of this Slice class or interface.
        Returns:
            The type id."""
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
