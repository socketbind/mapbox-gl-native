#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <node.h>
#include <nan.h>
#pragma GCC diagnostic pop

#include "node_mapbox_gl_native.hpp"
#include "node_map.hpp"
#include "node_log.hpp"
#include "node_request.hpp"

namespace node_mbgl {

mbgl::util::RunLoop& NodeRunLoop() {
    static mbgl::util::RunLoop nodeRunLoop;
    return nodeRunLoop;
}

}

NAN_MODULE_INIT(RegisterModule) {
    // This has the effect of:
    //   a) Ensuring that the static local variable is initialized before any thread contention.
    //   b) unreffing an async handle, which otherwise would keep the default loop running.
    node_mbgl::NodeRunLoop().stop();

    node_mbgl::NodeMap::Init(target);
    node_mbgl::NodeRequest::Init(target);

    // Make the exported object inherit from process.EventEmitter
    v8::Local<v8::Object> process = Nan::Get(
        Nan::GetCurrentContext()->Global(),
        Nan::New("process").ToLocalChecked()).ToLocalChecked()->ToObject();

    v8::Local<v8::Object> EventEmitter = Nan::Get(process,
        Nan::New("EventEmitter").ToLocalChecked()).ToLocalChecked()->ToObject();

    Nan::SetPrototype(target,
        Nan::Get(EventEmitter, Nan::New("prototype").ToLocalChecked()).ToLocalChecked());

    mbgl::Log::setObserver(std::make_unique<node_mbgl::NodeLogObserver>(target->ToObject()));
}

NODE_MODULE(mapbox_gl_native, RegisterModule)
