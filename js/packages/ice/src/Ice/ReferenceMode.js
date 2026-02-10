// Copyright (c) ZeroC, Inc.

// Using a separate module for these constants so that ObjectPrx does not need to include Reference.
export const ReferenceMode = {
    ModeTwoway: 0,
    ModeOneway: 1,
    ModeBatchOneway: 2,
    ModeDatagram: 3,
    ModeBatchDatagram: 4,
    ModeLast: 4,
};
