import socket

HOST = '127.0.0.1'
PORT = 5050

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((HOST, PORT))

action_array = [ 0,1,1,0,1 ]


client.send(b'Hello I\'m Python,')

for i in range(len(action_array)):
    s_action = str(action_array[i])
    client.send(bytes(s_action,'utf-8'))
    client.send(b',')


while True:
    data = client.recv(1024)
    if data:
        print(data.decode('utf-8'))
