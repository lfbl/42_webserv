#include "init.hpp"


std::map<std::string, std::string> initializeContentTypes()
{
    std::map<std::string, std::string> mimeTable;
    
    // Web documents
    mimeTable[".html"] = "text/html";
    mimeTable[".htm"] = "text/html";
    mimeTable[".shtml"] = "text/html";
    mimeTable[".css"] = "text/css";
    mimeTable[".xml"] = "text/xml";
    
    // Programming and scripts
    mimeTable[".js"] = "application/javascript";
    mimeTable[".php"] = "application/x-httpd-php";
    mimeTable[".py"] = "text/x-python";
    mimeTable[".json"] = "application/json";
    
    // Document formats
    mimeTable[".txt"] = "text/plain";
    mimeTable[".rtf"] = "application/rtf";
    mimeTable[".pdf"] = "application/pdf";
    mimeTable[".doc"] = "application/msword";
    mimeTable[".xls"] = "application/vnd.ms-excel";
    mimeTable[".ppt"] = "application/vnd.ms-powerpoint";
    
    // Image formats 
    mimeTable[".jpg"] = "image/jpeg";
    mimeTable[".jpeg"] = "image/jpeg"; 
    mimeTable[".png"] = "image/png";
    mimeTable[".gif"] = "image/gif";
    mimeTable[".bmp"] = "image/bmp";
    mimeTable[".svg"] = "image/svg+xml";
    mimeTable[".ico"] = "image/x-icon";
    
    // Audio formats
    mimeTable[".mp3"] = "audio/mpeg";
    mimeTable[".wav"] = "audio/wav";
    mimeTable[".ogg"] = "audio/ogg";
    
    // Video formats
    mimeTable[".mp4"] = "video/mp4";
    mimeTable[".webm"] = "video/webm";
    mimeTable[".avi"] = "video/x-msvideo";
    mimeTable[".mov"] = "video/quicktime";
    
    // Archives and binaries
    mimeTable[".zip"] = "application/zip";
    mimeTable[".gz"] = "application/gzip";
    mimeTable[".tar"] = "application/x-tar";
    mimeTable[".bin"] = "application/octet-stream";
    
    return mimeTable;
}



std::map<int, std::string> createStatusCodesMap()
{
    std::map<int, std::string> codes;
    // 1xx: Informational
    codes.insert(std::make_pair(100, "Continue"));
    codes.insert(std::make_pair(101, "Switching Protocols"));
    codes.insert(std::make_pair(102, "Processing"));
    // 2xx: Success
    codes.insert(std::make_pair(200, "OK"));
    codes.insert(std::make_pair(201, "Created"));
    codes.insert(std::make_pair(202, "Accepted"));
    codes.insert(std::make_pair(203, "Non-Authoritative Information"));
    codes.insert(std::make_pair(204, "No Content"));
    codes.insert(std::make_pair(205, "Reset Content"));
    codes.insert(std::make_pair(206, "Partial Content"));
    codes.insert(std::make_pair(207, "Multi-Status"));
    // 3xx: Redirection
    codes.insert(std::make_pair(300, "Multiple Choices"));
    codes.insert(std::make_pair(301, "Moved Permanently"));
    codes.insert(std::make_pair(302, "Found"));
    codes.insert(std::make_pair(303, "See Other"));
    codes.insert(std::make_pair(304, "Not Modified"));
    codes.insert(std::make_pair(305, "Use Proxy"));
    codes.insert(std::make_pair(307, "Temporary Redirect"));
    codes.insert(std::make_pair(308, "Permanent Redirect"));
    // 4xx: Client Error
    codes.insert(std::make_pair(400, "Bad Request"));
    codes.insert(std::make_pair(401, "Unauthorized"));
    codes.insert(std::make_pair(402, "Payment Required"));
    codes.insert(std::make_pair(403, "Forbidden"));
    codes.insert(std::make_pair(404, "Not Found"));
    codes.insert(std::make_pair(405, "Method Not Allowed"));
    codes.insert(std::make_pair(406, "Not Acceptable"));
    codes.insert(std::make_pair(407, "Proxy Authentication Required"));
    codes.insert(std::make_pair(408, "Request Timeout"));
    codes.insert(std::make_pair(409, "Conflict"));
    codes.insert(std::make_pair(410, "Gone"));
    codes.insert(std::make_pair(411, "Length Required"));
    codes.insert(std::make_pair(412, "Precondition Failed"));
    codes.insert(std::make_pair(413, "Payload Too Large"));
    codes.insert(std::make_pair(414, "URI Too Long"));
    codes.insert(std::make_pair(415, "Unsupported Media Type"));
    codes.insert(std::make_pair(416, "Range Not Satisfiable"));
    codes.insert(std::make_pair(417, "Expectation Failed"));
    codes.insert(std::make_pair(418, "I'm a teapot"));
    codes.insert(std::make_pair(421, "Misdirected Request"));
    codes.insert(std::make_pair(422, "Unprocessable Entity"));
    codes.insert(std::make_pair(423, "Locked"));
    codes.insert(std::make_pair(424, "Failed Dependency"));
    codes.insert(std::make_pair(425, "Too Early"));
    codes.insert(std::make_pair(426, "Upgrade Required"));
    codes.insert(std::make_pair(428, "Precondition Required"));
    codes.insert(std::make_pair(429, "Too Many Requests"));
    codes.insert(std::make_pair(431, "Request Header Fields Too Large"));
    codes.insert(std::make_pair(451, "Unavailable For Legal Reasons"));
    // 5xx: Server Error
    codes.insert(std::make_pair(500, "Internal Server Error"));
    codes.insert(std::make_pair(501, "Not Implemented"));
    codes.insert(std::make_pair(502, "Bad Gateway"));
    codes.insert(std::make_pair(503, "Service Unavailable"));
    codes.insert(std::make_pair(504, "Gateway Timeout"));
    codes.insert(std::make_pair(505, "HTTP Version Not Supported"));
    codes.insert(std::make_pair(506, "Variant Also Negotiates"));
    codes.insert(std::make_pair(507, "Insufficient Storage"));
    codes.insert(std::make_pair(508, "Loop Detected"));
    codes.insert(std::make_pair(510, "Not Extended"));
    codes.insert(std::make_pair(511, "Network Authentication Required"));
    return codes;
}
