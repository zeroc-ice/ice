- Fixed `Ice.Future.add_done_callback` and `Ice.InvocationFuture.add_sent_callback` in Ice for Python: when the future
  was already completed - or the request already sent - an exception raised by the callback propagated to the caller.
  Such an exception is now caught and logged with Python's `logging` module.
