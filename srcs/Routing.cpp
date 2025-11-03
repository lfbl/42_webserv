#include "Routing.hpp"

void Routing::RequestInfo::Initialize()
{
    		_statusCode = 200;
			_method = "";
			_path = "";
			_pathInfo = "";
			_filename = "";
			_queryString = "";
			_boundary = "";
			_data = "";
			_cType = "";
			_host = "";
			_protocol = "";
			_cLength = 0;

			_autoIndex = false;
			_useAutoIndex = false;
			_cgiEnabled = false;
			_uploadEnable = false;
			_methodsAllowed[0] = false;
            _methodsAllowed[1] = false;
            _methodsAllowed[2] = false;
			_upload = false;
			_validPaths = "";
			_path_cgi = "";
			_extensionCGI = "";
			_uploadPath = "";
			_root = "";
			_index = "";
			_gatewayInterface = "";
			_serverPort = "";
			_serverName = "";
			_remoteAddr = "" ;
			_httpUserAgent = "";
			_useCGI = false;
		    _max_body_size = 0;
			_connection = "";
			_redirectLocation = "";
		    _postBody = "";
			_STATUS_CODES = createStatusCodesMap();
}