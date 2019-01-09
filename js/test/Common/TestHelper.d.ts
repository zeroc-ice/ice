import { Ice } from "ice";
import ControllerHelper from "./ControllerI"

export class TestHelper
{
    getTestEndpoint(protocol:string):string;
    getTestEndpoint(num?:number, protocol?:string):string;
    getTestEndpoint(properties:Ice.Properties, num?:number, protocol?:string):string;

    getTestHost(properties:Ice.Properties):string;

    getTestProtocol(properties:Ice.Properties):string;
    getTestPort(num?:number):number;

    createTestProperties(args?:string[]):[Ice.Properties, string[]];

    initialize(initData:Ice.InitializationData):[Ice.Communicator, string[]];
    initialize(args:Ice.Properties):[Ice.Communicator, string[]];
    initialize(args:string[]):[Ice.Communicator, string[]];

    communicator():Ice.Communicator;
    shutdown():void;

    getWriter():ControllerHelper;

    setControllerHelper(controllerHelper:ControllerHelper):void;
    serverReady():void;
    static test(value:boolean, ex?:Error):void;
}
