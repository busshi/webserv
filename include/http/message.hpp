#pragma once
#include <map>
#include <string>

namespace HTTP {

    class Message {
        std::map<std::string, std::string> _headers;

        Message(void);

        public:
            std::string getHeaderField(const std::string& name) const;
            void setHeaderField(const std::string& name, const std::string& value);

            Message(const Message& other);
            Message& operator=(const Message& rhs);
    
    };

    class Response: public Message {

    };

    class Request: public Message {

    };
}