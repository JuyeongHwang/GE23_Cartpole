import gym
from custom_env import custom_env_origin
from PPO import PPO
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.distributions import Categorical


def main():
    env = custom_env_origin.custom_env()
    step = 0

    model = PPO(input_dim=2)
    # model.load_state_dict(torch.load("./save_weights/best_ppo.h5"))
    is_solved = False
    episode_reward = 0
    episode_count = 0
    best_score = 100

    state = env.reset()
    while (episode_count<10000):

        prob = model.pi(torch.from_numpy(np.array(state)).float())
        m = Categorical(prob)
        action = m.sample().item()

        n_state, reward, done = env.step(action)

        model.put_data((state, action, reward, n_state, prob[action].item(), done))
        episode_reward += reward
        if(done):
            episode_count+=1
            if(episode_count%50==0):
                model.train_net()
                print("===trained===")
                if (episode_reward > best_score):
                    best_score = episode_reward
                    torch.save(model.state_dict(), "./save_weights/best_ppo.h5")
                    print(f"episode {episode_count}, Reward :{episode_reward} <- best!")
                else:
                    print(f"episode {episode_count}, Reward :{episode_reward}")

            print(f"episode {episode_count}, Reward :{episode_reward}")
            episode_reward = 0
            state = env.reset()




if __name__ == '__main__':
    main()