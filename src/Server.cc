
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

#include "Server.h"

namespace node_rfc
{
    extern Napi::Env __env;

    uint_t Server::_id = 1;

    Napi::Object Server::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(
            env, "Server", {
                               InstanceAccessor("_id", &Server::IdGetter, nullptr),
                               InstanceAccessor("_alive", &Server::AliveGetter, nullptr),
                               InstanceAccessor("_connectionHandle", &Server::ConnectionHandleGetter, nullptr),
                               InstanceMethod("register", &Server::Register),
                           });

        Napi::FunctionReference *constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        constructor->SuppressDestruct();

        exports.Set("Server", func);
        return exports;
    }

    Napi::Value Server::IdGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(Env(), id);
    }

    Napi::Value Server::AliveGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(Env(), connectionHandle != NULL);
    }

    Napi::Value Server::ConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->connectionHandle);
    }

    class RegisterAsync : public Napi::AsyncWorker
    {
    public:
        RegisterAsync(Napi::Function &callback, Server *server)
            : Napi::AsyncWorker(callback), server(server) {}
        ~RegisterAsync() {}

        void Execute()
        {
            server->LockMutex();
            server->connectionHandle = RfcRegisterServer(server->client_params.connectionParams, server->client_params.paramSize, &errorInfo);
            server->UnlockMutex();
        }

        void OnOK()
        {
            if (server->connectionHandle == NULL)
            {
                Callback().Call({rfcSdkError(&errorInfo)});
            }
            else
            {
                Callback().Call({});
            }
            Callback().Reset();
        }

    private:
        Server *server;
        RFC_ERROR_INFO errorInfo;
    };

    Server::Server(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Server>(info)
    {
        init(info.Env());

        DEBUG("Server::Server ", id);

        if (!info[0].IsUndefined() && (info[0].IsFunction() || !info[0].IsObject()))
        {
            Napi::TypeError::New(Env(), "Server constructor requires connection parameters").ThrowAsJavaScriptException();
            return;
        }

        if (info.Length() > 0)
        {
            clientParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
            getConnectionParams(clientParamsRef.Value(), &client_params);
        }
    };

    Napi::Value Server::Register(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::Register");

        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Server register() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        (new RegisterAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Server::~Server(void)
    {
        DEBUG("~ Server ", id);

        uv_sem_destroy(&invocationMutex);
    }

    void Server::LockMutex()
    {
        uv_sem_wait(&invocationMutex);
    }

    void Server::UnlockMutex()
    {
        uv_sem_post(&invocationMutex);
    }

} // namespace node_rfc
