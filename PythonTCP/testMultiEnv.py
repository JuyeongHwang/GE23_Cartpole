import random
from custom_env import custom_env
from multi_env import MultiEnv
from PPO import PPO
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.distributions import Categorical
import matplotlib.pyplot as plt

from torch.utils.tensorboard import SummaryWriter
writer = SummaryWriter()
# writer = SummaryWriter('logs/')
# tensorboard --logdir=runs

def main():
    ENVCOUNT = 12
    multi_env = MultiEnv(ENVCOUNT)
    input_dim = 4
    model = PPO(input_dim=input_dim)
    #model.load_state_dict(torch.load(f"./save_weights/new_PPO{ENVCOUNT}_{input_dim}_r10_100000.h5"))
    episode_history = [[],[],[],[],[],[],[],[],[],[],[],[]]
    episode_rewards = [0,0,0,0,0,0,0,0,0,0,0,0]
    personalCount = [0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0]
    probs = []
    actions = []
    states = []

    for r in range(ENVCOUNT):
        state = multi_env.reset(r)
        states.append(state)
        print(f"id: {r}, state: {state}")

    for count in range(100000):
        #print(states)
        print(f"===={count}====")
        for i in range(ENVCOUNT):
            prob = model.pi(torch.from_numpy(np.array(states[i])).float())
            #print(count," ",states[i])
            m = Categorical(prob)
            action = m.sample().item()
            probs.append(prob[action].item())
            actions.append(action)

        n_states, rewards, dones = multi_env.step(actions)

        # print(len(states), " ", len(actions), " ", len(rewards), " ", len(n_states), " ", len(probs), " ", len(dones))
        for i in range(ENVCOUNT):
            personalCount[i] += 1
            transition = (states[i], actions[i], rewards[i], n_states[i], probs[i], dones[i])
            print(transition)
            model.put_data((states[i], actions[i], rewards[i], n_states[i], probs[i], dones[i]))
            if(dones[i] or personalCount[i] > 500):
                if(i==0):
                    writer.add_scalar("Reward", episode_rewards[i], count)
                episode_history[i].append(episode_rewards[i])
                episode_rewards[i] = 0
                personalCount[i] = 0
                multi_env.reset(i)
            else:
                episode_rewards[i] += rewards[i]

        probs.clear()
        actions.clear()
        states.clear()

        states = n_states


        # print("reward : ", episode_rewards)
        if count%300==0 and count !=0:
            print("===trained===")
            for r in range(ENVCOUNT):
                state = multi_env.reset(r)
            model.train_net()
            torch.save(model.state_dict(), f"./save_weights/new_PPO{ENVCOUNT}_{input_dim}_r10_100000.h5")


    '''    
    maxlist = 0
    for i in range(ENVCOUNT):
        plt.plot(list(range(0, len(episode_history[i]))), episode_history[i])
        print(i, " ", episode_history[i])
        if len(episode_history[i]) > maxlist:
            maxlist = len(episode_history[i])
    #plt.xticks(list(range(0,maxlist)))
    '''

    fig, axes = plt.subplots(3,4)


    # for i in range(4):
    #     x =list(range(0, len(episode_history[i])))
    #     y =episode_history[i]
    #     axes[0][i].plot(x,y)
    #
    # for i in range(4):
    #     x =list(range(0, len(episode_history[4+i])))
    #     y =episode_history[4+i]
    #     axes[1][i].plot(x,y)
    #
    # for i in range(4):
    #     x =list(range(0, len(episode_history[8+i])))
    #     y =episode_history[8+i]
    #     axes[2][i].plot(x,y)
    #
    # plt.tight_layout()
    # plt.show()

if __name__ == '__main__':
    main()
    writer.close()



'''


        print("===Step===")
        n_states, rewards, dones = multi_env.step(actions)
        print(len(states), " " ,len(actions), " " ,len(rewards), " " ,len(n_states), " " ,len(probs), " " ,len(dones))
        for i in range(len(states)):
            model.put_data((states[i], actions[i], rewards[i], n_states[i], probs[i], dones[i]))


'''