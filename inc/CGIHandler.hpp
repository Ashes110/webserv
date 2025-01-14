#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <sys/types.h>
#include <sys/wait.h>

#include <map>
#include <string>

#include "Request.hpp"
#include "Server.hpp"

#define CGI_BUFFERSIZE 4096
#define CGI_LIMIT_LOCAL_REDIR 5

class CGIHandler {
	public:
		enum cgiphase { CGIWRITE, CGIRECV, CGISET, CGIFIN };

	private:
		Server *_server;
		Request *_request;
		std::vector<const char *> _env;
		std::string _scriptPath;
		std::string _runtimePath;
		std::string _wbuffer;
		std::string _rbuffer;
		bool _isActive;
		short _revents;
		int _wpfd;
		int _rpfd;
		pid_t _pid;
		cgiphase _phase;
		static std::vector<bool (CGIHandler::*)(const Request &,
												const std::string &)>
		_initMetaVarSetterVec();
		static std::vector<bool (CGIHandler::*)(const Request &,
												const std::string &)>
			_metaVarSetterVec;
		bool _deleteEnv();
		static const std::size_t _timeoutSec;
		const std::time_t _startSec;
		std::size_t _exeTime;

	public:
		CGIHandler();
		bool init(Request &request, Server &server, std::string const &actPath);
		bool activate();
		void setRuntimePath(const std::string &scriptPath);
		void setScriptPath(const std::string &scriptPath);
		const std::string &getRbuffer() const;
		void eraseRbuffer(std::string::size_type readBytes);
		bool isActive() const;
		void setRevents(const short revents);
		short getRevents() const;
		int getMonitoredFd() const;
		pid_t getPid() const;
		bool setAuthType(const Request &request, const std::string &actPath);
		bool setContentLength(const Request &request,
							  const std::string &actPath);
		bool setContentType(const Request &request, const std::string &actPath);
		bool setGateInterface(const Request &request,
							  const std::string &actPath);
		bool setPathInfo(const Request &request, const std::string &actPath);
		bool setPathTranslated(const Request &request,
							   const std::string &actPath);
		bool setQueryString(const Request &request, const std::string &actPath);
		bool setRemoteAddr(const Request &request, const std::string &actPath);
		bool setRemoteHost(const Request &request, const std::string &actPath);
		bool setRemoteMethod(const Request &request,
							 const std::string &actPath);
		bool setScriptName(const Request &request, const std::string &actPath);
		bool setServerName(const Request &request, const std::string &actPath);
		bool setServerPort(const Request &request, const std::string &actPath);
		bool setServerProtocol(const Request &request,
							   const std::string &actPath);
		bool setServerSoftware(const Request &request,
							   const std::string &actPath);
		bool setHttpCookie(const Request &request, const std::string &actPath);
		CGIHandler::cgiphase getCGIPhase() const;
		void setCGIPhase(CGIHandler::cgiphase phase);
		CGIHandler::cgiphase detectCGIPhase() const;
		CGIHandler::cgiphase tryWrite();
		CGIHandler::cgiphase tryRecv();
		pid_t tryWait() const;
		void clear();
};

#endif