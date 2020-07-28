import { NodeRfcEnvironment } from "./noderfc-bindings";
import { RfcConnectionParameters } from "./sapnwrfc-client";
export interface RfcServerBinding {
    new (connectionParameters: RfcConnectionParameters): RfcServerBinding;
    (connectionParameters: RfcConnectionParameters): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _connectionHandle: number;
    register(callback: Function): void | Promise<void>;
}
export declare class Server {
    private __server;
    constructor(connectionParameters: RfcConnectionParameters);
    register(callback?: Function): Promise<unknown> | undefined;
    static get environment(): NodeRfcEnvironment;
    get environment(): NodeRfcEnvironment;
    get binding(): RfcServerBinding;
    get id(): number;
    get alive(): boolean;
    get connectionHandle(): number;
}
