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

#ifndef NodeRfc_Server_H
#define NodeRfc_Server_H

#include <uv.h>
#include "Client.h"

namespace node_rfc
{
    extern Napi::Env __env;

    class Server : public Napi::ObjectWrap<Server>
    {
    public:
        friend class RegisterAsync;
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        Server(const Napi::CallbackInfo &info);
        ~Server(void);

    private:
        Napi::Value IdGetter(const Napi::CallbackInfo &info);
        Napi::Value AliveGetter(const Napi::CallbackInfo &info);
        Napi::Value ConnectionHandleGetter(const Napi::CallbackInfo &info);

        Napi::Value Register(const Napi::CallbackInfo &info);

        Napi::ObjectReference clientParamsRef;
        RFC_CONNECTION_HANDLE connectionHandle;
        ConnectionParamsStruct client_params;

        void init(Napi::Env env)
        {
            if (node_rfc::__env == NULL)
            {
                node_rfc::__env = env;
            };
            id = Server::_id++;

            connectionHandle = NULL;

            uv_sem_init(&invocationMutex, 1);
        };

        static uint_t _id;
        uint_t id;

        void LockMutex();
        void UnlockMutex();
        uv_sem_t invocationMutex;
    };

} // namespace node_rfc

#endif
