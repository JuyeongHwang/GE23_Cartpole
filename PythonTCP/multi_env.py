from custom_env import custom_env
import socket

HOST = '127.0.0.1'
PORT = 5050

class MultiEnv:
    def __init__(self, num_env):
        self.envs = []

        for _ in range(num_env):
            env = custom_env.custom_env(_)
            self.envs.append(env)

        self.hasConnection = False
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            self.socket.connect((HOST, PORT))
            print("successfully connected")
            self.hasConnection = True
        except socket.error:
            print("couldn't connect", socket.error)

    def reset(self,id):
        state = self.envs[id].reset(self.socket)
        return state

    def step(self, actions):
        obs = []
        rewards = []
        dones = []

        for i in range(len(actions)):
            env = self.envs[i]
            ac = actions[i]
            #print(env, " ", ac)
            ob, rew, done = env.step(self.socket,ac)
            obs.append(ob)
            rewards.append(rew)
            dones.append(done)

            if done:
                env.reset(self.socket)

        return obs, rewards, dones