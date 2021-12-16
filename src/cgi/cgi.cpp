#include "cgi/cgi.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/status.hpp"
#include "utils/string.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

using HTTP::MessageParser;
using std::map;
using std::ostringstream;
using std::string;

#define GET_CGI(loc) reinterpret_cast<CGI*>(loc)

static void
onCgiHeaderField(const string& name, const string& value, uintptr_t cgiLoc)
{
    CGI* cgi = GET_CGI(cgiLoc);

    cgi->header().setField(name, value);
}

static void
onCgiHeaderParsed(uintptr_t cgiLoc)
{
    CGI* cgi = GET_CGI(cgiLoc);
    HTTP::Request* req = cgi->request();
    HTTP::Response* res = req->response();

    res->header().merge(cgi->header());

    string status = res->getHeaderField("status");

    if (!status.empty()) {
        unsigned long long code = parseInt(status, 10);

        if (code >= 400 && code <= 599) {
            throw HTTP::Exception(
              req, HTTP::toStatusCode(code), "Someone wrote cursed php again");
        }

        res->setStatus(parseInt(status, 10));
    }

    res->header().setField("Transfer-Encoding", "chunked");
    res->data = res->formatHeader();
}

static void
onCgiBodyFragment(const Buffer<>& fragment, uintptr_t cgiLoc)
{
    CGI* cgi = GET_CGI(cgiLoc);

    Buffer<> chunk(ntos(fragment.size(), 16) + CRLF);

    chunk += fragment;
    chunk += Buffer<>(CRLF, 2);

    // send each CGI body fragment as a chunk
    cgi->request()->response()->data = chunk;
}

static void
onCgiBodyParsed(uintptr_t cgiLoc)
{
    CGI* cgi = GET_CGI(cgiLoc);

    (void)cgi;

    // terminate chunk
    cgi->request()->response()->data += Buffer<>("0" CRLF CRLF, 5);
}

CGI::CGI(int csockFd,
         HTTP::Request* req,
         const string& cgiExecName,
         const string& filepath)
  : _cgiExecName(cgiExecName)
  , _filepath(filepath)
  , _csockFd(csockFd)
  , _req(req)
  , _hasStarted(false)
  , _pid(-1)
  , parser(0)
{
    _inputFd[0] = -1;
    _inputFd[1] = -1;
    _outputFd[0] = -1;
    _outputFd[1] = -1;
}

static std::pair<const string, string>
transformClientHeaders(const std::pair<const string, string>& p)
{
    // in case header is prefixed by 'X-' don't transform it
    if (p.first.size() >= 2 && toupper(p.first[0]) == 'X' &&
        p.first[1] == '-') {
        return make_pair(toUpperCase(p.first), p.second);
    }

    return make_pair(string("HTTP_" + toUpperCase(p.first)), p.second);
}

bool
CGI::hasStarted(void) const
{
    return _hasStarted;
}

void
CGI::stopParser(void)
{
    parser->stopBodyParsing();
}

void
CGI::_runCgiProcess(void)
{
    destroyHosts();

    for (map<int, CGI*>::const_iterator cit = cgis.begin(); cit != cgis.end();
         ++cit) {
        CGI* cgi = cit->second;
        close(cgi->getInputFd());
        close(cgi->getOutputFd());
    }

    for (map<int, HTTP::Request*>::const_iterator cit = requests.begin();
         cit != requests.end();
         ++cit) {
        close(cit->first);
    }

    // we don't want php-cgi errors to be printed out
    close(STDERR_FILENO);

    if (dup2(_outputFd[0], STDIN_FILENO) == -1) {
        return;
    }

    if (dup2(_inputFd[1], STDOUT_FILENO) == -1) {
        return;
    }

    CLOSE_FD(_outputFd[0]);
    CLOSE_FD(_outputFd[1]);
    CLOSE_FD(_inputFd[0]);
    CLOSE_FD(_inputFd[1]);

    string tmp;

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
    CLOSE_FD(_inputFd[0]);
    CLOSE_FD(_outputFd[1]);

    char** envp = henv.toEnv();

    execle(_cgiExecName.c_str(), _cgiExecName.c_str(), NULL, envp);
}

void
CGI::start(void)
{
    map<string, string> cgiEnv;
    ostringstream oss;

    MessageParser::Config conf;

    memset(&conf, 0, sizeof(conf));
    conf.onHeaderField = onCgiHeaderField;
    conf.onHeaderParsed = onCgiHeaderParsed;
    conf.onBodyFragment = onCgiBodyFragment;
    conf.parseFullBody = true;
    conf.onBodyParsed = onCgiBodyParsed;

    parser = new MessageParser(conf, MessageParser::PARSING_HEADER_FIELD_NAME);

    _hasStarted = true;
    if (pipe(_inputFd) == -1 || pipe(_outputFd) == -1) {
        throw HTTP::Exception(
          _req, HTTP::INTERNAL_SERVER_ERROR, "Could not pipe for CGI");
    }

    _pid = fork();

    if (_pid == -1) {
        throw HTTP::Exception(
          _req, HTTP::INTERNAL_SERVER_ERROR, "Could not fork for CGI");
    }

    if (_pid == 0) {
        _runCgiProcess();
        CLOSE_FD(_inputFd[0]);
        CLOSE_FD(_inputFd[1]);
        CLOSE_FD(_outputFd[0]);
        CLOSE_FD(_outputFd[1]);
        exit(-1);
    }

    fcntl(_inputFd[0], F_SETFL, O_NONBLOCK);
    fcntl(_outputFd[1], F_SETFL, O_NONBLOCK);
    FD_SET(_inputFd[0], &select_rset); // where cgi response will come from
    FD_SET(_outputFd[1],
           &select_wset); // write end of the pipe used to provide cgi's body

    CLOSE_FD(_outputFd[0]); // used by the CGI to read body
    CLOSE_FD(_inputFd[1]);
}

HTTP::Request*
CGI::request(void)
{
    return _req;
}

bool
CGI::parse(const char* data, size_t n)
{
    return parser->parse(data, n, reinterpret_cast<uintptr_t>(this));
}

CGI::~CGI(void)
{
    delete parser;

    FD_CLR(_inputFd[0], &select_rset);
    FD_CLR(_outputFd[1], &select_wset);

    CLOSE_FD(_inputFd[0]);
    CLOSE_FD(_inputFd[1]);
    CLOSE_FD(_outputFd[0]);
    CLOSE_FD(_outputFd[1]);

    if (_pid != -1) {
        int status;

        while (waitpid(_pid, &status, WNOHANG) != _pid) {
            kill(_pid, SIGKILL);
        }
    }
}

int
CGI::getClientFd() const
{
    return _csockFd;
}

int
CGI::getInputFd() const
{
    return _inputFd[0];
}

int
CGI::getOutputFd() const
{
    return _outputFd[1];
}

HTTP::Header&
CGI::header(void)
{
    return _header;
}

bool
CGI::isDone(void) const
{
    return parser && parser->getState() == MessageParser::DONE;
}
