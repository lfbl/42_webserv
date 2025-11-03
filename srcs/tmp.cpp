void	handleUpload(Routing::RequestInfo& reqInfo, const char *buffer)
{
	if (reqInfo._cLength > reqInfo._max_body_size)
	{
		reqInfo._statusCode = 413;
		return;
	}
	std::string input = buffer;
	if (reqInfo._cType == "multipart/form-data")
	{
        const char *headerEnd = NULL;
        for (size_t i = 0; i < totalLength - 3; i++)
        {
            if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                buffer[i+2] == '\r' && buffer[i+3] == '\n')
            {
                headerEnd = buffer + i;
                break;
            }
        }
        if (headerEnd == NULL)
        {
            reqInfo._statusCode = 400;
            return;
        }
        
        size_t bodyStart = (headerEnd - buffer) + 4;
        
        // Boundary aus Content-Type Header extrahieren
        std::string boundary = "--" + reqInfo._boundary;
        const char *boundaryStr = boundary.c_str();
        size_t boundaryLen = boundary.length();
        
        // Durchsuche BYTES nach Boundary
        size_t currentPos = bodyStart;
        while (currentPos < totalLength)
        {
            // Finde nächste Boundary
            const char *boundaryPos = NULL;
            for (size_t i = currentPos; i < totalLength - boundaryLen; i++)
            {
                if (memcmp(buffer + i, boundaryStr, boundaryLen) == 0)
                {
                    boundaryPos = buffer + i;
                    break;
                }
            }
            
            if (boundaryPos == NULL)
                break;
            
            // Gehe über die Boundary hinweg
            size_t partHeaderStart = boundaryPos - buffer + boundaryLen;
            
            // Finde \r\n\r\n nach dieser Boundary
            const char *partHeaderEnd = NULL;
            for (size_t i = partHeaderStart; i < totalLength - 3; i++)
            {
                if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                    buffer[i+2] == '\r' && buffer[i+3] == '\n')
                {
                    partHeaderEnd = buffer + i;
                    break;
                }
            }
            
            if (partHeaderEnd == NULL)
                break;
            
            // Parse Part-Header (filename, etc.)
            std::string partHeader(buffer + partHeaderStart, partHeaderEnd - (buffer + partHeaderStart));
            std::string filename = "";
            
            size_t filenamePos = partHeader.find("filename=\"");
            if (filenamePos != std::string::npos)
            {
                filenamePos += 10;
                size_t filenameEnd = partHeader.find("\"", filenamePos);
                filename = partHeader.substr(filenamePos, filenameEnd - filenamePos);
            }
            
            // Finde Anfang der Datei-Daten
            size_t dataStart = (partHeaderEnd - buffer) + 4;
            
            // Finde Ende der Datei-Daten (nächste Boundary)
            size_t dataEnd = dataStart;
            for (size_t i = dataStart; i < totalLength - boundaryLen - 2; i++)
            {
                if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                    memcmp(buffer + i + 2, boundaryStr, boundaryLen) == 0)
                {
                    dataEnd = i;
                    break;
                }
            }
            
            if (dataEnd == dataStart)
                dataEnd = totalLength;
            
            // Speichere die BINARY-Daten
            if (!filename.empty())
            {
                std::string filepath = reqInfo._uploadPath + "/" + filename;
                std::ofstream file;
                file.open(filepath.c_str(), std::ios::out | std::ios::binary);
                if (!file.is_open())
                {
                    reqInfo._statusCode = 500;
                    return;
                }
                
                // Schreibe RAW BYTES - KEINE String-Verarbeitung!
                file.write(buffer + dataStart, dataEnd - dataStart);
                file.close();
                std::cerr << "Saved file: " << filepath << " (" << (dataEnd - dataStart) << " bytes)" << std::endl;
            }
            
            // Nächste Part
            currentPos = dataEnd + 2 + boundaryLen;
        }
    }
	else
	{
		const char* headerEnd = strstr(buffer, "\r\n\r\n");
		size_t bodyPos = headerEnd - buffer + 4;
		if (bodyPos == std::string::npos)
		{
			reqInfo._statusCode = 400;
			return;
		}
		size_t bodyLength = reqInfo._cLength;
		std::string filename = reqInfo._path;
		std::ofstream file;
		file.open(filename.c_str(), std::ios::out | std::ios::binary);
		if (!file.is_open())
		{
			reqInfo._statusCode = 500;
			return;
		}
		file.write(buffer + bodyPos, bodyLength);
		file.close();
	}
	reqInfo._statusCode = 201;
}
