
export class DOMElement
{
    val(value:string):string;
    scrollTop():void;
}

export class Output
{
    constructor(output:DOMElement);
    write(msg:string):void;
    writeLine(msg:string):void;
    get():string;
    clear():void;
}

export default class ControllerHelper
{
    constructor(exe:string, output:Output);

    serverReady(ex?:Error):void;
    waitReady():PromiseLike<void>;

    write(msg:string):void;
    writeLine(msg:string):void;
}
