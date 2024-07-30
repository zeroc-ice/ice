//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        class OutgoingResponse {
            constructor(
                outputStream: OutputStream,
                replyStatus: ReplyStatus,
                exceptionId: string,
                exceptionDetails: string,
            );

            replyStatus: ReplyStatus;
            outputStream: OutputStream;
            exceptionId: string;
            exceptionDetails: string;
            size: number;
        }
    }
}
