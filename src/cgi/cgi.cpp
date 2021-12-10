#include "cgi/cgi.hpp"
#include "core.hpp"
#include "utils/string.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

using std::map;

#define GET_CGI(loc) reinterpret_cast<CommonGatewayInterface*>(loc)

static void
onCgiHeaderField(const std::string& name,
                 const std::string& value,
                 uintptr_t cgiLoc)
{
    CommonGatewayInterface* cgi = GET_CGI(cgiLoc);

    cgi->header().setField(name, value);
}

static void
onCgiHeaderParsed(uintptr_t cgiLoc)
{
    CommonGatewayInterface* cgi = GET_CGI(cgiLoc);
    HTTP::Request* req = cgi->request();
    HTTP::Response* res = req->response();

    res->header().merge(cgi->header());
    res->header().setField("Transfer-Encoding", "chunked");

    // TODO: special header field status blize

    res->data.append(res->formatHeader());
}

static void
onCgiBodyFragment(const std::string& fragment, uintptr_t cgiLoc)
{
    CommonGatewayInterface* cgi = GET_CGI(cgiLoc);

    // send each CGI body fragment as a chunk
    cgi->request()->response()->data.append(ntos(fragment.size(), 16) + CRLF +
                                            fragment + CRLF);
}

static void
onCgiBodyParsed(uintptr_t cgiLoc)
{
    CommonGatewayInterface* cgi = GET_CGI(cgiLoc);

    cgi->request()->response()->data.append("0" CRLF CRLF);
}

CommonGatewayInterface::CommonGatewayInterface(int csockFd,
                                               HTTP::Request* req,
                                               const std::string& cgiExecName,
                                               const std::string& filepath)
  : _cgiExecName(cgiExecName)
  , _filepath(filepath)
  , _csockFd(csockFd)
  , _req(req)
  , _hasStarted(false)
  , _parser(0)
  , _pid(-1)
{
    _inputFd[0] = -1;
    _inputFd[1] = -1;
    _outputFd[0] = -1;
    _outputFd[1] = -1;
}

static std::pair<const std::string, std::string>
transformClientHeaders(const std::pair<const std::string, std::string>& p)
{
    // in case header is prefixed by 'X-' don't transform it
    if (p.first.size() >= 2 && toupper(p.first[0]) == 'X' &&
        p.first[1] == '-') {
        return make_pair(toUpperCase(p.first), p.second);
    }

    return make_pair(std::string("HTTP_" + toUpperCase(p.first)), p.second);
}

bool
CommonGatewayInterface::hasStarted(void) const
{
    return _hasStarted;
}

void
CommonGatewayInterface::stopParser(void)
{
    _parser->stopBodyParsing();
}

void
CommonGatewayInterface::start(void)
{
    std::map<std::string, std::string> cgiEnv;
    std::string tmp;
    std::ostringstream oss;

    HttpParser::Config conf;

    memset(&conf, 0, sizeof(conf));
    conf.onHeaderField = onCgiHeaderField;
    conf.onHeaderParsed = onCgiHeaderParsed;
    conf.onBodyFragment = onCgiBodyFragment;
    conf.parseFullBody = true;
    conf.onBodyParsed = onCgiBodyParsed;

    _parser = new HttpParser(conf, HttpParser::PARSING_HEADER_FIELD_NAME);

    _hasStarted = true;

    if (pipe(_inputFd) == -1) {
        perror("CGI pipe input: ");
    }

    if (pipe(_outputFd) == -1) {
        perror("CGI pipe output: ");
    }

    HTTP::Header henv;
    // Merge all the request header fields into the CGI environment, prefixing
    // each field with "HTTP_"
    henv.merge(_req->header(), &transformClientHeaders);

    // Server information

    henv.setField("SERVER_PROTOCOL", _req->getProtocol());
    henv.setField("SERVER_SOFTWARE", henv.getField("Server"));

    henv.setField("REDIRECT_STATUS", "200");

    henv.setField("SCRIPT_FILENAME", _filepath);
    henv.setField("SCRIPT_NAME", _filepath);

    tmp = henv.getField("HTTP_CONTENT-LENGTH");
    if (!tmp.empty()) {
        henv.setField("CONTENT_LENGTH", henv.getField("HTTP_CONTENT-LENGTH"));
    }

    tmp = henv.getField("HTTP_CONTENT-TYPE");
    if (!tmp.empty()) {
        henv.setField("CONTENT_TYPE", henv.getField("HTTP_CONTENT-TYPE"));
    }

    henv.setField("REMOTE_HOST", henv.getField("Host"));
    henv.setField("HTTPS", "off");
    henv.setField("REQUEST_METHOD", _req->getMethod());
    henv.setField("PATH_INFO", _filepath);

    _pid = fork();

    if (_pid == -1) {
        perror("CGI fork: ");
    }

    if (_pid == 0) {

        destroyHosts();

        for (map<int, CommonGatewayInterface*>::const_iterator cit =
               cgis.begin();
             cit != cgis.end();
             ++cit) {
            CommonGatewayInterface* cgi = cit->second;
            close(cgi->getInputFd());
            close(cgi->getOutputFd());
        }

        for (map<int, HTTP::Request*>::const_iterator cit = requests.begin();
             cit != requests.end();
             ++cit) {
            close(cit->first);
        }

        if (dup2(_outputFd[0], STDIN_FILENO) == -1) {
            perror("CGI dup2 output: ");
        }

        if (dup2(_inputFd[1], STDOUT_FILENO) == -1) {
            perror("CGI dup2 input");
        }

        close(_outputFd[0]);
        close(_inputFd[1]);

        char** envp = henv.toEnv();

        execle(_cgiExecName.c_str(), _cgiExecName.c_str(), NULL, envp);

        perror("execvp: ");
        exit(1);
    }

    fcntl(_inputFd[0], F_SETFL, O_NONBLOCK);
    fcntl(_outputFd[1], F_SETFL, O_NONBLOCK);
    FD_SET(_inputFd[0], &select_rset); // where cgi response will come from
    FD_SET(_outputFd[1],
           &select_wset); // write end of the pipe used to provide cgi's body

    close(_outputFd[0]); // used by the CGI to read body
    close(_inputFd[1]);
}

HTTP::Request*
CommonGatewayInterface::request(void)
{
    return _req;
}

bool
CommonGatewayInterface::parse(const std::string& data)
{
    _parser->parse(data, reinterpret_cast<uintptr_t>(this));

    return true;
}

CommonGatewayInterface::~CommonGatewayInterface(void)
{
    delete _parser;

    FD_CLR(_inputFd[0], &select_rset);
    FD_CLR(_outputFd[1], &select_wset);

    if (_inputFd[0] != -1) {
        close(_inputFd[0]);
    }

    if (_outputFd[1] != -1) {
        close(_outputFd[1]);
    }

    if (_pid != -1) {
        int ret = kill(_pid, SIGKILL);

        if (ret == -1) {
            perror("kill: ");
        }

        // make sure not to create a zombie
        waitpid(_pid, 0, 0);
    }
}

int
CommonGatewayInterface::getClientFd() const
{
    return _csockFd;
}

int
CommonGatewayInterface::getInputFd() const
{
    return _inputFd[0];
}

int
CommonGatewayInterface::getOutputFd() const
{
    return _outputFd[1];
}

HTTP::Header&
CommonGatewayInterface::header(void)
{
    return _header;
}

bool
CommonGatewayInterface::isDone(void) const
{
    return _parser && _parser->getState() == HttpParser::DONE;
}
