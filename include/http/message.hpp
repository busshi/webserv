#pragma once
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <netinet/in.h>

#include "http/status.hpp"
#include "utils/string.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include "net/socket.hpp"
#include "http/header.hpp"

/* Everything related to the HTTP protocol */
namespace HTTP {
    static const std::string CRLF = "\r\n";
    static const std::string BODY_DELIMITER = "\r\n\r\n";

    struct compareIgnoreCase {
        bool operator()(const std::string& s1, const std::string& s2) const {
            const std::string s1Lower = toLowerCase(s1), s2Lower = toLowerCase(s2);

            return std::lexicographical_compare(s1Lower.begin(), s1Lower.end(), s2Lower.begin(), s2Lower.end());
        }
    };

    /**
     * @brief A HTTP message is an all encompassing term that usually refers to a HTTP::Request or HTTP::Response.
     This is the base class HTTP::Request and HTTP::Response are inheriting from. It mostly provide a map that contains
     the header fields of the message.
     *
     * @see HTTP::Request
     * @see HTTP::Response
     */

    class Message {
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
     * @brief Type used to represent the data sent by the user agent well known as "HTTP request".
     *
     * This type is mainly intented to represent the data that has been sent, parsing it so that critical information
     * are made available in an easier way.
     */

    class Request: public Message {
        std::string _method, _resourceURI, _URI, _protocol;
        ConfigItem* _serverBlock;
        int _csockFd;
        
        public:
            std::ostringstream body;
            int remContentLength;

            Request(void);
            Request(int csockfd, const std::string& headerRawData);
            Request(const Request& other);
            Request& operator=(const Request& rhs);
            ~Request(void);

            /* No mutators are provided for these: it would make no sense to mutate them */

            const std::string& getMethod(void) const;
            const std::string& getResourceURI(void) const;
            const std::string& getURI(void) const;
            const std::string& getProtocol(void) const;
            const std::string getBody(void) const;

            HTTP::Request& setServerBlock(ConfigItem* serverBlock);
            ConfigItem* getServerBlock(void) const;

            HTTP::Request& _parseHeader(const std::string& headerData);

            std::ostream& printHeader(std::ostream& os = std::cout) const;
    };

    /**
     * @brief Type used to represent the data SENT BACK to the user agent, well known as "HTTP Response".
     *
     * A response must be based on a complete HTTP::Request object: this is the request the response is litteraly
     * "responding" to.
     *
     * A response has a statusCode and a body that respectively indicate what happened (did an error happen?) and which data is
     * sent back to the user agent.
     *
     * Additionally, a response also has its own set of headers, almost none of them being strictly mandatory.
     *
     * @see HTTP::Request
     */

    class Response: public Message {
        StatusCode _statusCode;
        Request _req;
        std::string _body;
        int _csock;

        std::string _sendHeader(void);

        struct MediaTypeEntry {
            std::string mediaType;
            std::string extensions;
        };

        std::string _detectMediaType(const std::string& resource) const;

        public:
            Response(int csock = -1);
            Response(const Request& req);
            Response(const Response& other);
            ~Response(void);
            Response& operator=(const Response& res);

            Response& setStatus(StatusCode statusCode);
            Response& setStatus(unsigned intStatusCode);

            const Request& getReq(void) const;

            Response& sendFile(const std::string& filepath);
            Response& send(const std::string& s);
            Response& append(const std::string& s);
        
            int getClientSocket(void) const;

            std::string str(void);
    };
}