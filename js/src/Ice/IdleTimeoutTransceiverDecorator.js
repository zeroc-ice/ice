// Copyright (c) ZeroC, Inc.

export class IdleTimeoutTransceiverDecorator {
    constructor(decoratee, connection, timer, idleTimeout, enableIdleCheck) {
        DEV: console.assert(idleTimeout > 0);

        this._decoratee = decoratee;
        this._idleTimeout = idleTimeout * 1000; // Convert seconds to milliseconds
        this._timer = timer;
        this._connection = connection;

        // _idleCheckEnabled is initially enableIdleCheck (by default, true) unlike C++/C#/Java.
        // Since JS supports only client connections, we know the connection will read at a minimum the initial
        // ValidateConnection message, and this reading will start or reset the read timer.
        this._idleCheckEnabled = enableIdleCheck;
    }

    setCallbacks(connectedCallback, bytesAvailableCallback, bytesWrittenCallback) {
        this._decoratee.setCallbacks(connectedCallback, bytesAvailableCallback, bytesWrittenCallback);
    }

    initialize(readBuffer, writeBuffer) {
        return this._decoratee.initialize(readBuffer, writeBuffer);
    }

    register() {
        this._decoratee.register();
    }

    unregister() {
        this._decoratee.unregister();
    }

    close() {
        this.cancelReadTimer();
        this.cancelWriteTimer();
        this._decoratee.close();
    }

    destroy() {
        this._decoratee.destroy();
    }

    write(buffer, bufferFullyWritten) {
        this.cancelWriteTimer();
        const completed = this._decoratee.write(buffer, bufferFullyWritten);
        if (completed) {
            // write completed
            this.rescheduleWriteTimer();
        }
        return completed;
    }

    read(buf, moreData) {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        this.rescheduleReadTimer();
        return this._decoratee.read(buf, moreData);
    }

    type() {
        return this._decoratee.type();
    }

    getInfo() {
        return this._decoratee.getInfo();
    }

    setBufferSize(rcvSize, sndSize) {
        this._decoratee.setBufferSize(rcvSize, sndSize);
    }

    toString() {
        return this._decoratee.toString();
    }

    scheduleHeartbeat() {
        // Reschedule because the connection establishment may have already written to the connection and scheduled a
        // heartbeat.
        this.rescheduleWriteTimer();
    }

    cancelReadTimer() {
        if (this._readTimerToken !== undefined) {
            this._timer.cancel(this._readTimerToken);
            this._readTimerToken = undefined;
        }
    }

    cancelWriteTimer() {
        if (this._writeTimerToken !== undefined) {
            this._timer.cancel(this._writeTimerToken);
            this._writeTimerToken = undefined;
        }
    }

    rescheduleReadTimer() {
        if (this._idleCheckEnabled) {
            this.cancelReadTimer();
            this._readTimerToken = this._timer.schedule(() => {
                this._connection.idleCheck(this._idleTimeout);
            }, this._idleTimeout);
        }
    }

    rescheduleWriteTimer() {
        this.cancelWriteTimer();
        this._writeTimerToken = this._timer.schedule(() => {
            this._connection.sendHeartbeat();
        }, this._idleTimeout / 2);
    }
}
