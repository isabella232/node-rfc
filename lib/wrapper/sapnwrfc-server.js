"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Server = void 0;
const util_1 = require("util");
const noderfc_bindings_1 = require("./noderfc-bindings");
class Server {
    constructor(connectionParameters) {
        this.__server = new noderfc_bindings_1.noderfc_binding.Server(connectionParameters);
    }
    register(callback) {
        if (util_1.isUndefined(callback)) {
            return new Promise((resolve, reject) => {
                this.__server.register((err) => {
                    if (util_1.isUndefined(err)) {
                        resolve();
                    }
                    else {
                        reject(err);
                    }
                });
            });
        }
        this.__server.register(callback);
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
    get connectionHandle() {
        return this.__server._connectionHandle;
    }
}
exports.Server = Server;
//# sourceMappingURL=sapnwrfc-server.js.map