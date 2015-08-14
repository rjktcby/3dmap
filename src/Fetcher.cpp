#include <iostream>
#include <sstream>
#include <algorithm>

#include <curl/multi.h>

#include "Fetcher.h"
#include "Dispatcher.h"

#define DEFAULT_ALLOC_SIZE 0

// #define _FETCHER_DEBUG 1

static size_t write_cb(char *data, size_t n, size_t l, HandleInfo *hInfo)
{

#ifdef _FETCHER_DEBUG
        std::cout << "[Fetcher] Got " << n*l << " bytes for tile=" << hInfo->id << std::endl;
#endif

        hInfo->buf->append(data, n*l);

        return n*l;
}

Fetcher::Fetcher(Dispatcher *d)
{
        curl_global_init(CURL_GLOBAL_DEFAULT);
        /* init a multi stack */
        multi_handle = curl_multi_init();
        still_running = 0;
        dispatcher = d;
}

Fetcher::~Fetcher()
{
        std::list<HandleInfo *>::iterator h;
        for (h = httpHandles.begin(); h != httpHandles.end(); ++h) {
                curl_multi_remove_handle(multi_handle, (*h)->http_handle);
                curl_easy_cleanup((*h)->http_handle);
                delete (*h)->buf;
                delete *h;
        }
        httpHandles.clear();

        curl_multi_cleanup(multi_handle);
        curl_global_cleanup();
}


void Fetcher::addRequest(tileId id)
{
        std::list<HandleInfo *>::iterator pos = std::find_if(httpHandles.begin(),
                                                      httpHandles.end(),
                                                      has_id(id));
        if (pos != httpHandles.end()) { // Tile request is already processed
                return;
        }

        std::stringstream stringStream;
        stringStream << "http://api.tiles.mapbox.com/v4/mapbox.pencil/"<<id.z()<<"/"<<id.x()<<"/"<<id.y()<<".png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6IlhHVkZmaW8ifQ.hAMX5hSW-QnTeRCMAy9A8Q";
        std::string url = stringStream.str();

#ifdef _FETCHER_DEBUG
        std::cout << "[Fetcher] tile=" << id << " url=" << url << std::endl;
#endif

        HandleInfo *hInfo = new HandleInfo();
        hInfo->id = id;
        hInfo->http_handle = curl_easy_init();
        hInfo->buf = new buffer(DEFAULT_ALLOC_SIZE);

        curl_easy_setopt(hInfo->http_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(hInfo->http_handle, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(hInfo->http_handle, CURLOPT_WRITEDATA, hInfo);

        curl_multi_add_handle(multi_handle, hInfo->http_handle);
        httpHandles.push_back(hInfo);

        if (!still_running) {
                curl_multi_perform(multi_handle, &still_running);
        }
}

void Fetcher::requestFinished(CURL *http_handle)
{
        std::list<HandleInfo *>::iterator pos = std::find_if(httpHandles.begin(),
                                                      httpHandles.end(),
                                                      has_handle(http_handle));
        HandleInfo *hInfo = *pos;

        if (hInfo == NULL) {
                std::cout << "[Fetcher] error: unable to find handle=" << http_handle << std::endl;
                return;
        }

        // Get HTTP status code
        int http_status_code = 0;
        const char *szUrl = NULL;
        curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
        curl_easy_getinfo(http_handle, CURLINFO_PRIVATE, &szUrl);

        if (http_status_code != 200) {
                std::cout << "[Fetcher] error: GET of url=" << szUrl << " returned http status code=" << http_status_code << std::endl;
        }

        httpHandles.remove(hInfo);

        curl_multi_remove_handle(multi_handle, http_handle);
        curl_easy_cleanup(http_handle);

        if (!dispatcher) {
                std::cout << "[Fetcher] error: dispatcher not set" << std::endl;
        } else {
                dispatcher->tileRequestFinished(hInfo->id, hInfo->buf);
        }

        delete hInfo->buf;
        delete hInfo;
}

void Fetcher::perform()
{
        CURLMsg *msg = NULL;
        CURL *http_handle;
        int msgs_left = 0;
        int rc = 0;
        int numfds;

        if (still_running) {
                rc = curl_multi_wait(multi_handle, NULL, 0, 10, &numfds);

                if (rc != CURLM_OK) {
                    std::cout << "[Fetcher] error: curl_multi_wait() failed, code=" << rc << std::endl;
                    still_running = 0;
                }
        }

        while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
                if (msg->msg == CURLMSG_DONE) {
                        http_handle = msg->easy_handle;
                        rc = msg->data.result;
                        if (rc != CURLE_OK) {
                                std::cout << "[Fetcher] error: CURL error=" << msg->data.result << std::endl;
                                continue;
                        }
                        requestFinished(http_handle);
                } else {
                        std::cout << "[Fetcher] error: after curl_multi_info_read(), CURLMsg=" << msg->msg << std::endl;
                }
        }

        if (still_running) {
                curl_multi_perform(multi_handle, &still_running);
        }

}