# Copyright (c) ZeroC, Inc.

import inspect
import sys


def is_future(obj):
    return callable(getattr(obj, "add_done_callback", None))


def dispatch(cb, method, args):
    """
    Dispatch a request to the given servant method.

    This function is called by IcePy from an Ice server thread pool thread to dispatch a request to a servant method
    with the given arguments. The method's result is then sent back to IcePy using the provided callback.

    The method can return:
    - A direct result, which is immediately sent back.
    - A coroutine.
    - A future.

    If the result is a coroutine and the Ice communicator has a custom coroutine executor, the executor is used to
    execute the coroutine, and the resulting future is used to wait for the dispatch completion.

    Parameters
    ----------
    cb : IcePy.DispatchCallback
        The callback used to return the result or report an exception to IcePy.
    method : callable
        The servant method to invoke. This method is bound to the servant instance and takes the request parameters as
        arguments.
    args : list
        The request parameters.
    """

    # Invoke the servant method with the request parameters. Let exceptions propagate to the caller.
    result = method(*args)

    # If the result is a coroutine and the communicator has a custom coroutine executor, execute the coroutine using
    # the configured coroutine executor.
    if inspect.iscoroutine(result):
        assert len(args) > 0, "args must have at least one argument, current"
        current = args[-1]
        eventLoopAdapter = current.adapter.getCommunicator().eventLoopAdapter
        if eventLoopAdapter:
            result = eventLoopAdapter.runCoroutine(result)
            if not is_future(result):
                raise TypeError("The runCoroutine implementation must return a Future-like object")

    # If the result is a future, attach a done callback to handle dispatch completion.
    if is_future(result):

        def handle_future_result(future):
            try:
                cb.response(future.result())
            except Exception:
                cb.exception(sys.exc_info()[1])

        result.add_done_callback(handle_future_result)

    # If the result is a coroutine and no custom coroutine executor is available, run the coroutine synchronously in
    # the current thread using run_coroutine helper function.
    elif inspect.iscoroutine(result):
        run_coroutine(cb, result)

    # Otherwise, return the result directly.
    else:
        cb.response(result)


def run_coroutine(cb, coroutine, value=None, exception=None):
    """
    Run a coroutine until completion.

    This function is invoked by the `dispatch` function to execute coroutines when no custom coroutine executor is
    configured.

    Unlike a general-purpose executor, this function is specifically designed for handling coroutines produced by
    AMI calls nested in AMD dispatch.

    .. code-block:: python

        async def op(self, current):
            await self.prox.ice_pingAsync()
    """
    try:
        if exception:
            result = coroutine.throw(exception)
        else:
            result = coroutine.send(value)

        if is_future(result):

            def handle_future_result(future):
                try:
                    run_coroutine(cb, coroutine, value=future.result())
                except BaseException:
                    run_coroutine(cb, coroutine, exception=sys.exc_info()[1])

            # There is a potential recursive call here if the future is already completed.
            # However, synchronous completion of AMI calls is rare, and this executor is specifically
            # designed for AMI calls nested in AMD dispatch.
            result.add_done_callback(handle_future_result)
        else:
            raise RuntimeError(f"unexpected value of type {type(result)} returned by coroutine.send()")
    except StopIteration as ex:
        cb.response(ex.value)
    except BaseException:
        cb.exception(sys.exc_info()[1])
