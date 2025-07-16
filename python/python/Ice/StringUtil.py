# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import threading

_repr_running = threading.local()


def safe_repr(obj: object) -> str:
    if not hasattr(_repr_running, "set"):
        _repr_running.set = set()

    obj_id = id(obj)
    if obj_id in _repr_running.set:
        return "..."

    _repr_running.set.add(obj_id)
    try:
        return repr(obj)
    finally:
        _repr_running.set.remove(obj_id)


def format_fields(**fields: object) -> str:
    return ", ".join(f"{k}={safe_repr(v)}" for k, v in fields.items())

__all__ = ["format_fields"]
