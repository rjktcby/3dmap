#ifndef FETCHER_H_INCLUDED
#define FETCHER_H_INCLUDED

#include <curl/curl.h>
#include <string>
#include <list>
#include "buffer.hpp"
#include "tileId.hpp"

struct HandleInfo {
        tileId id;
        CURL *http_handle;
        buffer *buf;
};

//For std::find
struct has_handle {
    has_handle(CURL *h) : h(h) { }
    bool operator()(HandleInfo* hInfo) const { return hInfo->http_handle == h; }
private:
    CURL *h;
};

//For std::find
struct has_id {
    has_id(tileId id) : id(id) { }
    bool operator()(HandleInfo* hInfo) const { return hInfo->id == id; }
private:
    tileId id;
};

class Dispatcher;

class Fetcher
{
        Dispatcher *dispatcher;

        CURLM *multi_handle;
        std::list<HandleInfo *> httpHandles;
        int still_running;

        void requestFinished(CURL *http_handle);

public:
        Fetcher(Dispatcher *d);
        ~Fetcher();

        void addRequest(tileId id);
        void perform();
        void setDispatcher(Dispatcher *d);
};

#endif //FETCHER_H_INCLUDED