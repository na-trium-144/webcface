#include <algorithm>
#include <vector>
#include <webcface/c_wcf.h>
#include <webcface/client.h>

using namespace webcface;

static std::vector<wcfClient> wcli_list;
static Client *getWcli(wcfClient wcli){
    if(std::find(wcli_list.begin(), wcli_list.end(), wcli) == wcli_list.end()){
        return nullptr;
    }
    return static_cast<Client *>(wcli);
}

extern "C"{
wcfClient wcfInit(const char *name, const char *host, int port){
    auto wcli = new Client(name, host, port);
    wcli_list.push_back(wcli);
    return wcli;
}
int wcfIsInstance(wcfClient wcli){
    if(getWcli(wcli)){
        return 1;
    }else{
        return 0;
    }
}
int wcfClose(wcfClient wcli){
    auto wcli_ = getWcli(wcli);
    if(!wcli_){
        return 1;
    }
    wcli_list.erase(std::find(wcli_list.begin(), wcli_list.end(), wcli));
    delete wcli_;
    return 0;
}

int wcfStart(wcfClient wcli){
    auto wcli_ = getWcli(wcli);
    if(!wcli_){
        return 1;
    }
    wcli_->start();
    return 0;
}
int wcfSync(wcfClient wcli){
    auto wcli_ = getWcli(wcli);
    if(!wcli_){
        return 1;
    }
    wcli_->sync();
    return 0;
}
int wcfValueSet(wcfClient wcli, const char *field, double value){
    auto wcli_ = getWcli(wcli);
    if(!wcli_){
        return 1;
    }
    wcli_->value(field).set(value);
    return 0;
}
int wcfValueSetVecD(wcfClient wcli, const char *field, double* value, int size){
    auto wcli_ = getWcli(wcli);
    if(!wcli_){
        return 1;
    }
    wcli_->value(field).set(std::vector<double>(value, value + size));
    return 0;
}


}