# from btree_src.fontana_generator import FontanaGen
# from btree_src.btree_generator import BtreeGen
import copy
import random
import sys
import pandas as pd
import os
sys.path.insert(0, '..')


from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, write_expressions_to_file


RSEED = 8085
DIRNAME = "standard_random_trees"
MAX_OBS = 1000
HEAP_SIZE = 800
MAX_STEPS = 500
DEPTH_MAX = 5
N = 5

# Read binary tree expressions from file
def read_expressions(fname):
    """
    Read expressions from file
    """
    with open(fname, "r") as f:
        lines = f.readlines()
    lines = [l.strip() for l in lines]
    lines = [l.strip("\n") for l in lines]
    return lines

# Define write expressions to file
def write_expressions(exprs, fname):
    """
    Write expressions to file
    """
    i = 1
    with open(fname, "w") as f:
        for expr in exprs:
            l = expr + "  {" + str(i) + " 1 0} \n"
            f.write(l)
            i += 1

def generate_expression_dict(all_exprs, n):
    """
    Generate a dict of n expressions from all_exprs
    """
    exprs = []
    if n > len(all_exprs):
        raise ValueError("n is greater than the number of expressions")
    else:
        shuffled_exprs = copy.deepcopy(all_exprs)
        random.shuffle(shuffled_exprs)
        exprs = shuffled_exprs[:n]
    exprs_dict = {expr: 1 for expr in exprs}
    return exprs_dict
# Function to compute topological properties of trees

# Function to validate expressions

# Function to run simulations 
def run_compositions(compositions):
    """Run simulations with different compositions"""
    n_repeats= N

    all_sims = []

    for i in range(len(compositions)):
        for j in range(n_repeats):
            seed = RSEED + i*n_repeats + j
            run_name = f"composition_{i}_repeat{j}_"
            run_comp = copy.deepcopy(compositions[i])
            c_ids = {v:k for k,v in enumerate(run_comp.keys())}
            current_obs = sum([v for v in run_comp.values()])
            max_obs = int(current_obs)
            # Write IC to file
            write_expressions_to_file(run_comp, c_ids, os.path.join(DIRNAME, run_name))

            randomizer = LambdaRandomizer(max_depth=7, bind_all_free_vars=True)
            n_collisions = max_obs*1000
            output_freq = int(n_collisions/100)
            this_sim = Simulation(run_name + f"_run_{j}_{seed}_", DIRNAME, 
                                    LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                    randomizer, seed, max_obs,
                                    n_collisions, output_freq, copy_allowed=False, 
                                    input_file=os.path.join(DIRNAME, run_name))
            this_sim_params = run_sim(this_sim)
            all_sims.append(this_sim_params)

            randomizer = LambdaRandomizer(max_depth=7, bind_all_free_vars=True)
            control_sim = Simulation(f"control_run_{j}_{seed}_", DIRNAME, 
                                    LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                    randomizer, seed, max_obs,
                                    n_collisions, output_freq, copy_allowed=False)

            control_sim_params = run_sim(control_sim)
            all_sims.append(control_sim_params)

    run_params_df = pd.DataFrame(all_sims)

    return run_params_df

if __name__ == "__main__":
    # Read expressions from file
    all_exprs = read_expressions("bound_fontana_more_free.txt")
    # Generate a list of dicts of n expressions
    compositions = []
    for i in range(1, 6):
        exprs = generate_expression_dict(all_exprs, 1000)
        compositions.append(exprs)
    # Run simulations
    run_params_df = run_compositions(compositions)
    # Save results to file
    run_params_df.to_csv("new_std_random_trees.csv")
