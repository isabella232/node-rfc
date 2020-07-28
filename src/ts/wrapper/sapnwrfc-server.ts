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
    new (connectionParameters: RfcConnectionParameters): RfcServerBinding;
    (connectionParameters: RfcConnectionParameters): RfcServerBinding;
    _id: number;
    _alive: boolean;
    _connectionHandle: number;
    register(callback: Function): void | Promise<void>;
}

export class Server {
    private __server: RfcServerBinding;

    constructor(connectionParameters: RfcConnectionParameters) {
        this.__server = new noderfc_binding.Server(connectionParameters);
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

    get connectionHandle(): number {
        return this.__server._connectionHandle;
    }
}
