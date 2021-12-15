#pragma once
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>

#include "HttpParser.hpp"
#include "Timer.hpp"
#include "config/ConfigItem.hpp"
#include "utils/BinBuffer.hpp"

#include "config/ConfigParser.hpp"
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

/**
 * @brief Type used to represent the data sent by the user agent well known as
 * "HTTP request".
 *
 * This type is mainly intented to represent the data that has been sent,
 * parsing it so that critical information are made available in an easier way.
 */

class Response; // forward decl

class Request : public Message
{
  public:
    std::ostringstream body;
    HttpParser* parser;
    Timer timer;

  private:
    std::string _method, _location, _protocol, _origLocation;
    int _csockFd;
    Response* _res;
    unsigned long long _timeout;
    ConfigItem* _block;

    Request(const Request& other);
    Request& operator=(const Request& rhs);

  public:
    Request(int csockfd, const HttpParser::Config& parserConf);

    ~Request(void);

    bool parse(const char* data, size_t n);

    bool isBodyChunked(void) const;
    bool isDone(void) const;

    int getClientFd(void) const;
    const std::string& getMethod(void) const;
    const std::string& getProtocol(void) const;
    const std::string& getLocation(void) const;
    const std::string& getOriginalLocation(void) const;
    ConfigItem* getBlock(void) const;

    void setProtocol(const std::string& protocol);
    void setLocation(const std::string& loc);
    void setOriginalLocation(const std::string& origLocation);
    void setMethod(const std::string& method);
    void setBlock(ConfigItem* block);

    void rewrite(const std::string& location);

    Response* createResponse(void);
    Response*& response(void);

    std::ostream& printHeader(std::ostream& os = std::cout) const;

    std::ostream& log(std::ostream& os) const;
};

/**
 * @brief Type used to represent the data SENT BACK to the user agent, well
 * known as "HTTP Response".
 *
 * A response must be based on a complete HTTP::Request object: this is the
 * request the response is litteraly "responding" to.
 *
 * A response has a statusCode and a body that respectively indicate what
 * happened (did an error happen?) and which data is sent back to the user
 * agent.
 *
 * Additionally, a response also has its own set of headers, almost none of them
 * being strictly mandatory.
 *
 * @see HTTP::Request
 */

class Response : public Message
{
    StatusCode _statusCode;
    int _csock;

    struct MediaTypeEntry
    {
        std::string mediaType;
        std::string extensions;
    };

    std::string _detectMediaType(const std::string& resource) const;

  public:
    Buffer<> data, body;

    Response(int csock = -1);
    Response(const Request& req);
    Response(const Response& other);
    ~Response(void);

    Response& operator=(const Response& res);

    Response& setStatus(StatusCode statusCode);
    Response& setStatus(unsigned intStatusCode);

    StatusCode getStatus(void) const;

    Response& sendFile(const std::string& filepath);
    Response& send(const std::string& s);
    Response& append(const std::string& s);

    Header& header(void);

    int getClientSocket(void) const;

    std::string str(void);

    std::string formatHeader(void) const;

    BinBuffer format(void);
};
}
