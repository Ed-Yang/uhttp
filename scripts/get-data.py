#!/usr/bin/env python3

"""
    walk trough local javascripts file to make url request to a server and
    save data to file.

    pip3 install requests

    Example (js is in ~/tmp/js folder, and output is in ~/tmp/json):
        $ cd scripts
        $ python3 get-data.py -s localhost:8008 -j ~/tmp/treadnet -o ~/tmp/json 
"""
import requests
import getopt
import os
import sys
import re
from make_req import make_request

rpc_exp = r"RpcGet\('(.+)'"
var_exp = r'\"([a-zA-Z._]+\.)+\"'

def get_method(handle):
    """
    """
    for line in handle:
        #print("line:", line)
        objs = re.findall( rpc_exp, line)
        if objs:
            yield objs[0]
        else:
            objs = re.findall( var_exp, line)
            if objs:
                yield objs[0]+'get'
    
def parse_js(js_folder):
    result = []    
    print('js_folder: ', js_folder)
    for subdir, dirs, files in os.walk(js_folder):
        for file in files:
            print("subdir:", subdir, "dirs:", dirs, "files:", files)
            if file.lower().endswith(".js"):
                js_file = os.path.join(subdir, file)
                print("====> ", js_file)      
                with open(js_file, "r") as f:
                    mlist = get_method(f)
                    #print(list(mlist))
                    for link in mlist:
                        if link not in result:
                            #make_request(server, link, link+'.json')
                            result.append(link)
    #print("===========================")
    #print(result)
    return result

def download_json(server, js_folder, out_folder, usrname="", passwd=""):    
    req_list = parse_js(js_folder)
    for req in req_list:
        fname = out_folder+req+'.json'
        make_request(server, req, fname)
                  
usage_str = 'Usage: %s -s server[:port]  [-j <js-folder>] -o <out-folder>'

def main(argv):
    remote_server = ""
    js_folder = "./"
    out_folder = "./"
    print(os.sys.version)

    try:
        opts, args = getopt.getopt(argv, "hs:j:o:", ["server=", "js-folder=", "out-folder="])
    except getopt.GetoptError:
        print (usage_str)
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print (usage_str)
            sys.exit()
        elif opt in ("-s", "--server"):
            remote_server = arg
        elif opt in ("-j", "--js-folder"):
            js_folder = arg
            if not js_folder.endswith("/"):
                js_folder += '/'
        elif opt in ("-o", "--out-folder"):
            out_folder = arg
            if not out_folder.endswith("/"):
                out_folder += '/'
        
    if not remote_server:
        print ('remote server is not specified !')
        print (usage_str)
        sys.exit()
    
    print('starting download from: %s, js %s out %s' % 
        (remote_server, js_folder, out_folder))
    download_json(remote_server, js_folder, out_folder)
    print('program end.')
    pass


if __name__ == "__main__":
    main(sys.argv[1:])