//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

function generateUUID()
{
    let d = new Date().getTime();
    const uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, c => {
        const r = (d + Math.random() * 16) % 16 | 0;
        d = Math.floor(d / 16);
        return (c == 'x' ? r : (r & 0x3 | 0x8)).toString(16);
    });
    return uuid;
}

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.generateUUID = generateUUID;
module.exports.Ice = Ice;
