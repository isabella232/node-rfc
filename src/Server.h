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
#include <map>
#include "Client.h"
typedef struct _ServerFunctionStruct
{
    RFC_ABAP_NAME func_name;
    RFC_FUNCTION_DESC_HANDLE func_desc_handle = NULL;
    Napi::FunctionReference callback;

    _ServerFunctionStruct()
    {
        func_name[0] = 0;
    }

    _ServerFunctionStruct(RFC_ABAP_NAME name, RFC_FUNCTION_DESC_HANDLE desc_handle, Napi::Function cb)
    {
        strcpyU(func_name, name);
        func_desc_handle = desc_handle;
        callback = Napi::Persistent(cb);
    }

    _ServerFunctionStruct &operator=(_ServerFunctionStruct &src) // note: passed by copy
    {
        strcpyU(func_name, src.func_name);
        func_desc_handle = src.func_desc_handle;
        callback = Napi::Persistent(src.callback.Value());
        return *this;
    }

    ~_ServerFunctionStruct()
    {
        callback.Reset();
    }
} ServerFunctionStruct;

typedef std::map<std::string, ServerFunctionStruct> ServerFunctionsMap;
namespace node_rfc
{
    extern Napi::Env __env;

    class Server : public Napi::ObjectWrap<Server>
    {
    public:
        friend class StartAsync;
        friend class GetFunctionDescAsync;
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        Server(const Napi::CallbackInfo &info);
        ~Server(void);
        ServerFunctionsMap serverFunctions;

    private:
        Napi::Value IdGetter(const Napi::CallbackInfo &info);
        Napi::Value AliveGetter(const Napi::CallbackInfo &info);
        Napi::Value ServerConnectionHandleGetter(const Napi::CallbackInfo &info);
        Napi::Value ClientConnectionHandleGetter(const Napi::CallbackInfo &info);

        Napi::Value Start(const Napi::CallbackInfo &info);
        Napi::Value Stop(const Napi::CallbackInfo &info);
        Napi::Value AddFunction(const Napi::CallbackInfo &info);
        Napi::Value RemoveFunction(const Napi::CallbackInfo &info);
        Napi::Value GetFunctionDescription(const Napi::CallbackInfo &info);
        //RFC_RC SAP_API metadataLookup(SAP_UC *func_name, RFC_ATTRIBUTES rfc_attributes, RFC_FUNCTION_DESC_HANDLE *func_handle);
        //RFC_RC SAP_API genericHandler(RFC_CONNECTION_HANDLE conn_handle, RFC_FUNCTION_HANDLE func_handle, RFC_ERROR_INFO *errorInfo);

        RFC_CONNECTION_HANDLE server_conn_handle;
        RFC_CONNECTION_HANDLE client_conn_handle;
        RFC_SERVER_HANDLE serverHandle;
        ConnectionParamsStruct server_params;
        ConnectionParamsStruct client_params;
        ClientOptionsStruct client_options;
        Napi::ObjectReference serverParamsRef;
        Napi::ObjectReference clientParamsRef;
        Napi::ObjectReference clientOptionsRef;

        void init(Napi::Env env)
        {
            if (node_rfc::__env == NULL)
            {
                node_rfc::__env = env;
            };
            id = Server::_id++;

            server_conn_handle = NULL;
            client_conn_handle = NULL;
            serverHandle = NULL;

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
