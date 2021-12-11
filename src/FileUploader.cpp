#include "FileUploader.hpp"
#include <cstring>

#define GET_UPLOADER(loc) reinterpret_cast<FileUploader*>(loc)

static void
onFinalBoundary(uintptr_t)
{
    std::cout << "On final boundary!" << std::endl;
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

            if (uploader->ofs.is_open()) {
                uploader->ofs.close();
            }

            uploader->ofs.open((std::string("/tmp/") + filename).c_str(),
                               std::ios::out | std::ios::binary);
        }
    }
}

static void
onEntryBodyFragment(const std::string& fragment, uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    uploader->ofs.write(fragment.data(), fragment.size());
}

static void
onEntryBodyParsed(uintptr_t param)
{
    FileUploader* uploader = GET_UPLOADER(param);

    uploader->ofs.close();
}

FileUploader::FileUploader(HTTP::Request* request)
{
    const std::string s = request->getHeaderField("Content-Type");
    std::string::size_type pos = s.find_last_of("boundary=");

    if (pos == std::string::npos) {
        std::cerr
          << "Failed to retrieve boundary, can't construct FileUploader object!"
          << std::endl;
        return;
    }

    std::string boundary = s.substr(pos + 1);

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
FileUploader::parseFormDataFragment(const std::string& fragment)
{
    _parser->parse(fragment, reinterpret_cast<uintptr_t>(this));
}

bool
FileUploader::isDone(void) const
{
    return _parser->getState() == HTTP::FormDataParser::DONE;
}
