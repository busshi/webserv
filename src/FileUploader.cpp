#include "FileUploader.hpp"
#include <cstring>

#define GET_UPLOADER(loc) reinterpret_cast<FileUploader*>(loc)

static void
onFinalBoundary(uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    if (uploader->isUploading()) {
        uploader->stopUpload();
    }

    std::cout << "Final boundary" << std::endl;
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

            std::cout << "filename " << filename << std::endl;

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
        uploader->stopUpload();
    }

    std::cout << "Uploaded" << std::endl;
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
        std::cerr
          << "Failed to retrieve boundary, can't construct FileUploader object!"
          << std::endl;
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
    if (isUploading()) {
        stopUpload();
    }

    _isUploading = true;
    _currentUploadFilePath = _uploadPath + "/" + filename;

    std::cout << "Will upload " << _currentUploadFilePath << std::endl;

    ofs.open(_currentUploadFilePath.c_str(), std::ios::out);
}

bool
FileUploader::uploadData(const Buffer<>& data)
{
    if (isUploading()) {
        if (_maxUploadFileSize > 0 &&
            _currentUploadFileSize + data.size() >= _maxUploadFileSize) {
            std::cerr << "Upload cancelled: max upload size reached"
                      << std::endl;
            stopUpload();
            return false;
        }
        ofs << data;
        _currentUploadFileSize += data.size();
    }

    return true;
}

void
FileUploader::stopUpload(void)
{
    _isUploading = false;
    _currentUploadFileSize = 0;
    _currentUploadFilePath = "";
    if (ofs.is_open()) {
        ofs.close();
    }
}
