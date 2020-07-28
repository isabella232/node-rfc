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

#include "Client.h"
#include "Pool.h"
#include "Throughput.h"
#include "Server.h"

namespace node_rfc
{
    Napi::Env __env = NULL;

    Napi::Value BindingVersions(Napi::Env env)
    {
        uint_t major, minor, patchLevel;
        Napi::EscapableHandleScope scope(env);

        RfcGetVersion(&major, &minor, &patchLevel);

        Napi::Object nwrfcsdk = Napi::Object::New(env);
        nwrfcsdk.Set("major", major);
        nwrfcsdk.Set("minor", minor);
        nwrfcsdk.Set("patchLevel", patchLevel);

        Napi::Object version = Napi::Object::New(env);
        version.Set("version", NODERFC_VERSION);
        version.Set("nwrfcsdk", nwrfcsdk);

        return scope.Escape(version);
    }

    Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
        exports.Set("bindingVersions", BindingVersions(env));

        Pool::Init(env, exports);
        Client::Init(env, exports);
        Throughput::Init(env, exports);
        Server::Init(env, exports);

        return exports;
    }

    NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule);
} // namespace node_rfc
