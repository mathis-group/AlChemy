import sys
sys.path.insert(0, '..')

import pandas as pd
import time
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
from PyAlchemy import check_reaction_graph
import os

DIRNAME = "hunt_for_L1"
RSEED = 137

if not os.path.exists("run_data"):
    os.mkdir("run_data")

BIND_FREE_VARS = True
MAX_OBS = 1000
HEAP_SIZE = 800
MAX_STEPS = 500
RANDOM_DEPTH_MIN = 4
RANDOM_DEPTH_MAX = 9
N = 20

def generate_random_perturbation_expressions(n, d):
    randomizer = LambdaRandomizer(max_depth=d, bind_all_free_vars=BIND_FREE_VARS)

    name = f"{n}_random_exprs_depth_{d}_"
    this_sim = Simulation(name, DIRNAME, LambdaReducer(), randomizer,
                            RSEED, n, 10, 10)
    this_sim_params = run_sim(this_sim)
    return this_sim_params

def run_L1_seed():
    L1_seeds = []
    directory = DIRNAME
    n_collisions = 1000000
    output_freq = int(n_collisions/100)
    max_obs = MAX_OBS
    # make the simulation and write input to file
    depths_to_attempt = range(RANDOM_DEPTH_MIN,RANDOM_DEPTH_MAX)
    for d in depths_to_attempt:
        for n in range(N):
            randomizer = LambdaRandomizer(max_depth=d, bind_all_free_vars=BIND_FREE_VARS)

            name = f"L1_seed_{n}_depth{d}_"
            start = time.time()
            this_sim = Simulation(name, directory, LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS), 
                                    randomizer, RSEED + 100*d + n, max_obs, n_collisions, output_freq,
                                    copy_allowed=False)
            this_sim_params = run_sim(this_sim)
            L1_seeds.append(this_sim_params)
            print(time.time()- start)
    seed_params_df = pd.DataFrame(L1_seeds)
    if os.path.exists("L1_seeds.csv"):
        old_params_df = pd.read_csv("L1_seeds.csv", index_col=0)
        combined_params_df = pd.concat([old_params_df, seed_params_df], ignore_index=True)
        combined_params_df.to_csv("L1_seeds.csv")
    else:
        seed_params_df.to_csv("L1_seeds.csv")
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
        combined_fname = os.path.join(row["directory"],
                                    "p_" + str(perturbation_size) + "_" + row["name"] + str(0))
        write_expressions_to_file(combined_counts, combined_ids, combined_fname)
        input_file_map[row["savename"]] = combined_fname
    
    seeds_param_df["perturbed_file"] = seeds_param_df["savename"].map(input_file_map)
    seeds_param_df["perturbation_size"] = perturbation_size

    return seeds_param_df

def rerun_from_input(seed_df, n_collisions, output_dfname= None):
    perturbed_sims = []
    for i,seed in seed_df.iterrows():
        directory = seed["directory"]
        print(f"Directory in rerun_from_input {directory}")
        output_freq = seed["output_freq"]
        randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=BIND_FREE_VARS)
        name = "p_" + str(seed["perturbation_size"]) + "_" + seed["name"]
        start = time.time()
        max_obs = seed["max_objects"] # + seed["perturbation_size"]
        rng_seed = seed["random_seed"] + max_obs
        this_sim = Simulation(name, directory, LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS), randomizer,
                                rng_seed, max_obs, n_collisions, output_freq, copy_allowed=False,
                                input_file=seed["perturbed_file"])
        this_sim_params = run_sim(this_sim)
        this_sim_params["parent"] = seed["savename"]
        # graph_dat = check_reaction_graph(this_sim)
        perturbed_sims.append(this_sim_params)
        print(time.time()- start)
        
    params_df = pd.DataFrame(perturbed_sims)
    if output_dfname:
        combined_df = None
        if os.path.exists(output_dfname):
            old_df = pd.read_csv(output_dfname, index_col=0)
            combined_df = pd.concat([old_df, params_df], ignore_index=True)
            combined_df.to_csv(output_dfname)
        if combined_df is None:
            params_df.to_csv(output_dfname)
    return params_df


if __name__ == "__main__":
    
    seed_files = run_L1_seed()
    # seed_files.to_csv("L1_seeds.csv")
    for i in range(5):
        random_exprs_data = generate_random_perturbation_expressions(int(MAX_OBS/10.0), 4)
        merged_files = perturb_all_seeds(seed_files, random_exprs_data)
        seed_files = rerun_from_input(merged_files, 1000000, output_dfname=f"p_{i}_L1_seeds.csv")