import pandas as pd
import time
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
import os

DIRNAME = "hunt_for_L0"
RSEED = 100 #random.randrange(sys.maxsize)
print(RSEED)

def generate_random_perturbation_expressions(n, d):
    randomizer = LambdaRandomizer(max_depth=d)

    name = f"{n}_random_exprs_depth_{d}_"
    this_sim = Simulation(name, DIRNAME, LambdaReducer(), randomizer,
                          RSEED, n, 10, 10)
    this_sim_params = run_sim(this_sim)
    return this_sim_params

def run_L0_seed():
    L0_seeds = []
    directory = DIRNAME
    n_collisions = 1000000
    output_freq = int(n_collisions/100)
    max_obs = 1000
    N = 2
    # make the simulation and write input to file
    depths_to_attempt = range(4,5)
    for d in depths_to_attempt:
        for n in range(N):
            randomizer = LambdaRandomizer(max_depth=d)

            name = f"L0_seed_{n}_depth{d}_"
            start = time.time()
            this_sim = Simulation(name, directory, LambdaReducer(), randomizer,
                                  1337 + 100*d + n, max_obs, n_collisions, output_freq)
            this_sim_params = run_sim(this_sim)
            L0_seeds.append(this_sim_params)
            print(time.time()- start)
    seed_params_df = pd.DataFrame(L0_seeds)
    seed_params_df.to_csv("L0_seeds_runner.csv")
    return seed_params_df

def perturb_all_seeds(seeds_param_df, random_exprs_data):

    seeds_param_df["max_time"] = seeds_param_df["n_collisions"]/seeds_param_df["output_freq"]
    seeds_param_df = seeds_param_df.astype({'max_time':'int'})

    random_expr_file = os.path.join(random_exprs_data["directory"],
                                    random_exprs_data["name"] + str(0))
    perturbation_size = random_exprs_data["max_objects"]

    input_file_map = dict()
    for i, row in seeds_param_df.iterrows():
        seed_file = os.path.join(row["directory"], row["name"] + str(row["max_time"]))
        combined_counts, combined_ids = merge_two_expression_files(random_expr_file, seed_file)
        combined_fname = os.path.join("..", "LambdaReactor",
                                    row["directory"],
                                    "perturbed_n" + str(perturbation_size) + "_" + row["name"] + str(0))
        write_expressions_to_file(combined_counts, combined_ids, combined_fname)
        input_file_map[row["savename"]] = combined_fname
    
    seeds_param_df["perturbed_file"] = seeds_param_df["savename"].map(input_file_map)
    seeds_param_df["perturbation_size"] = perturbation_size

    return seeds_param_df

def rerun_from_input(seed_df, n_collisions):
    perturbed_sims = []
    for i,seed in seed_df.iterrows():
        directory = seed["directory"]
        output_freq = seed["output_freq"]
        randomizer = LambdaRandomizer(max_depth=4)
        name = "perturbed_n" + str(seed["perturbation_size"]) + "_" + seed["name"]
        start = time.time()
        max_obs = seed["max_objects"] + seed["perturbation_size"]
        rng_seed = seed["random_seed"] + max_obs
        this_sim = Simulation(name, directory, LambdaReducer(), randomizer,
                              rng_seed, max_obs, n_collisions, output_freq,
                              input_file=seed["perturbed_file"])
        this_sim_params = run_sim(this_sim)
        this_sim_params["parent"] = seed["savename"]
        perturbed_sims.append(this_sim_params)
        print(time.time()- start)
        
    params_df = pd.DataFrame(perturbed_sims)
    params_df.to_csv("test_L0_perturbed.csv")
    return params_df


if __name__ == "__main__":
    random_exprs_data = generate_random_perturbation_expressions(100, 4)
    seed_files = run_L0_seed()
    merged_files = perturb_all_seeds(seed_files, random_exprs_data)
    rerun_from_input(merged_files, 1000000)