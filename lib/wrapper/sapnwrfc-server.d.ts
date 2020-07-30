import { NodeRfcEnvironment } from "./noderfc-bindings";
import { RfcConnectionParameters } from "./sapnwrfc-client";
export interface RfcServerBinding {
    new (serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters): RfcServerBinding;
    (serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _server_conn_handle: number;
    _client_conn_handle: number;
    register(callback: Function): void;
    addFunction(functionName: string, jsFunction: Function, callback: Function): void;
    removeFunction(functionName: string, callback: Function): void;
    serve(callback: Function): void;
    getFunctionDescription(rfmName: string, callback: Function): void;
}
export declare class Server {
    private __server;
    constructor(serverParams: RfcConnectionParameters, clientParams: RfcConnectionParameters);
    register(callback?: Function): Promise<unknown> | undefined;
    addFunction(functionName: string, jsFunction: Function, callback?: Function): Promise<unknown> | undefined;
    removeFunction(functionName: string, callback?: Function): Promise<unknown> | undefined;
    serve(callback?: Function): Promise<unknown> | undefined;
    getFunctionDescription(rfmName: string, callback?: Function): Promise<unknown> | undefined;
    static get environment(): NodeRfcEnvironment;
    get environment(): NodeRfcEnvironment;
    get binding(): RfcServerBinding;
    get id(): number;
    get alive(): boolean;
    get server_connection(): number;
    get client_connection(): number;
}
