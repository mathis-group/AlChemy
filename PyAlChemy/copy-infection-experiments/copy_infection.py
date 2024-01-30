import sys
sys.path.insert(0, '..')
import pandas as pd
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
from PyAlchemy import check_reaction_graph
import os
import pickle
import json
import copy
import random

DIRNAME = "copy_infected_L0"
RSEED = 808 #random.randrange(sys.maxsize)
print(RSEED)
if not os.path.exists("run_data"):
    os.mkdir("run_data")

COPY = r"\x1.x1"
HEAP_SIZE = 1000
MAX_STEPS = 600

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


def run_compositions(compositions, n_copiers = [999, 998, 995, 990, 900, 500]):
    all_sims = []
    
    names_compositions = [k for k in compositions.items()]
    
    for i in range(len(names_compositions)):
        fname1, compo1 = names_compositions[i]
        # Get base names
        base1 = os.path.basename(fname1).split(".")[0]

        for c_count in n_copiers:
            current_copy = 0
            combo_name = base1 + f"_copy_{c_count}"
            # Run sims
            for i in range(20):
                # Get combined composition
                combined_compo = copy.deepcopy(compo1[0])
                combined_compo = remove_down_to_n(combined_compo, 1000 - c_count)
                current_copy = combined_compo.get(COPY,0)
                current_copy += c_count
                combined_compo[COPY] = current_copy
                
                combined_ids = {v:k for k,v in enumerate(combined_compo.keys())}
                current_obs = sum([v for v in combined_compo.values()])
                max_obs = int(current_obs)
                # Write IC to file
                write_expressions_to_file(combined_compo, combined_ids, combo_name)

                randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
                n_collisions = max_obs*1000
                output_freq = int(n_collisions/100)
                this_sim = Simulation(combo_name + f"_run{i}_", DIRNAME, 
                                        LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                        randomizer, RSEED + i*c_count + i, max_obs,
                                        n_collisions, output_freq, copy_allowed=True, 
                                        input_file=combo_name)
                this_sim_params = run_sim(this_sim)
                all_sims.append(this_sim_params)
    run_params_df = pd.DataFrame(all_sims)

    return run_params_df

def remove_down_to_n(count_dict, n):
    n_removed = 0
    current_count = sum([v for v in count_dict.values()])
    n_to_remove = current_count - n

    kvs = list(count_dict.items())
    random.shuffle(kvs)

    while current_count > n:
        for k,v in kvs:
            if v > 0:
                count_dict[k] -= 1
                n_removed += 1
                if n_removed == n_to_remove:
                    break
        count_dict = {k:v for k,v in count_dict.items() if v > 0}
        current_count = sum([v for v in count_dict.values()])
        kvs = list(count_dict.items())
        random.shuffle(kvs)
    
    count_dict = {k:v for k,v in count_dict.items() if v > 0}
    current_count = sum([v for v in count_dict.values()])
    return count_dict


def copy_files(files, source, dest):
    for f in files:
        src_file = os.path.join(source, f)
        dest_file = os.path.join(dest, os.path.basename(f))
        os.system(f"cp {src_file} {dest_file}")

if __name__ == "__main__":
    
    seed_incr = 0
    
    int_runs = pd.read_csv("../L0_source/L0_interesting_runs.csv")
    all_files= [k for k in int_runs["savename"]]
    all_files = list(set(all_files))
    copy_files(all_files, "../L0_source/", "run_data")

    last_compositions = get_last_from_runs(int_runs.to_dict("records"))
    pickle.dump(last_compositions, open("Interesting_L0_compositions.pickle", "wb"))
    # last_compositions = pickle.load(open("Interesting_L0_compositions.pickle", "rb"))
    all_sims= run_compositions(last_compositions)
    all_sims.to_csv("copy_infections.csv")
