#pragma once
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

  public:
    std::ofstream ofs;

    FileUploader(HTTP::Request* req);
    ~FileUploader(void);

    void parseFormDataFragment(const char* data, size_t n);
    bool isDone(void) const;
};
