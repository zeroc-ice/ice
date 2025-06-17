// Copyright (c) ZeroC, Inc.

import { Instance, StateDestroyInProgress, StateDestroyed } from "./Instance.js";
import { DefaultsAndOverrides } from "./DefaultsAndOverrides.js";
import { EndpointFactoryManager } from "./EndpointFactoryManager.js";
import { ImplicitContext } from "./ImplicitContext.js";
import { LocatorManager } from "./LocatorManager.js";
import { ObjectAdapterFactory } from "./ObjectAdapterFactory.js";
import { OutgoingConnectionFactory } from "./OutgoingConnectionFactory.js";
import { Properties } from "./Properties.js";
import { ReferenceFactory } from "./ReferenceFactory.js";
import { RetryQueue } from "./RetryQueue.js";
import { RouterManager } from "./RouterManager.js";
import { Timer } from "./Timer.js";
import { TraceLevels } from "./TraceLevels.js";
import { LocalException } from "./LocalException.js";
import { CommunicatorDestroyedException, InitializationException, ParseException } from "./LocalExceptions.js";
import { getProcessLogger } from "./ProcessLogger.js";
import { ToStringMode } from "./ToStringMode.js";
import { ProtocolInstance } from "./ProtocolInstance.js";
import { TcpEndpointFactory } from "./TcpEndpointFactory.js";
import { WSEndpointFactory } from "./WSEndpointFactory.js";
import { Promise } from "./Promise.js";
import { ConnectionOptions } from "./ConnectionOptions.js";
import { StringUtil } from "./StringUtil.js";
import { FileLogger } from "./FileLogger.js";

import { Ice as Ice_Router } from "./Router.js";
const { RouterPrx } = Ice_Router;
import { Ice as Ice_Locator } from "./Locator.js";
const { LocatorPrx } = Ice_Locator;

import { Ice as Ice_EndpointTypes } from "./EndpointTypes.js";
const { TCPEndpointType, WSEndpointType, SSLEndpointType, WSSEndpointType } = Ice_EndpointTypes;

Instance.prototype.initializationData = function () {
    // No check for destruction. It must be possible to access the initialization data after destruction.
    //
    // This value is immutable.
    return this._initData;
};

Instance.prototype.traceLevels = function () {
    // This value is immutable.
    DEV: console.assert(this._traceLevels !== null);
    return this._traceLevels;
};

Instance.prototype.defaultsAndOverrides = function () {
    // This value is immutable.
    DEV: console.assert(this._defaultsAndOverrides !== null);
    return this._defaultsAndOverrides;
};

Instance.prototype.routerManager = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._routerManager !== null);
    return this._routerManager;
};

Instance.prototype.locatorManager = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._locatorManager !== null);
    return this._locatorManager;
};

Instance.prototype.referenceFactory = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._referenceFactory !== null);
    return this._referenceFactory;
};

Instance.prototype.outgoingConnectionFactory = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._outgoingConnectionFactory !== null);
    return this._outgoingConnectionFactory;
};

Instance.prototype.objectAdapterFactory = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._objectAdapterFactory !== null);
    return this._objectAdapterFactory;
};

Instance.prototype.retryQueue = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._retryQueue !== null);
    return this._retryQueue;
};

Instance.prototype.timer = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._timer !== null);
    return this._timer;
};

Instance.prototype.endpointFactoryManager = function () {
    if (this._state === StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    DEV: console.assert(this._endpointFactoryManager !== null);
    return this._endpointFactoryManager;
};

Instance.prototype.messageSizeMax = function () {
    // This value is immutable.
    return this._messageSizeMax;
};

Instance.prototype.batchAutoFlushSize = function () {
    // This value is immutable.
    return this._batchAutoFlushSize;
};

Instance.prototype.classGraphDepthMax = function () {
    // This value is immutable.
    return this._classGraphDepthMax;
};

Instance.prototype.toStringMode = function () {
    // this value is immutable
    return this._toStringMode;
};

Instance.prototype.getImplicitContext = function () {
    return this._implicitContext;
};

Instance.prototype.setDefaultLocator = function (locator) {
    if (this._state == StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    this._referenceFactory = this._referenceFactory.setDefaultLocator(locator);
};

Instance.prototype.setDefaultRouter = function (router) {
    if (this._state == StateDestroyed) {
        throw new CommunicatorDestroyedException();
    }

    this._referenceFactory = this._referenceFactory.setDefaultRouter(router);
};

Instance.prototype.setLogger = function (logger) {
    this._initData.logger = logger;
};

Instance.prototype.finishSetup = function (communicator) {
    try {
        if (this._initData.properties === null) {
            this._initData.properties = new Properties();
        }

        this._clientConnectionOptions = new ConnectionOptions(
            this._initData.properties.getIcePropertyAsInt("Ice.Connection.Client.ConnectTimeout"),
            this._initData.properties.getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout"),
            this._initData.properties.getIcePropertyAsInt("Ice.Connection.Client.IdleTimeout"),
            this._initData.properties.getIcePropertyAsInt("Ice.Connection.Client.EnableIdleCheck") > 0,
            this._initData.properties.getIcePropertyAsInt("Ice.Connection.Client.InactivityTimeout"),
        );

        const programName = this._initData.properties.getIceProperty("Ice.ProgramName");
        const logFile = this._initData.properties.getIceProperty("Ice.LogFile");
        if (logFile.length > 0) {
            if (FileLogger === null) {
                throw new InitializationException("Ice.LogFile property is not supported in Web Browsers");
            }
            this._initData.logger = new FileLogger(programName, logFile);
        }

        if (this._initData.logger === null) {
            this._initData.logger = getProcessLogger(programName);
        }

        this._traceLevels = new TraceLevels(this._initData.properties);

        this._defaultsAndOverrides = new DefaultsAndOverrides(this._initData.properties, this._initData.logger);

        // The maximum size of an Ice protocol message in bytes. This is limited to 0x7fffffff, which corresponds to
        // the maximum value of a 32-bit signed integer.
        const messageSizeMaxUpperLimit = 0x7fffffff;
        let messageSizeMax = this._initData.properties.getIcePropertyAsInt("Ice.MessageSizeMax");
        if (messageSizeMax > messageSizeMaxUpperLimit / 1024) {
            throw new InitializationException(
                `Ice.MessageSizeMax '${messageSizeMax}' is too large, it must be less than or equal to '${messageSizeMaxUpperLimit / 1024}' KiB`,
            );
        } else if (messageSizeMax < 1) {
            this._messageSizeMax = messageSizeMaxUpperLimit;
        } else {
            // The property is specified in kibibytes (KiB); _messageSizeMax is stored in bytes.
            this._messageSizeMax = messageSizeMax * 1024;
        }

        let batchAutoFlushSize = this._initData.properties.getIcePropertyAsInt("Ice.BatchAutoFlushSize");
        if (batchAutoFlushSize > messageSizeMaxUpperLimit / 1024) {
            throw new InitializationException(
                `Ice.BatchAutoFlushSize '${batchAutoFlushSize}' is too large, it must be less than or equal to '${messageSizeMaxUpperLimit / 1024}' KiB`,
            );
        } else if (batchAutoFlushSize < 1) {
            this._batchAutoFlushSize = messageSizeMaxUpperLimit;
        } else {
            // The property is specified in kibibytes (KiB); _batchAutoFlushSize is stored in bytes.
            this._batchAutoFlushSize = batchAutoFlushSize * 1024;
        }

        let classGraphDepthMax = this._initData.properties.getIcePropertyAsInt("Ice.ClassGraphDepthMax");
        if (classGraphDepthMax < 1) {
            this._classGraphDepthMax = 0x7fffffff;
        } else {
            this._classGraphDepthMax = classGraphDepthMax;
        }

        const toStringModeStr = this._initData.properties.getIceProperty("Ice.ToStringMode");
        if (toStringModeStr === "ASCII") {
            this._toStringMode = ToStringMode.ASCII;
        } else if (toStringModeStr === "Compat") {
            this._toStringMode = ToStringMode.Compat;
        } else if (toStringModeStr !== "Unicode") {
            throw new InitializationException(
                `illegal value '${toStringModeStr}' in property Ice.ToStringMode; expected 'Unicode', 'ASCII', or 'Compat'`,
            );
        }

        this._implicitContext = ImplicitContext.create(this._initData.properties.getIceProperty("Ice.ImplicitContext"));

        this._routerManager = new RouterManager();

        this._locatorManager = new LocatorManager(this._initData.properties);

        this._referenceFactory = new ReferenceFactory(this, communicator);

        this._endpointFactoryManager = new EndpointFactoryManager(this);

        const tcpInstance = new ProtocolInstance(this, TCPEndpointType, "tcp", false);
        const tcpEndpointFactory = new TcpEndpointFactory(tcpInstance);
        this._endpointFactoryManager.add(tcpEndpointFactory);

        const wsInstance = new ProtocolInstance(this, WSEndpointType, "ws", false);
        const wsEndpointFactory = new WSEndpointFactory(wsInstance, tcpEndpointFactory.clone(wsInstance));
        this._endpointFactoryManager.add(wsEndpointFactory);

        const sslInstance = new ProtocolInstance(this, SSLEndpointType, "ssl", true);
        const sslEndpointFactory = new TcpEndpointFactory(sslInstance);
        this._endpointFactoryManager.add(sslEndpointFactory);

        const wssInstance = new ProtocolInstance(this, WSSEndpointType, "wss", true);
        const wssEndpointFactory = new WSEndpointFactory(wssInstance, sslEndpointFactory.clone(wssInstance));
        this._endpointFactoryManager.add(wssEndpointFactory);

        this._outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

        this._objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

        this._retryQueue = new RetryQueue(this);
        const retryIntervals = this._initData.properties.getIcePropertyAsList("Ice.RetryIntervals");
        if (retryIntervals.length > 0) {
            this._retryIntervals = [];

            for (let i = 0; i < retryIntervals.length; i++) {
                let v;
                try {
                    v = StringUtil.toInt32(retryIntervals[i]);
                } catch {
                    throw new ParseException(
                        `invalid value '${retryIntervals[i]}' in property Ice.RetryIntervals; expected a list of integers`,
                    );
                }
                // If -1 is the first value, no retry and wait intervals.
                if (i === 0 && v === -1) {
                    break;
                }

                this._retryIntervals[i] = v > 0 ? v : 0;
            }
        } else {
            this._retryIntervals = [0];
        }

        this._timer = new Timer(this._initData.logger);

        const router = communicator.propertyToProxy("Ice.Default.Router");
        if (router !== null) {
            this._referenceFactory = this._referenceFactory.setDefaultRouter(new RouterPrx(router));
        }

        const loc = communicator.propertyToProxy("Ice.Default.Locator");
        if (loc !== null) {
            this._referenceFactory = this._referenceFactory.setDefaultLocator(new LocatorPrx(loc));
        }
    } catch (ex) {
        if (ex instanceof LocalException) {
            this.destroy().catch(err => {
                if (this._initData.logger !== null) {
                    this._initData.logger.warning(err);
                } else {
                    console.error(err);
                }
            });
        }
        throw ex;
    }
};

Instance.prototype.destroy = async function () {
    if (this._state == StateDestroyInProgress) {
        // Destroy is in progress, wait for it to be done. This is necessary in case destroy() is called multiple times.
        DEV: console.assert(this._destroyPromise !== null);
        return this._destroyPromise;
    } else if (this._state == StateDestroyed) {
        // Already destroyed.
        return;
    }
    this._state = StateDestroyInProgress;
    this._destroyPromise = new Promise();

    try {
        if (this._outgoingConnectionFactory !== null) {
            this._outgoingConnectionFactory.destroy();
        }

        if (this._objectAdapterFactory !== null) {
            this._objectAdapterFactory.destroy();
        }

        if (this._outgoingConnectionFactory !== null) {
            await this._outgoingConnectionFactory.waitUntilFinished();
        }

        if (this._retryQueue) {
            this._retryQueue.destroy();
        }

        if (this._timer) {
            this._timer.destroy();
        }

        if (this._routerManager) {
            this._routerManager.destroy();
        }
        if (this._locatorManager) {
            this._locatorManager.destroy();
        }

        if (this._initData.properties.getIcePropertyAsInt("Ice.Warn.UnusedProperties") > 0) {
            const unusedProperties = this._initData.properties.getUnusedProperties();
            if (unusedProperties.length > 0) {
                const message = [];
                message.push("The following properties were set but never read:");
                unusedProperties.forEach(p => message.push("\n    ", p));
                this._initData.logger.warning(message.join(""));
            }
        }

        this._objectAdapterFactory = null;
        this._outgoingConnectionFactory = null;
        this._retryQueue = null;
        this._timer = null;

        this._referenceFactory = null;
        this._routerManager = null;
        this._locatorManager = null;
        this._endpointFactoryManager = null;

        this._state = StateDestroyed;
        this._destroyPromise.resolve();
    } catch (ex) {
        this._state = StateDestroyed;
        this._destroyPromise.reject(ex);
        throw ex;
    }
};

Object.defineProperty(Instance.prototype, "clientConnectionOptions", {
    get: function () {
        return this._clientConnectionOptions;
    },
    enumerable: true,
});
