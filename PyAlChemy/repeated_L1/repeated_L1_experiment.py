
import sys
sys.path.insert(0, '..')

import pandas as pd
import time
import pickle
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
from PyAlchemy import check_reaction_graph
import os
import json
import copy
import random

DIRNAME = "repeated_L1"
BIND_FREE_VARS = True
RSEED = 1337 #random.randrange(sys.maxsize)

MAX_OBS = 1000
HEAP_SIZE = 800
MAX_STEPS = 500
DEPTH_MAX = 5
N = 5

if not os.path.exists("run_data"):
    os.mkdir("run_data")



def get_last_from_runs(long_runs):
    last_compositions = dict()

    for sim in long_runs:
        ts_file = sim["savename"]
        ts_data = json.load(open(ts_file, "r"))
        last_snap = last_snapshot(ts_data)
        last_compositions[sim["savename"]] = last_snap

    return last_compositions

def last_snapshot(ts_dict, normed = False):
    """Grab the last snapshot from a timeseries"""
    times = [int(k) for k in ts_dict.keys()]
    last_time = max(times)
    last_counts = ts_dict[str(last_time)]
    ids = {v:k for k,v in enumerate(last_counts)}

    return last_counts,ids

def generate_random_perturbation_expressions(n, d):
    randomizer = LambdaRandomizer(max_depth=d, bind_all_free_vars=BIND_FREE_VARS)

    name = f"{n}_random_exprs_depth_{d}_"
    this_sim = Simulation(name, DIRNAME, LambdaReducer(), randomizer,
                            RSEED, n, 10, 10)
    this_sim_params = run_sim(this_sim)
    return this_sim_params

def generate_random_expressions(n, d):
    randomizer = LambdaRandomizer(max_depth=d, bind_all_free_vars=BIND_FREE_VARS)

    name = f"{n}_random_exprs_depth_{d}_"
    this_sim = Simulation(name, DIRNAME, LambdaReducer(), randomizer,
                            RSEED, n, 10, 10)
    this_sim_params = run_sim(this_sim)
    exprs = json.load(open(this_sim_params["savename"], "r"))
    # print(exprs)
    return exprs["0"]


def run_L1_seed():
    L1_seeds = []
    directory = DIRNAME
    n_collisions = 1000000
    output_freq = int(n_collisions/100)
    max_obs = MAX_OBS
    # make the simulation and write input to file
    d = DEPTH_MAX
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

def run_from_input(seed_fname, name, max_obs, n_collisions, dirname, rng_seed):

    randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=BIND_FREE_VARS)
    output_freq = int(n_collisions/100)
    this_sim = Simulation(name, dirname, LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS), randomizer,
                            rng_seed, max_obs, n_collisions, output_freq, copy_allowed=False,
                            input_file=seed_fname)
    this_sim_params = run_sim(this_sim)
    return this_sim_params


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

def copy_files(files, source, dest):
    for f in files:
        src_file = os.path.join(source, f)
        dest_file = os.path.join(dest, os.path.basename(f))
        os.system(f"cp {src_file} {dest_file}")


def run_compositions(compositions, mode=["seed","perturb"]):
    """Randomize with respect to _only_ seed or _only_ a perturbation"""
    n_repeats= 7

    all_sims = []
    names_compositions = [k for k in compositions.items()]
    
    for i in range(len(names_compositions)):
        fname1, compo1 = names_compositions[i]
        for j in range(n_repeats):
            # Get base names
            base1 = os.path.basename(fname1).split(".")[0]
            if "seed" in mode:
                # Run simulation with same input and different RNG seed 
                seed = RSEED + i*n_repeats + j
                run_name = base1 + f"_seed_mode"
                run_comp = copy.deepcopy(compo1[0])
                c_ids = {v:k for k,v in enumerate(run_comp.keys())}
                current_obs = sum([v for v in run_comp.values()])
                max_obs = int(current_obs)
                # Write IC to file
                write_expressions_to_file(run_comp, c_ids, run_name)

                randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
                n_collisions = max_obs*1000
                output_freq = int(n_collisions/100)
                this_sim = Simulation(run_name + f"_run_{j}_{seed}_", DIRNAME, 
                                        LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                        randomizer, seed, max_obs,
                                        n_collisions, output_freq, copy_allowed=True, 
                                        input_file=run_name)
                this_sim_params = run_sim(this_sim)
                all_sims.append(this_sim_params)
                # print("yo")
            if "perturb" in mode:
                # Run simulation with different input and same RNG seed
                # Run simulation with same input and different RNG seed 
                seed = RSEED #+ i*n_repeats + j
                run_name = base1 + f"_perturb_mode"
                run_comp = copy.deepcopy(compo1[0])
                current_obs = sum([v for v in run_comp.values()])
                run_comp = perturb_comp(run_comp, int(0.1*current_obs))
                c_ids = {v:k for k,v in enumerate(run_comp.keys())}
                max_obs = int(current_obs)
                # Write IC to file
                write_expressions_to_file(run_comp, c_ids, run_name+ f"_{j}")

                randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
                n_collisions = max_obs*1000
                output_freq = int(n_collisions/100)
                this_sim = Simulation(run_name + f"_run_{j}_{seed}_", DIRNAME, 
                                        LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                        randomizer, seed, max_obs,
                                        n_collisions, output_freq, copy_allowed=True, 
                                        input_file=run_name+ f"_{j}")
                this_sim_params = run_sim(this_sim)
                all_sims.append(this_sim_params)

    run_params_df = pd.DataFrame(all_sims)

    return run_params_df

def perturb_comp(composition, n, d=6):
    """Perturb a composition by adding n random objects"""
    # get random objects
    random_objects = generate_random_expressions(n, d)
    # remove n objects randomly
    obs = random.sample(list(composition.keys()), n)
    for o in obs:
        composition[o] -= 1
    # add new compostion
    for o in random_objects:
        current_count = composition.get(o, 0)
        composition[o] = current_count + 1
    #return new composition
    composition = {k:v for k,v in composition.items() if v > 0}
    return composition

if __name__ == "__main__":

    int_runs = pd.read_csv("../L1_source/L1_interesting_runs.csv")
    all_files= [k for k in int_runs["savename"]]
    all_files = list(set(all_files))
    copy_files(all_files, "../L1_source/", "run_data")

    last_compositions = get_last_from_runs(int_runs.to_dict("records"))
    pickle.dump(last_compositions, open("Interesting_L1_compositions.pickle", "wb"))

    run_params_df = run_compositions(last_compositions, mode=["seed","perturb"])
    run_params_df.to_csv("Repeated_L1_runs.csv")

    # exprs = generate_random_expressions(100, 5)
    # ids = {v:k for k,v in enumerate(exprs)}
    # write_expressions_to_file(exprs, ids, "random_exprs_initial")
    # initial_runs = []
    # for i in range(10):
    #     initial_run = run_from_input("random_exprs_initial", f"s_{i}_L1", MAX_OBS, 1000000, DIRNAME, RSEED + 100*i)
    #     initial_runs.append(initial_run)
    # initial_runs_df = pd.DataFrame(initial_runs)
    # initial_runs_df.to_csv("repeated_L1_runs.csv")