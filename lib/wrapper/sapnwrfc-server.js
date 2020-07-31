"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Server = void 0;
const util_1 = require("util");
const noderfc_bindings_1 = require("./noderfc-bindings");
class Server {
    constructor(serverParams, clientParams) {
        this.__server = new noderfc_bindings_1.noderfc_binding.Server(serverParams, clientParams);
    }
    start(callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.start((err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.start(callback);
    }
    stop(callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.stop((err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.stop(callback);
    }
    addFunction(functionName, jsFunction, callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.addFunction(functionName, jsFunction, (err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.addFunction(functionName, jsFunction, callback);
    }
    removeFunction(functionName, callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.removeFunction(functionName, (err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.removeFunction(functionName, callback);
    }
    getFunctionDescription(rfmName, callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.getFunctionDescription(rfmName, (err, rfmFunctionDescription) => {
                    if (util_1.isUndefined(err)) {
                        resolve(rfmFunctionDescription);
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.getFunctionDescription(rfmName, callback);
    }
    static get environment() {
        return noderfc_bindings_1.environment;
    }
    get environment() {
        return noderfc_bindings_1.environment;
    }
    get binding() {
        return this.__server;
    }
    get id() {
        return this.__server._id;
    }
    get alive() {
        return this.__server._alive;
    }
    get server_connection() {
        return this.__server._server_conn_handle;
    }
    get client_connection() {
        return this.__server._client_conn_handle;
    }
}
exports.Server = Server;
//# sourceMappingURL=sapnwrfc-server.js.map