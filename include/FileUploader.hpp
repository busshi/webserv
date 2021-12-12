#pragma once
#include "Buffer.hpp"
#include "http/FormDataParser.hpp"
#include "http/message.hpp"
#include <fstream>

class FileUploader
{
  private:
    FileUploader(const FileUploader& other);
    FileUploader& operator=(const FileUploader& rhs);

    HTTP::Request* _req;
    HTTP::FormDataParser* _parser;
    bool _isUploading;
    std::string _uploadPath, _currentUploadFilePath;
    unsigned long long _maxUploadFileSize, _currentUploadFileSize;
    size_t _uploadCount;

  public:
    std::ofstream ofs;

    FileUploader(HTTP::Request* req,
                 const std::string& uploadPath = "/tmp",
                 size_t maxUploadFileSize = 0);
    ~FileUploader(void);

    void parseFormDataFragment(const char* data, size_t n);
    bool isDone(void) const;

    bool isUploading(void) const;

    void startUpload(const char* filepath);
    bool uploadData(const Buffer<>& data);
    void stopUpload(void);
};
