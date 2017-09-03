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

## Directory
```

uhttp
├── json        <-- file in URL line
├── src         <-- source code
└── web-data    <-- method response files
```

The server will parse the content and if "method:" is found, it will try to 
respond with "web-data/<method>.json" or it will try get respond with
"json/<url-file-portion>"

## Build & Run (debug version)

Get source code from GitHub

$ git clone https://github.com/Ed-Yang/uhttp.git

### Linux (build in command line)

```
$ cd uhttp
$ gcc -g uhttp.c -o uhttp
```

### CMAKE (VC IDE)

Open a "VC Developer Command Window" or in cmd windows run vc environment batch
file, like vcvars32.bat.

```
mkdir vc
cd vc
cmake  ..
Use IDE to open uhttp.sln
```

### CMAKE (Windows - NMake)

Open a "VC Developer Command Window" or in cmd windows run vc environment batch
file, like vcvars32.bat.

```
mkdir nmake
cd nmake
cmake -G "NMake Makefiles" ..
nmake
cd ..
.\nmake\uhttp 80
```

### CMAKE (Linux)
```
mkdir build
cd build
cmake ..
make
cd ..
./build/uhttp 80
```

### CMAKE (macOS)
```
mkdir macos
cd macos
cmake ..
make
cd ..
./macos/uhttp 80
```

## CURL
```
# install curl
$ sudo apt-get install curl

# Get a json file 'jjj.json'
$ curl http://localhost:8008/jjj.json
```

## Build web-data

The script sample command is listed in script.txt.

For example:
```
curl 172.16.1.140/json_rpc -X POST --data "{\"method\":\"systemUtility.config.systemInfo.get\",\"params\":[],\"id\":\"jsonrpc\"}" -o systemUtility.config.systemInfo.get.json
```

if it has to provide username/passwrd, add option "-u username:password".

## NOTE
- almost has no error checking
- ...

