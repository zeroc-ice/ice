// Copyright (c) ZeroC, Inc.

#ifndef ICE_APPLE_ASYNC_OPERATION_H
#define ICE_APPLE_ASYNC_OPERATION_H

#include <cassert>
#include <utility>

namespace IceInternal
{
    // The lifecycle of an asynchronous operation of a completion-based transceiver:
    //
    //     Idle -> Pending -> Completed -> Idle (consumed)
    //
    // The transceiver starts the operation, the transport's notification completes it with its result, and the
    // transceiver consumes the result when the thread pool finishes the operation. Completing reports whether the
    // operation was pending: exactly one notification completes it, and a later one, for example a further state
    // transition of the connection or the callback of an operation that cancellation already completed, leaves it
    // unchanged and must not post another completion. Cancellation completes a pending operation with a terminal
    // result through the same transition.
    //
    // The owner serializes the accesses, the operation is not thread-safe by itself.
    template<typename Result> class AsyncOperation
    {
    public:
        [[nodiscard]] bool pending() const { return _status == Status::Pending; }

        // Idle -> Pending.
        void start()
        {
            assert(_status == Status::Idle);
            _status = Status::Pending;
        }

        // Pending -> Completed. Returns false, and leaves the operation unchanged, when it was not pending.
        bool complete(Result result)
        {
            if (_status != Status::Pending)
            {
                return false;
            }
            _result = std::move(result);
            _status = Status::Completed;
            return true;
        }

        // Completed -> Idle. Returns the result.
        Result consume()
        {
            assert(_status == Status::Completed);
            _status = Status::Idle;
            return std::exchange(_result, Result{});
        }

    private:
        enum class Status
        {
            Idle,
            Pending,
            Completed
        };

        Status _status{Status::Idle};
        Result _result{};
    };
}

#endif
