#pragma once
#include <map>
#include <string>
#include <iostream>
#include "http/status.hpp"
#include "utils/string.hpp"

namespace HTTP {
    static const std::string CRLF = "\r\n";
    
    class Message {
        struct compareIgnoreCase {
            bool operator()(const std::string& s1, const std::string& s2) const {
                const std::string s1Lower = toLowerCase(s1), s2Lower = toLowerCase(s2);

                return std::lexicographical_compare(s1Lower.begin(), s1Lower.end(), s2Lower.begin(), s2Lower.end());
            }
        };

        protected:
            std::map<std::string, std::string, compareIgnoreCase> _headers;

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
        std::string _method, _resourceURI, _URI, _protocol, _body;
        
        public:
            Request(std::string rawData = "");
            Request(const Request& other);
            Request& operator=(const Request& rhs);
            ~Request(void);

            /* No mutators are provided for these: it would make no sense to mutate them */

            const std::string& getMethod(void) const;
            const std::string& getResourceURI(void) const;
            const std::string& getURI(void) const;
            const std::string& getProtocol(void) const;
            const std::string& getBody(void) const;

            std::ostream& printHeaders(std::ostream& os);
    };
  
}