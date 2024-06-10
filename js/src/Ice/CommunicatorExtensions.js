
import { Communicator } from './Communicator.js';
import { Instance } from './Instance.js';
import { AsyncResultBase } from './AsyncResultBase.js';

Communicator.prototype.createInstance = function(initData)
{
    return new Instance(initData);
};

Communicator.prototype.createAsyncResultBase = function(communicator, op, connection, proxy, adapter)
{
    return new AsyncResultBase(communicator, op, connection, proxy, adapter)
};
