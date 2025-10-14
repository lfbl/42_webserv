# webserv (42 project) 🚀

## Project Goal 🎯

A minimal, non-blocking HTTP/1.1 web server written in C++98. Serves static websites and executes CGI scripts. Developed as part of the 42 curriculum with emphasis on correct I/O handling, configurability, and browser compatibility.  
**No external HTTP libraries, no frameworks!**

---

## Features ✨

- Configuration file (`.conf`) – via argument or default path
- Multi-port / multi-site listening
- Supports GET, POST, DELETE methods
- File upload via POST
- CGI support (Python, PHP, etc., via fork)
- Custom & default error pages
- Optional directory listing
- HTTP redirects
- Configurable maximum request body size
- Fully non-blocking: all sockets & pipes via single `poll()` (or equivalent)
- Stress-test ready, robust against client issues
- Compatible with standard browsers (Chrome, Firefox, ...)

---

## Requirements (Project Specs) 📜

- Must use a config file (argument or default path)
- No execve for other web servers
- No blocking: all I/O non-blocking, single poll() for all sockets & pipes
- No read/write without poll readiness
- No checking errno after read/write
- Regular disk files may be read/written in blocking mode
- Must provide error pages (default if not configured)
- Only fork for CGI allowed
- Virtual hosts optional, not mandatory
- macOS: fcntl() only with F_SETFL, O_NONBLOCK, FD_CLOEXEC
- Server must never hang, even on faulty requests

---

## Build & Run 🛠️

```sh
make
./webserv [config/webserv.conf]
```
If no config is provided, a default path will be used.

---

## Example Configuration 📝

```conf
server {
    listen 0.0.0.0:8080;
    root /var/www/html;
    index index.html;
    error_page 404 /errors/404.html;
    client_max_body_size 4M;

    location /upload/ {
        methods POST;
        upload_store /var/www/uploads;
    }

    location /cgi-bin/ {
        cgi_pass /usr/bin/python3;
        root /var/www/cgi-bin;
    }

    location / {
        methods GET POST;
        autoindex on;
    }
}
```

---

## Testing 🧪

- Example configs & test files are in the repo.
- Test with standard browsers and tools like `curl`, `ab`, `wrk`, `siege`.
- Compare behavior and headers with NGINX for reference.

---

## FAQ ❓

**Why only one poll()?**  
> Project spec: No threads, everything non-blocking, one central I/O event loop.

**Can I use custom error pages?**  
> Yes! Just specify the path in your config file. If not, defaults are used.

**How do I set up CGI?**  
> Use a location block with `cgi_pass` and the file extension. CGI scripts are executed with proper environment variables.

---

## Authors 👨‍💻

- lfbl  
- elprofesssoro

---

## License 📚

For learning purposes only – part of the 42 School curriculum.
