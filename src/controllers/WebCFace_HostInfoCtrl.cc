#include "WebCFace_HostInfoCtrl.h"
#include "../lib/host.hpp"
using namespace WebCFace;

void HostInfoCtrl::asyncHandleHttpRequest(
    const HttpRequestPtr& /*req*/, std::function<void(const HttpResponsePtr&)>&& callback)
{
    // write your application logic here
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    resp->setBody("{\"name\":\"" + host.name + "\",\"addr\":\"" + host.addr + "\"}");
    callback(resp);
}
