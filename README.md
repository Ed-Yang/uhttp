# uhttp

A simple and ugly C http server.

## Description

In this repo, it implement a simple/ugly http server.  It will process the client request
and respond the requst file in mapped foler of the server.

The http header filed of this sample code is based on the message of Python BaseHTTPServer.

```C
"HTTP/1.0 200 OK\r\n"
"Server: BaseHTTP/0.3 Python/2.7.10\r\n"
"Date: Thu, 31 Aug 2017 06:52:32 GMT\r\n"
"Content-type: application/json\r\n"
```

## Usage

```
```

### Build (debug version)
```
$ git clone https://github.com/Ed-Yang/uhttp.git
$ cd uhttp
$ gcc -g uhttp.c -o uhttp
```
### Run
```
$ ./uhttp 8008
server is running on 8008
```

### client (GET)
```
# install curl
$ sudo apt-get install curl

# Get a json file 'jjj.json'

$ curl http://localhost:8008/jjj.json
```

### Hard-coded flags
```C
#ifdef WIN32
#define LOCAL_FOLDER ".\\json\\" // the mapped folder
#else
#define LOCAL_FOLDER "./json/" // the mapped folder
#endif
```
### NOTE
- almost has no error checking
- ...