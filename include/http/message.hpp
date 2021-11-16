#pragma once
#include <map>
#include <string>
#include <iostream>
#include "http/status.hpp"

namespace HTTP {
    static const std::string CRLF = "\r\n";
    
    class Message {
        protected:
            std::map<std::string, std::string> _headers;

        public:
            std::string getHeaderField(const std::string& name) const;
            void setHeaderField(const std::string& name, const std::string& value);

            Message(void);
            Message(const Message& other);
            Message& operator=(const Message& rhs);
            ~Message(void);
    
    };

    class Response: public Message {
    };

    class Request: public Message {
        public:
        std::string _method;
        std::string _URI;
        std::string _protocol;
        std::string _body;
        
        public:
            Request(std::string rawData = "");
            Request(const Request& other);
            Request& operator=(const Request& rhs);
            ~Request(void);

            std::ostream& printHeaders(std::ostream& os);
    };
  
}