import pandas as pd
import time
import os
from PyAlchemy import run_sim, Simulation, LambdaRandomizer, LambdaReducer


def extension_runs(list_of_sims):
    all_params = []
    directory = "random_expression_runner"
    n_collisions = 100000
    output_freq = 1000
    max_obs = 1000
    N = 5

    for old_sim in list_of_sims:
        for n in range(N):
            randomizer = LambdaRandomizer()
            reducer = LambdaReducer(heap = 8000,
                                    max_steps = 20000,
                                    stack_size = 4000)
            name = f"extension_{old_sim}_"
            start = time.time()
            this_sim = Simulation(name, directory, LambdaReducer(), randomizer,
                                  8008 + 100*n , max_obs, n_collisions, output_freq,
                                  input_file= os.path.join(directory,old_sim))
            this_sim_params = run_sim(this_sim)
            all_params.append(this_sim_params)
            print(time.time()- start)
    all_params_df = pd.DataFrame(all_params)
    all_params_df.to_csv("Extension_files.csv")
    return 0

if __name__ == "__main__":
    list_of_sim = ['randomizer3_depth6' + "100",
                   'randomizer2_depth7' + "100",
                   'randomizer0_depth8' + "100",
                   'randomizer1_depth9' + "100",
                   'randomizer3_depth11'+ "100",
                   'randomizer1_depth10'+ "100",
                   'randomizer2_depth10'+ "100",
                   'randomizer3_depth10'+ "100",
                   'randomizer0_depth11'+ "100"]
    extension_runs(list_of_sim)