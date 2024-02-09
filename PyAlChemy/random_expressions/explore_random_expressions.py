import pandas as pd
import time
from PyAlchemy import run_sim, Simulation, LambdaRandomizer, LambdaReducer


def param_sweep():
    all_params = []
    directory = "random_expression_runner"
    n_collisions = 100000
    output_freq = 1000
    max_obs = 1000
    N = 5
    # make the simulation and write input to file
    depths_to_attempt = range(4,12)
    for d in depths_to_attempt:
        for n in range(N):
            randomizer = LambdaRandomizer(max_depth=d)

            name = f"randomizer{n}_depth{d}"
            start = time.time()
            this_sim = Simulation(name, directory, LambdaReducer(), randomizer,
                                  1337 + 100*n + d, max_obs, n_collisions, output_freq)
            this_sim_params = run_sim(this_sim)
            all_params.append(this_sim_params)
            print(time.time()- start)
    all_params_df = pd.DataFrame(all_params)
    all_params_df.to_csv("param_sweep_random_runner.csv")
    return 0

if __name__ == "__main__":
    param_sweep()