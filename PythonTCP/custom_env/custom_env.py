import gym



class custom_env(gym.Env):
    def __init__(self,id):
        self.id = id
        self.action_space = gym.spaces.Discrete(2)
        self.observation_space = gym.spaces.Box(-180, 180, [1])
        print(f"env of {self.id} created")
        #print(self.observation_space.high)
        #print(self.observation_space.low)

    def step(self, client,action):

        client.send(bytes(str(self.id) + ":" + str(action) + ",", encoding="utf-8"))

        state = []

        # wait till you get a response
        while (True):
            data = client.recv(1024)
            if not data:
                break
            data=data.decode('utf-8')
            #print(data)
            s = data.split(',')
            state.append(float(s[0]))
            state.append(float(s[1])-200)
            state.append(float(s[2]))
            state.append(float(s[3]))
            # state.append(float(s[4])-200)
            break

        reward = 1 # - abs(state[0]) / 20


        done = False
        if (abs(state[0]) > 30):
            done = True
        elif (abs(state[1] > 800)):
            done = True

        # info = {}
        return state, reward, done

    def reset(self,client):

        client.send(bytes(str(self.id)+":"+str(10)+",", encoding= "utf-8"))

        state = [1,1,1,1]
        #print(f"start {self.id}")
        # wait till you get a response
        while (True):
            data = client.recv(1024)
            if not data:
                break
            data = data.decode('utf-8')
            #print(data)

            # state = int.from_bytes(data, byteorder='big', signed=True)
            break
        #print(f"end {self.id}")
        state = [0,0,0,0]
        return state


'''

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((HOST, PORT))
        print("success")

        while (True):
            s.send(bytes([1]))

            while (True):
                data = s.recv(10)
                if not data:
                    break

                print(data)
                break

    except socket.error:
        print("couldn't connect", socket.error)

'''