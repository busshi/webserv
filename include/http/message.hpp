#pragma once
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>

#include "Timer.hpp"
#include "config/ConfigItem.hpp"
#include "utils/BinBuffer.hpp"

#include "config/ConfigParser.hpp"
#include "http/MessageParser.hpp"
#include "http/header.hpp"
#include "http/status.hpp"
#include "utils/string.hpp"

/* Everything related to the HTTP protocol */
namespace HTTP {
static const std::string BODY_DELIMITER = "\r\n\r\n";

struct compareIgnoreCase
{
    bool operator()(const std::string& s1, const std::string& s2) const
    {
        const std::string s1Lower = toLowerCase(s1), s2Lower = toLowerCase(s2);

        return std::lexicographical_compare(
          s1Lower.begin(), s1Lower.end(), s2Lower.begin(), s2Lower.end());
    }
};

/**
 * @brief A HTTP message is an all encompassing term that usually refers to a
 HTTP::Request or HTTP::Response. This is the base class HTTP::Request and
 HTTP::Response are inheriting from. It mostly provide a map that contains the
 header fields of the message.
 *
 * @see HTTP::Request
 * @see HTTP::Response
 */

class Message
{
  protected:
    Header _header;

  public:
    std::string getHeaderField(const std::string& name) const;
    void setHeaderField(const std::string& name, const std::string& value);

    Message(void);
    Message(const Message& other);
    Message& operator=(const Message& rhs);
    ~Message(void);

    Header& header(void);
};

class Response : public Message
{
    StatusCode _statusCode;
    int _csock;

    struct MediaTypeEntry
    {
        std::string mediaType;
        std::string extensions;
    };

    Response(const Response& other);
    Response& operator=(const Response& res);

    void _initHeader(void);

  public:
    Buffer<> data, body;

    Response(int csock = -1);
    ~Response(void);

    Response& setStatus(StatusCode statusCode);
    Response& setStatus(unsigned intStatusCode);
    void setContentType(const std::string& path);

    StatusCode getStatus(void) const;

    Response& send(const std::string& s);
    Response& append(const std::string& s);

    Header& header(void);

    void clear(void);

    std::string formatHeader(void) const;
};

class Request : public Message
{
  public:
    Buffer<> body;
    MessageParser* parser;
    Timer timer;

  private:
    std::string _method, _location, _protocol, _origLocation, _queryString;
    int _csockFd, _resourceFd;
    Response _res;
    unsigned long long _timeout;
    ConfigItem* _block;
    unsigned long long _bodySize, _maxBodySize;

    Request(const Request& other);
    Request& operator=(const Request& rhs);

  public:
    Request(int csockfd, const MessageParser::Config& parserConf);
    ~Request(void);

    bool parse(const char* data, size_t n);

    void grabFile(int fd);
    void dropFile(void);
    int getFile(void) const;

    bool isBodyChunked(void) const;
    bool isDone(void) const;

    int getClientFd(void) const;
    const std::string& getMethod(void) const;
    const std::string& getProtocol(void) const;
    const std::string& getLocation(void) const;
    const std::string& getQueryString(void) const;
    const std::string& getOriginalLocation(void) const;
    unsigned long long getCurrentBodySize(void) const;
    unsigned long long getMaxBodySize(void) const;
    ConfigItem* getBlock(void) const;

    void setProtocol(const std::string& protocol);
    void setLocation(const std::string& loc);
    void setOriginalLocation(const std::string& origLocation);
    void setMethod(const std::string& method);
    void setBlock(ConfigItem* block);
    void setCurrentBodySize(unsigned long long size);
    void setMaxBodySize(unsigned long long size);
    void setQueryString(const std::string& queryString);

    void clear(void);

    void rewrite(const std::string& location);

    Response& res(void);

    std::ostream& printHeader(std::ostream& os = std::cout) const;

    std::ostream& log(std::ostream& os) const;
};
}
