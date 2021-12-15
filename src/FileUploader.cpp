#include "FileUploader.hpp"
#include "http/Exception.hpp"
#include "http/status.hpp"
#include "utils/string.hpp"
#include <cstring>

#define GET_UPLOADER(loc) reinterpret_cast<FileUploader*>(loc)

static void
onFinalBoundary(uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    HTTP::Response* res = uploader->request()->response();

    res->setStatus(HTTP::CREATED);

    res->append("");
    res->data = res->formatHeader();
    res->data += res->body;
}

static void
onEntryHeaderField(const std::string& name,
                   const std::string& value,
                   uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    if (equalsIgnoreCase(name, "Content-Disposition")) {
        std::string::size_type pos = value.find("filename=\"");

        if (pos != std::string::npos) {
            std::string tmp = value.substr(pos + 10);
            std::string filename = tmp.substr(0, tmp.find('"'));

            if (filename.empty()) {
                return;
            }

            uploader->startUpload(filename.c_str());
        }
    }
}

static void
onEntryBodyFragment(const Buffer<>& fragment, uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    uploader->uploadData(fragment);
}

static void
onEntryBodyParsed(uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    if (uploader->isUploading()) {
        uploader->finishUpload();
    }
}

FileUploader::FileUploader(HTTP::Request* req,
                           const std::string& uploadPath,
                           size_t maxUploadFileSize)
  : _req(req)
  , _uploadPath(uploadPath)
  , _maxUploadFileSize(maxUploadFileSize)
  , _currentUploadFileSize(0)
  , _uploadCount(0)
{
    const std::string s = _req->getHeaderField("Content-Type");
    std::string::size_type pos = s.find("boundary=");

    if (pos == std::string::npos) {
        throw HTTP::Exception(
          req,
          HTTP::BAD_REQUEST,
          "File uploader: missing boundary in Content-Type");
        return;
    }

    std::string boundary = s.substr(pos + 9);

    HTTP::FormDataParser::CallbackList conf;

    memset(&conf, 0, sizeof(conf));

    conf.onEntryBodyFragment = onEntryBodyFragment;
    conf.onFinalBoundary = onFinalBoundary;
    conf.onEntryHeaderField = onEntryHeaderField;
    conf.onEntryBodyParsed = onEntryBodyParsed;

    _parser = new HTTP::FormDataParser(boundary, conf);
}

FileUploader::~FileUploader(void)
{
    delete _parser;
}

void
FileUploader::parseFormDataFragment(const char* data, size_t n)
{
    _parser->parse(data, n, reinterpret_cast<uintptr_t>(this));
}

bool
FileUploader::isDone(void) const
{
    return _parser->getState() == HTTP::FormDataParser::DONE;
}

bool
FileUploader::isUploading(void) const
{
    return _isUploading;
}

void
FileUploader::startUpload(const char* filename)
{
    _isUploading = true;
    _currentUploadFilePath = _uploadPath + "/" + filename;
    _origFilename = filename;

    ofs.open(_currentUploadFilePath.c_str(), std::ios::out);

    if (!ofs) {
        std::cerr << "Open error" << std::endl;
    }
}

bool
FileUploader::uploadData(const Buffer<>& data)
{
    if (isUploading()) {
        if (_maxUploadFileSize > 0 &&
            _currentUploadFileSize + data.size() >= _maxUploadFileSize) {
            throw HTTP::Exception(_req,
                                  HTTP::REQUEST_PAYLOAD_TOO_LARGE,
                                  "Uploaded file is too large");
            _isUploading = false;
            ofs.close();
            return false;
        }
        ofs << data;
        if (!ofs) {
            std::cout << "Stream error!" << std::endl;
        }
        _currentUploadFileSize += data.size();
    }

    return true;
}

void
FileUploader::finishUpload(void)
{
    _req->response()->append(_origFilename + " - OK\n");

    _isUploading = false;
    _currentUploadFileSize = 0;
    _currentUploadFilePath = "";
    if (ofs.is_open()) {
        ofs.close();
    }
}

HTTP::Request*
FileUploader::request()
{
    return _req;
}
