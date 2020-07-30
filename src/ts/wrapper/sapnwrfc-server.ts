// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

import { isUndefined } from "util";
import {
    //Promise,
    noderfc_binding,
    environment,
    NodeRfcEnvironment,
} from "./noderfc-bindings";

import { RfcConnectionParameters } from "./sapnwrfc-client";
//
// RfcServer
//

export interface RfcServerBinding {
    new (
        serverParams: RfcConnectionParameters,
        clientParams: RfcConnectionParameters
    ): RfcServerBinding;
    (
        serverParams: RfcConnectionParameters,
        clientParams: RfcConnectionParameters
    ): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _server_conn_handle: number;
    _client_conn_handle: number;
    register(callback: Function): void;
    addFunction(
        functionName: string,
        jsFunction: Function,
        callback: Function
    ): void;
    removeFunction(functionName: string, callback: Function): void;
    serve(callback: Function): void;
    getFunctionDescription(rfmName: string, callback: Function): void;
}

export class Server {
    private __server: RfcServerBinding;

    constructor(
        serverParams: RfcConnectionParameters,
        clientParams: RfcConnectionParameters
    ) {
        this.__server = new noderfc_binding.Server(serverParams, clientParams);
    }

    register(callback?: Function) {
        if (isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.register((err: any) => {
                    if (isUndefined(err)) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        this.__server.register(callback);
    }

    addFunction(
        functionName: string,
        jsFunction: Function,
        callback?: Function
    ) {
        if (isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.addFunction(
                    functionName,
                    jsFunction,
                    (err: any) => {
                        if (isUndefined(err)) {
                            resolve();
                        } else {
                            reject(err);
                        }
                    }
                );
            });
        }

        this.__server.addFunction(functionName, jsFunction, callback);
    }

    removeFunction(functionName: string, callback?: Function) {
        if (isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.removeFunction(functionName, (err: any) => {
                    if (isUndefined(err)) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        this.__server.removeFunction(functionName, callback);
    }

    serve(callback?: Function) {
        if (isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.serve((err: any) => {
                    if (isUndefined(err)) {
                        resolve();
                    } else {
                        reject(err);
                    }
                });
            });
        }

        this.__server.serve(callback);
    }

    getFunctionDescription(rfmName: string, callback?: Function) {
        if (isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.getFunctionDescription(
                    rfmName,
                    (err: any, rfmFunctionDescription: object) => {
                        if (isUndefined(err)) {
                            resolve(rfmFunctionDescription);
                        } else {
                            reject(err);
                        }
                    }
                );
            });
        }

        this.__server.serve(callback);
    }

    static get environment(): NodeRfcEnvironment {
        return environment;
    }

    get environment(): NodeRfcEnvironment {
        return environment;
    }

    get binding(): RfcServerBinding {
        return this.__server;
    }

    get id(): number {
        return this.__server._id;
    }

    get alive(): boolean {
        return this.__server._alive;
    }

    get server_connection(): number {
        return this.__server._server_conn_handle;
    }

    get client_connection(): number {
        return this.__server._client_conn_handle;
    }
}
