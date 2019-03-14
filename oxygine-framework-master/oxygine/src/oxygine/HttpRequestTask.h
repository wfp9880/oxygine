#pragma once
#include "oxygine-include.h"
#include "AsyncTask.h"
#include "core/file.h"
#include <string>
#include <vector>

namespace oxygine
{
    DECLARE_SMART(HttpRequestTask, spHttpRequestTask);
    class HttpRequestTask: public AsyncTask
    {
    public:
        static spHttpRequestTask create();
        typedef HttpRequestTask* (*createHttpRequestCallback)();
        typedef std::function< bool(int) > responseCodeChecker;
        static void setCustomRequests(createHttpRequestCallback);
        static void init();
        static void release();


        /**dispatching AsyncTask events*/
        enum
        {
            ERROR = AsyncTask::ERROR,
            PROGRESS = AsyncTask::PROGRESS,
            COMPLETE = AsyncTask::COMPLETE,
        };

        class ProgressEvent : public Event
        {
        public:
            enum {EVENT = PROGRESS};
            ProgressEvent(size_t Delta, size_t Loaded, size_t Total, bool First) : Event(PROGRESS), delta(Delta), loaded(Loaded), total(Total), first(First) {};

            size_t delta;
            size_t loaded;
            size_t total;
            bool first;
        };

        HttpRequestTask();
        ~HttpRequestTask();

        const std::vector<unsigned char>&   getResponse() const;
        const std::vector<unsigned char>&   getPostData() const;
        const std::string&                  getFileName() const;
        const std::string&                  getUrl() const;
        size_t                              getReceivedSize() const;
        size_t                              getExpectedSize() const;
        spObject                            getObject() const;

        /**swap version of getResponse if you want to modify result buffer inplace*/
        void getResponseSwap(std::vector<unsigned char>&);
        int  getResponseCode() const { return _responseCode; }
        const responseCodeChecker& getResponseCodeChecker() const {return _responseCodeChecker;}
        void addHeader(const std::string& key, const std::string& value);

        void setPostData(const std::vector<unsigned char>& data);
        void setUrl(const std::string& url);
        void setFileName(const std::string& name, bool continueDownload = false);
        void setCacheEnabled(bool enabled);
        void setObject(spObject obj);

        void setResponseCodeChecker(const responseCodeChecker& f) {_responseCodeChecker = f;}
        /**by default only response code == 200 is succeded, other codes are dispatching Event::ERROR*/
        void setSuccessOnAnyResponseCode(bool en);
        void setExpectedSize(size_t size) { _expectedContentSize = size; }

    protected:
        bool _prerun() override;
        void _onError() override;
        void _onComplete() override;
        void _dispatchComplete() override;
        void _finalize(bool error) override;

        std::string _getRunInfo() const override { return _url; }

        void gotHeaders();
        void write(const void* data, size_t size);

        //async
        void asyncProgress(size_t delta, size_t loaded, size_t total);

        void dispatchProgress(size_t delta, size_t loaded, size_t total);

        virtual void _setFileName(const std::string& name) {}
        virtual void _setUrl(const std::string& url) {}
        virtual void _setPostData(const std::vector<unsigned char>& data) {}
        virtual void _setCacheEnabled(bool enabled) {}
        virtual void _addHeader(const std::string& key, const std::string& value) {}

        std::string _url;
        std::string _fname;
        bool _writeFileError;
        bool _cacheEnabled;
        bool _firstTimeProgressDispatched;
        bool _progressOnWrite;

        bool _progressDispatched;
        unsigned int _progressDeltaDelayed;

        std::vector<unsigned char> _response;
        std::vector<unsigned char> _postData;

        bool _continueDownload;

        size_t _expectedContentSize;
        size_t _receivedContentSize;
        file::handle _fhandle;
        bool _suitableResponse;
        responseCodeChecker _responseCodeChecker;

        int _responseCode;

        spObject _object;

        typedef std::vector< std::pair<std::string, std::string> >  headers;
        headers _headers;
    };
}
