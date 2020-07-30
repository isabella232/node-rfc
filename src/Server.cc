
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
#include <v8.h>

namespace node_rfc
{
    extern Napi::Env __env;

    uint_t Server::_id = 1;

    Server *__server = NULL;

    Napi::Object Server::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(
            env, "Server", {
                               InstanceAccessor("_id", &Server::IdGetter, nullptr),
                               InstanceAccessor("_alive", &Server::AliveGetter, nullptr),
                               InstanceAccessor("_server_conn_handle", &Server::ServerConnectionHandleGetter, nullptr),
                               InstanceAccessor("_client_conn_handle", &Server::ClientConnectionHandleGetter, nullptr),
                               InstanceMethod("register", &Server::Register),
                               InstanceMethod("addFunction", &Server::AddFunction),
                               InstanceMethod("serve", &Server::Serve),
                               InstanceMethod("getFunctionDescription", &Server::GetFunctionDescription),
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
        return Napi::Boolean::New(Env(), server_conn_handle != NULL);
    }

    Napi::Value Server::ServerConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->server_conn_handle);
    }
    Napi::Value Server::ClientConnectionHandleGetter(const Napi::CallbackInfo &info)
    {
        return Napi::Number::New(info.Env(), (double)(unsigned long long)this->client_conn_handle);
    }

    Server::Server(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Server>(info)
    {
        node_rfc::__server = this;

        init(info.Env());

        DEBUG("Server::Server ", id);

        if (!info[0].IsObject())
        {
            Napi::TypeError::New(Env(), "Server constructor requires server parameters").ThrowAsJavaScriptException();
            return;
        }

        serverParamsRef = Napi::Persistent(info[0].As<Napi::Object>());
        getConnectionParams(serverParamsRef.Value(), &server_params);

        if (!info[1].IsObject())
        {
            Napi::TypeError::New(Env(), "Server constructor requires client parameters").ThrowAsJavaScriptException();
            return;
        }

        clientParamsRef = Napi::Persistent(info[1].As<Napi::Object>());
        getConnectionParams(clientParamsRef.Value(), &client_params);
    };

    RFC_RC SAP_API metadataLookup(SAP_UC const *func_name, RFC_ATTRIBUTES rfc_attributes, RFC_FUNCTION_DESC_HANDLE *func_desc_handle)
    {
        DEBUG("metadataLookup looking for: ");
        printfU(func_name);

        RFC_RC rc = RFC_NOT_FOUND;

        Server *server = node_rfc::__server; // todo check if null

        ServerFunctionsMap::iterator it = server->serverFunctions.begin();
        while (it != server->serverFunctions.end())
        {
            if (strcmpU(func_name, it->second.func_name) == 0)
            {
                *func_desc_handle = it->second.func_desc_handle;
                rc = RFC_OK;
                DEBUG("\nmetadataLookup found: ", (pointer_t)*func_desc_handle);
                break;
            }
            it++;
        }

        return rc;
    }

    RFC_RC SAP_API genericHandler(RFC_CONNECTION_HANDLE conn_handle, RFC_FUNCTION_HANDLE func_handle, RFC_ERROR_INFO *errorInfo)
    {
        Server *server = node_rfc::__server;

        RFC_RC rc = RFC_NOT_FOUND;

        RFC_FUNCTION_DESC_HANDLE func_desc = RfcDescribeFunction(func_handle, errorInfo);
        if (errorInfo->code != RFC_OK)
        {
            return errorInfo->code;
        }
        RFC_ABAP_NAME func_name;
        RfcGetFunctionName(func_desc, func_name, errorInfo);
        if (errorInfo->code != RFC_OK)
        {
            return errorInfo->code;
        }

        DEBUG("genericHandler ", (pointer_t)func_handle);
        printfU(func_name);

        ServerFunctionsMap::iterator it = server->serverFunctions.begin();
        while (it != server->serverFunctions.end())
        {
            if (strcmpU(func_name, it->second.func_name) == 0)
            {
                DEBUG("genericHandler found: ", (pointer_t)it->second.func_desc_handle);
                break;
            }
            it++;
        }

        if (it == server->serverFunctions.end())
        {
            return rc;
        }

        RFC_FUNCTION_DESC_HANDLE func_desc_handle = it->second.func_desc_handle;
        uint_t paramCount;

        //
        // ABAP -> JS parameters
        //
        //RfmErrorPath errorPath;
        //ClientOptionsStruct client_options;
        //ValuePair jsContainer = wrapResult(func_desc_handle, func_handle, &errorPath, &client_options);

        //
        // Call
        //
        //it->second.callback.Call({jsContainer});

        //
        // JS -> ABAP
        //
        //uint_t paramCount;
        RfcGetParameterCount(func_desc_handle, &paramCount, errorInfo);
        if (errorInfo->code != RFC_OK)
        {
            return errorInfo->code;
        }

        //Napi::Value err = Undefined();
        //for (uint_t i = 0; i < paramCount; i++)
        //{
        //    Napi::String name = paramNames.Get(i).ToString();
        //    Napi::Value value = params.Get(name);
        //    err = client->fillFunctionParameter(functionDescHandle, functionHandle, name, value);
        //
        //    if (!err.IsUndefined())
        //    {
        //        break;
        //    }
        //}

        return RFC_OK;
    }

    class ServeAsync : public Napi::AsyncWorker
    {
    public:
        ServeAsync(Napi::Function &callback, Server *server)
            : Napi::AsyncWorker(callback), server(server) {}
        ~ServeAsync()
        {
        }

        void Execute()
        {
            server->LockMutex();
            DEBUG("ServeAsync locked");
            server->client_conn_handle = RfcOpenConnection(server->client_params.connectionParams, server->client_params.paramSize, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: client connection ok");

            server->server_conn_handle = RfcRegisterServer(server->server_params.connectionParams, server->server_params.paramSize, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: registered");

            RfcInstallGenericServerFunction(genericHandler, metadataLookup, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: installed");

            server->serverHandle = RfcCreateServer(server->server_params.connectionParams, 1, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: created");

            RfcLaunchServer(server->serverHandle, &errorInfo);
            if (errorInfo.code != RFC_OK)
            {
                return;
            }
            DEBUG("Server:: launched ", (pointer_t)server->serverHandle)

            server->UnlockMutex();
            DEBUG("ServeAsync unlocked");
        }

        void OnOK()
        {
            Napi::EscapableHandleScope scope(Env());
            if (errorInfo.code != RFC_OK)
            {
                //Callback().Call({Env().Undefined()});
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

    class GetFunctionDescAsync : public Napi::AsyncWorker
    {
    public:
        GetFunctionDescAsync(Napi::Function &callback, Server *server)
            : Napi::AsyncWorker(callback), server(server) {}
        ~GetFunctionDescAsync() {}

        void Execute()
        {
            server->LockMutex();
            server->server_conn_handle = RfcRegisterServer(server->server_params.connectionParams, server->server_params.paramSize, &errorInfo);
            server->UnlockMutex();
        }

        void OnOK()
        {
            if (server->server_conn_handle == NULL)
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

        //Napi::Function callback = info[0].As<Napi::Function>();

        //(new RegisterAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Napi::Value Server::RemoveFunction(const Napi::CallbackInfo &info)
    {
        Napi::EscapableHandleScope scope(info.Env());

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server removeFunction() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        std::string function_name = info[0].As<Napi::String>().Utf8Value();

        if (!info[1].IsFunction())
        {
            errmsg << "Server removeFunction() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[1].As<Napi::Function>();

        //ServerFunctionsMap::iterator it = serverFunctions.find(function_name);

        serverFunctions.erase(function_name);

        DEBUG("Removed function ", function_name);
        // todo destroy handle

        callback.Call({});
        return scope.Escape(info.Env().Undefined());
    };

    Napi::Value Server::AddFunction(const Napi::CallbackInfo &info)
    {
        Napi::EscapableHandleScope scope(info.Env());

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server addFunction() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::String functionName = info[0].As<Napi::String>();

        if (functionName.Utf8Value().length() == 0 || functionName.Utf8Value().length() > 30)
        {
            errmsg << "Server addFunction() accepts max. 30 characters long ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        DEBUG("Server::AddFunction ", functionName.Utf8Value());

        if (!info[1].IsFunction())
        {
            errmsg << "Server addFunction() requires a NodeJS handler function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function jsFunction = info[1].As<Napi::Function>();

        if (!info[2].IsFunction())
        {
            errmsg << "Server addFunction() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[2].As<Napi::Function>();

        // Install function
        RFC_ERROR_INFO errorInfo;

        SAP_UC *func_name = fillString(functionName);

        RFC_FUNCTION_DESC_HANDLE func_desc_handle = RfcGetFunctionDesc(client_conn_handle, func_name, &errorInfo);

        if (errorInfo.code != RFC_OK)
        {
            free(func_name);
            callback.Call({rfcSdkError(&errorInfo)});
            return scope.Escape(info.Env().Undefined());
        }

        ServerFunctionStruct sfs = ServerFunctionStruct(func_name, func_desc_handle, jsFunction);
        free(func_name);

        serverFunctions[functionName.Utf8Value()] = sfs;
        DEBUG("Server::AddFunction added ", functionName.Utf8Value(), ": ", (pointer_t)func_desc_handle);

        callback.Call({});
        return scope.Escape(info.Env().Undefined());
    };

    Napi::Value Server::Serve(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::Serve");

        std::ostringstream errmsg;

        if (!info[0].IsFunction())
        {
            errmsg << "Server register() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[0].As<Napi::Function>();

        (new ServeAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Napi::Value Server::GetFunctionDescription(const Napi::CallbackInfo &info)
    {
        DEBUG("Server::GetFunctionDescription");

        std::ostringstream errmsg;

        if (!info[0].IsString())
        {
            errmsg << "Server getFunctionDescription() requires ABAP RFM name; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        if (!info[0].IsFunction())
        {
            errmsg << "Server getFunctionDescription() requires a callback function; see" << USAGE_URL;
            Napi::TypeError::New(info.Env(), errmsg.str()).ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        Napi::Function callback = info[1].As<Napi::Function>();

        (new GetFunctionDescAsync(callback, this))->Queue();

        return info.Env().Undefined();
    };

    Server::~Server(void)
    {
        DEBUG("~ Server ", id);

        uv_sem_destroy(&invocationMutex);
        if (serverHandle != NULL)
        {
            RfcShutdownServer(serverHandle, 60, NULL);
            RfcDestroyServer(serverHandle, NULL);
        }

        if (client_conn_handle != NULL)
        {
            RfcCloseConnection(client_conn_handle, NULL);
        }
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
