import requests
import json

# curl localhost/json_rpc -X POST --data 
# "{\"method\":\"fan.status.speed.get\",\"params\":[],\"id\":\"jsonrpc\"}"

def make_request(server, json_method, save_file, usrname="", passwd=""):
    data_exp = "{\"method\":\"%s\",\"params\":[],\"id\":\"jsonrpc\"}"
    url = 'http://' + server + '/' + 'json_rpc'
    payload = data_exp % json_method
    #print('request url: ', url)
    #print('payload: ', payload)
    #print('file: ', save_file)

    try:
        r = requests.post(url, data=payload)
        #print('status: ', r.status_code)
        if 'jsonrpc' in r.text:
            #print('data: ', r.text)
            with open(save_file, "w") as outfile:
                outfile.write(r.text)
                print('url: ', url, 'file:', save_file, 'OK')
        else:
            print('url: ', url, 'method:', json_method, 'not found or malformat !!', r.status_code)
            print('data ==> ', r.text)

    except Exception as e:
        print(e)
        print('url: ', url, 'method:', json_method, 'timeout !!', r.status_code)
        
if __name__ == '__main__':
    make_request("localhost", "fan.status.speed.get", "fan.status.speed.get.json")


