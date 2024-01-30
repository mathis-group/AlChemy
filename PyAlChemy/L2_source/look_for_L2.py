import sys
sys.path.insert(0, '..')

import pandas as pd
import time
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
from PyAlchemy import check_reaction_graph, merge_json_timeseries
import pickle
import os
import numpy as np
import json

if not os.path.exists("run_data"):
    os.mkdir("run_data")


DIRNAME = "hunt_for_L2"
RSEED = 137 #random.randrange(sys.maxsize)

HEAP_SIZE = 800
MAX_STEPS = 500

seed_incr = 0

def typical_snapshot(ts_dict, normed = False):
    """This is also hacky but might be worth keeping"""
    observations = dict()
    for _, counts in ts_dict.items():
        for expr,c in counts.items():
            # if c > 1:
                count_list = observations.get(expr, [])
                count_list.append(c)
                observations[expr] = count_list
    reduced_typical = {k: int(np.median(v)) for k,v in observations.items() if len(v) > 2}
    reduced_typical = {k: v for k,v in reduced_typical.items() if v >=5 }
    # typical = {k: np.median(v) for k,v in observations.items() if len(v) > 3}
    exprs = list(reduced_typical.keys())
    ids = {v:k for k,v in enumerate(exprs)}

    return reduced_typical,ids

def last_snapshot(ts_dict, normed = False):
    """Grab the last snapshot from a timeseries"""
    times = [int(k) for k in ts_dict.keys()]
    last_time = max(times)
    last_counts = ts_dict[str(last_time)]
    ids = {v:k for k,v in enumerate(last_counts)}

    return last_counts,ids


def run_L2_seeds(input_file, daughter_parents):
    """This is a hot mess, rewrite it all with more care."""
    seed_incr = 0

    interesting_df = pd.read_csv(input_file)
    parents_daughters = {v:k for k,v in daughter_parents.items()}

    # Get the combined timeseries from the interesting runs
    long_runs = []
    for int_run in interesting_df["savename"]:

        base_fname = os.path.basename(int_run).split(".")[0] + "_"
        
        this_group = []
        current_name = int_run
        this_group.append(current_name)
        daughter = parents_daughters.get(current_name, None)
        while daughter:
            this_group.append(daughter)
            current_name = daughter
            daughter = parents_daughters.get(current_name, None)
        # grouped_json_files.append(this_group)
    
        combined_ts = merge_json_timeseries(this_group)
        # Get a typical snapshot by taking the median occurance
        typical_counts, typical_ids = last_snapshot(combined_ts, normed=True)
        max_obs = sum([v for v in typical_counts.values()])
        
        # Run those simulations for 100k collisions
        write_expressions_to_file(typical_counts, typical_ids, base_fname + "_typical" )
        randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
        name = f"{base_fname}_"
        n_collisions = max_obs * 100 
        output_freq = int(n_collisions/100)
        this_sim = Simulation(name, DIRNAME, LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                randomizer, RSEED + 100 + seed_incr, max_obs,
                                n_collisions, output_freq, copy_allowed=False, 
                                input_file=base_fname + "_typical")
        seed_incr += 1
        this_sim_params = run_sim(this_sim)
        this_sim_params["original_run"] = int_run
        long_runs.append(this_sim_params)
    
    return long_runs

def get_typical_from_runs(long_runs):
    typical_compositions = dict()

    for sim in long_runs:
        ts_file = sim["savename"]
        ts_data = json.load(open(ts_file, "r"))
        typical_snap = typical_snapshot(ts_data)
        typical_compositions[sim["savename"]] = typical_snap

    return typical_compositions

def get_last_from_runs(long_runs):
    last_compositions = dict()

    for sim in long_runs:
        ts_file = sim["savename"]
        ts_data = json.load(open(ts_file, "r"))
        last_snap = last_snapshot(ts_data)
        last_compositions[sim["savename"]] = last_snap

    return last_compositions


def run_compositions(compositions):
    all_sims = []
    names_compositions = [k for k in compositions.items()]
    print(names_compositions)
    for i in range(len(names_compositions)):
        for j in range(i):
            fname1, compo1 = names_compositions[i]
            fname2, compo2 = names_compositions[j]
            if fname1 != fname2:
                # Get base names
                base1 = os.path.basename(fname1).split(".")[0]
                base2 = os.path.basename(fname2).split(".")[0]
                combo_name = base1 + "_" + base2 + "_combined"
                # Get combined composition
                for k,v in compo1[0].items():
                    compo2[0][k] = int(compo2[0].get(k,0) + v)
                combined_compo = compo2[0]
                combined_ids = {v:k for k,v in enumerate(combined_compo.keys())}
                current_obs = sum([v for v in combined_compo.values()])
                max_obs = int(current_obs * 1.5)
                # Write IC to file
                write_expressions_to_file(combined_compo, combined_ids, combo_name)

                # Run sims
                for i in range(20):
                    randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
                    n_collisions = max_obs*1000
                    output_freq = int(n_collisions/100)
                    this_sim = Simulation(combo_name + f"_run{i}_", DIRNAME, 
                                            LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                            randomizer, RSEED + i, max_obs,
                                            n_collisions, output_freq, copy_allowed=False, 
                                            input_file=combo_name)
                    this_sim_params = run_sim(this_sim)
                    all_sims.append(this_sim_params)
    run_params_df = pd.DataFrame(all_sims)
    
    return run_params_df
    
def copy_files(files, source, dest):
    for f in files:
        src_file = os.path.join(source, f)
        dest_file = os.path.join(dest, os.path.basename(f))
        os.system(f"cp {src_file} {dest_file}")

if __name__ == "__main__":
    
    seed_incr = 0
    daughter_parents = pickle.load(open("../L1_source/L1_daughter_parent_map.pickle", "rb"))
    # Copy run_data from L1_source
    
    all_files = [k for k in daughter_parents.values()]
    all_files.extend([k for k in daughter_parents.keys()])
    # We shouldn't need this last bit here, but I'm not sure why it's not working
    int_runs = pd.read_csv("../L1_source/L1_interesting_runs.csv")
    all_files.extend([k for k in int_runs["savename"]])
    all_files = list(set(all_files))
    copy_files(all_files, "../L1_source/", "run_data")

    long_runs = run_L2_seeds("../L1_source/L1_interesting_runs.csv", daughter_parents)

    last_compositions = get_last_from_runs(long_runs)
    pickle.dump(last_compositions, open("Interesting_L1_compositions.pickle", "wb"))
    last_compositions = pickle.load(open("Interesting_L1_compositions.pickle", "rb"))
    # print(last_compositions)
    all_sims= run_compositions(last_compositions)
    all_sims.to_csv("L2_combinations_last_snap.csv")
