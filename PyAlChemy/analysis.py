import pandas as pd
import tqdm
import pickle
import json
from numpy import log10, mean, std, median, quantile
import re
import glob
from PyAlchemy import parse_single_file
from lambda_parse import lambda_to_net_props



def get_variables(lambda_str):
    """
    Extracts the free and bound variables from a lambda expression.

    Args:
        lambda_str (str): The lambda expression as a string.

    Returns:
        dict: A dictionary containing two lists:
            - 'free': A list of free variables found in the lambda expression.
            - 'bound': A list of bound variables found in the lambda expression.
    """
    bound_vars_pat = "\\\\x\d+."
    bound_vars = [x.strip(".") for x in set(re.findall(bound_vars_pat, lambda_str))]

    free_vars_pat = "\\\\x\d+"
    free_lambda_terms = re.findall(free_vars_pat, lambda_str)
    free_lambda_terms = [f for f in free_lambda_terms if f not in bound_vars]

    letter_pat = "[a-zA-Z]"
    free_vars = re.findall(letter_pat, lambda_str)
    free_vars = [f for f in free_vars if f != 'x']
    free_vars.extend(free_lambda_terms)
    free_vars = list(set(free_vars))

    return {"free": free_vars, "bound": bound_vars}


def get_unique_expressions(timeseries):
    n_timeseries = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        n_timeseries[this_time] = len(lambdas)
    return n_timeseries

def get_new_expressions(timeseries):
    new_expr_timeseries = dict()
    time_stamps = sorted([int(t) for t in timeseries.keys()])
    all_expressions = set()
    for t in time_stamps:
        lambda_exprs = timeseries[str(t)]
        lambdas = set(list(lambda_exprs.keys()))
        n_new = len(lambdas - all_expressions)
        new_expr_timeseries[t]= n_new
        all_expressions.update(lambdas)
    return new_expr_timeseries

def get_tree_props(timeseries):
    cfactor_timeseries = dict()
    med_depth_timeseries = dict()
    branching_factor_timeseries = dict()

    for time_stamp, lambdas in tqdm.tqdm(timeseries.items()):
        this_time = int(time_stamp)
        c_factors = []
        med_depths = []
        branching_factors = []
        for l in lambdas.keys():
            this_props = lambda_to_net_props(l)
            c_factors.append(this_props["c_factor"])
            med_depths.append(this_props["med_depth"])
            branching_factors.append(this_props["branching_factor"])

        cfactor_timeseries[this_time] = c_factors
        med_depth_timeseries[this_time] = med_depths
        branching_factor_timeseries[this_time] = branching_factors

    return med_depth_timeseries, branching_factor_timeseries, cfactor_timeseries

def get_expression_lengths(timeseries):
    len_timeseries = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        lengths = []
        for l in lambdas.keys():
            lengths.append(len(l))
        len_timeseries[this_time] = lengths
    
    return len_timeseries

def get_expression_vars(timeseries):
    var_timeseries = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        vars = []
        for l in lambdas.keys():
            this_v = get_variables(l)
            n_var = len(this_v["free"]) + len(this_v["bound"])
            vars.append(n_var)
        var_timeseries[this_time] = vars
    return var_timeseries

def get_pop_entropy(timeseries):
    pop_e_timeseries = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        ps = []
        for l, count in lambdas.items():
            ps.append(count)
        tot_p = sum(ps)
        ps = [float(p)/float(tot_p) for p in ps]
        H = sum([-p*log10(p) for p in ps])
        pop_e_timeseries[this_time] =  H
    return pop_e_timeseries

def get_identity_count(timeseries):
    I_count = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        I_count[this_time] = lambdas.get("\\x1.x1", 0)
    return I_count

def get_expr_count(timeseries, expr):
    I_count = dict()
    for time_stamp, lambdas in timeseries.items():
        this_time = int(time_stamp)
        I_count[this_time] = lambdas.get(expr, 0)
    return I_count

def make_ts_df(ts_data, vars=["count", "entropy", "lengths", "n_vars", "n_novel"]):
    """
    Create a time series DataFrame from the given ts_data.

    Parameters:
    - ts_data (dict): A dictionary containing time series data.
    - vars (list): A list of variables to include in the DataFrame. Default is ["count", "entropy", "lengths", "n_vars", "n_novel"].

    Returns:
    - ts_df (DataFrame): A DataFrame containing the time series data.

    The vars parameter allows you to specify which variables to include in the DataFrame. The available options are:
    - "count": The count of unique expressions over time.
    - "entropy": The population entropy over time.
    - "lengths": The lengths of expressions over time.
    - "n_vars": The number of variables in expressions over time.
    - "n_novel": The number of new expressions introduced at each time step.

    The DataFrame will include columns for the specified variables, as well as additional columns for average, standard deviation, and median values of certain variables.
    """
    # Calculate the count of unique expressions over time
    count_ts = get_unique_expressions(ts_data)
    
    # Calculate the population entropy over time
    entropy_ts = get_pop_entropy(ts_data)
    
    # Calculate the number of new expressions introduced at each time step
    novel_ts = get_new_expressions(ts_data)

    if "lengths" in vars:
        # Calculate the lengths of expressions over time
        lengths_ts = get_expression_lengths(ts_data)
        ave_lengths = {k:mean(v) for k,v in lengths_ts.items()}  # Calculate the average length of expressions
        std_lengths = {k:std(v) for k,v in lengths_ts.items()}   # Calculate the standard deviation of expression lengths
        med_lengths = {k:median(v) for k,v in lengths_ts.items()}  # Calculate the median length of expressions
    
    if "n_vars" in vars:
        # Calculate the number of variables in expressions over time
        var_ts = get_expression_vars(ts_data)
        ave_vars = {k:mean(v) for k,v in var_ts.items()}  # Calculate the average number of variables
        std_vars = {k:std(v) for k,v in var_ts.items()}   # Calculate the standard deviation of variable counts
        med_vars = {k:median(v) for k,v in var_ts.items()}  # Calculate the median number of variables

    if "tree_props" in vars:
        # print("getting tree_props")
        md_ts, bf_ts, cf_ts = get_tree_props(ts_data)

        med_cf = {k:median(v) for k,v in cf_ts.items()}
        med_md = {k:median(v) for k,v in md_ts.items()}
        med_bf = {k:median(v) for k,v in bf_ts.items()}
    # print("Making df")
    # Create a DataFrame to store the time series data
    ts_df = pd.DataFrame([count_ts, entropy_ts, novel_ts]).T
    ts_df.columns = ["count", "pop_entropy", "new_exprs"]
    ts_df["time_step"] = ts_df.index

    # Add columns for average, standard deviation, and median number of variables
    ts_df["ave_num_vars"] = ts_df["time_step"].map(ave_vars)
    ts_df["std_num_vars"] = ts_df["time_step"].map(std_vars)
    ts_df["med_num_vars"] = ts_df["time_step"].map(med_vars)

    # Add columns for average, standard deviation, and median expression length
    ts_df["ave_expr_len"] = ts_df["time_step"].map(ave_lengths)
    ts_df["std_expr_len"] = ts_df["time_step"].map(std_lengths)
    ts_df["med_expr_len"] = ts_df["time_step"].map(med_lengths)
    if "identity_count" in vars:
        I_count = get_identity_count(ts_data)
        ts_df["I_count"] = ts_df["time_step"].map(I_count)
    
    if "tree_props" in vars:
        ts_df["med_c_factor"] = ts_df["time_step"].map(med_cf)
        ts_df["med_med_depth"] = ts_df["time_step"].map(med_md)
        ts_df["med_branching_factor"] = ts_df["time_step"].map(med_bf)


    ts_df.sort_index(inplace=True)
    return ts_df


def analyze_runs(input_files, vars = ["count", "entropy", "lengths", "n_vars", "n_novel"]):

    input_runs = pd.read_csv(input_files,index_col=0)
    all_files = input_runs["savename"]
    all_dfs = []
    for fname in tqdm.tqdm(all_files):
        with open(fname, "r") as f:
            ts_data = json.load(f)
        ave_ts_data = make_ts_df(ts_data, vars=vars)
        ave_ts_data["savename"] = fname
        all_dfs.append(ave_ts_data)
    combined_df = pd.concat(all_dfs)
    merged_df = combined_df.join(input_runs.set_index("savename"), on = "savename")

    return merged_df

def analyze_perturbed_files(fname):

    perturbed_data = pd.read_csv(fname)

    all_ts_dfs = []
    for i, row in perturbed_data.iterrows():
        perturbed_ts_file = row["savename"]
        seed_ts_file = row["parent"]

        with open(perturbed_ts_file, "r") as f:
            perturbed_ts = json.load(f)
        perturbed_ts_df = make_ts_df(perturbed_ts)

        with open(seed_ts_file, "r") as f:
            seed_ts = json.load(f)
        seed_ts_df = make_ts_df(seed_ts)
        
        max_time = max(seed_ts_df["time_step"])
        perturbed_ts_df["time_step"] = perturbed_ts_df["time_step"] + max_time
        combined_ts_df = pd.concat([seed_ts_df, perturbed_ts_df])
        combined_ts_df["name"] = row["name"]
        all_ts_dfs.append(combined_ts_df)
    all_perturbed_df = pd.concat(all_ts_dfs)

    return all_perturbed_df

def analyze_repeated_perturbation(seed_fname, p_motif="p_", vars = ["count", "entropy", "lengths", "n_vars", "n_novel"]):
    # Read the seed data
    # Find all files that match the pattern, which would be all files that represent perturbations of the seed state
    all_perturbation_fnames = glob.glob(f"{p_motif}*_{seed_fname}")

    # Make a map from source to parent
    all_perturbation_fnames = sorted(all_perturbation_fnames, reverse=True)
    daughter_parent = dict()
    for p_fname in all_perturbation_fnames:
        p_data = pd.read_csv(p_fname)
        this_d_p_map = dict(zip(p_data["savename"], p_data["parent"]))
        daughter_parent = {**daughter_parent, **this_d_p_map}

    all_ts_dfs = dict()  # Dictionary to hold timeseries dataframes, filename to dataframe map
    all_combined_dfs = []  # List to hold combined timeseries dataframes

    # Add the seed file to the list of fnames
    all_perturbation_fnames.append(seed_fname)

    # For each row, retrieve the corresponding time series dataframe from all_ts_dfs
    # and the parent filename from daughter_parent
    for run_fname in all_perturbation_fnames:
        runs_df = pd.read_csv(run_fname)
        # Each row is another simulation run
        for i, row in tqdm.tqdm(runs_df.iterrows()):
            ts_file = row["savename"]
            with open(ts_file, "r") as f:
                ts_data = json.load(f)
            ts_df = make_ts_df(ts_data, vars =vars)
            all_ts_dfs[ts_file] = ts_df
    # Starting with the last run, which is the daughter run with no daughter, find the parent of each run
    last_run = all_perturbation_fnames[0]
    last_run_df = pd.read_csv(last_run, index_col=0)

    # Each row here is a simulation run saved to a different location
    # E.g. last_run_df is a dataframe with a bunch of different simulations, each perturbed from seeds.
    for _, row in last_run_df.iterrows():
        ts_fname = row["savename"]
        # The later run is the daughter
        daughter_df = all_ts_dfs[ts_fname]
        # Get it's parent
        parent_fname = daughter_parent[ts_fname]
        # If the parent is None, then we've reached the end original simulation
        while parent_fname is not None:
            # If there is a parent we want to concatenate the daughter and parent timeseries, and update
            # The daughter time steps to be after the parent time steps

            # Get the parent
            parent_df = all_ts_dfs[parent_fname]
            # Get the last timestep of the parent simulation
            max_time = max(parent_df["time_step"])
            # Update the daughter time steps
            daughter_df["time_step"] = daughter_df["time_step"] + max_time
            # Concatenate the parent and daughter timeseries
            combined_ts_df = pd.concat([parent_df, daughter_df])

            # We want to climb up the tree, so the parent from this step becomes the new daughter
            ts_fname = parent_fname
            # save the last parent for when we exit the while loop
            last_parent = parent_fname
            # Get the new parent
            parent_fname = daughter_parent.get(ts_fname, None)
            # And the new daughter ts is the concatenated ts
            daughter_df = combined_ts_df

        # The save name for the combined timeseries is the last parent filename
        combined_ts_df["savename"] = last_parent  # Set the savename to the last parent filename
        all_combined_dfs.append(combined_ts_df)
    # This is a file that contains the metadata for the concatenated simulations
    all_perturbed_df = pd.concat(all_combined_dfs, ignore_index=True)

    return all_perturbed_df, daughter_parent


def get_daughter_parent_sim_dict(seed_fname, p_motif = "p_"):
    # Read the seed data
    # seed_data = pd.read_csv(seed_fname)
    # Find all perturbation data
    all_perturbation_fnames = glob.glob(f"{p_motif}*_{seed_fname}")

    # Make a map from source to parent
    all_perturbation_fnames = sorted(all_perturbation_fnames, reverse=True)
    daughter_parent = dict()
    for p_fname in all_perturbation_fnames:
        p_data = pd.read_csv(p_fname)
        this_d_p_map = dict(zip(p_data["savename"],p_data["parent"]))
        daughter_parent = {**daughter_parent, **this_d_p_map}
    
    savename = "daughter_parents_" + seed_fname.split(".")[0] + ".pickle"
    with open(savename, "wb") as f:
        pickle.dump(daughter_parent, f)

def jaccard_composition(compo1, compo2):

    A = set(compo1.keys())
    B = set(compo2.keys())
    C = A.intersection(B)
    D = A.union(B)
    return float(len(C))/float(len(D))

def changed_expressions(compo1, compo2):

    changed_counts = []
    A = set(compo1.keys())
    B = set(compo2.keys())
    C = A.union(B)
    
    for c in C:
        a = compo1.get(c, 0)
        b = compo2.get(c, 0)
        changed_counts.append( abs(a - b))
    return sum(changed_counts)

def percent_similarity(compo1, compo2):

    min_counts = []
    A = set(compo1.keys())
    B = set(compo2.keys())
    C = A.union(B)
    d1 = 0
    d2 = 0
    for c in C:
        a = compo1.get(c, 0)
        b = compo2.get(c, 0)
        d1 += a 
        d2 += b 
        min_counts.append( min(a,b) )

    return 200.0*(sum(min_counts)/float(d1+d2))

def get_autocorrelations(metadata_fname):
    all_data = pd.read_csv(metadata_fname)
    ac_dats = []
    for i, row in tqdm.tqdm(all_data.iterrows()):
        fname = row["savename"]
        ac_data = composition_autocorrelation(fname)
        ac_data["savename"] = fname
        ac_dats.append(ac_data)
    all_data = pd.concat(ac_dats)
    return all_data

def get_l2_compositions(s1_name, s2_name, combined_fname):
    
    # First read the compositions of the two seeds from the typical files
    s1_ts = json.load(open(s1_name, "r"))
    s1_typical, _ = last_snapshot(s1_ts)

    s2_ts = json.load(open(s2_name, "r"))
    s2_typical, _ = last_snapshot(s2_ts)

    s1_exprs = set([k for k in s1_typical.keys()])
    s2_exprs = set([k for k in s2_typical.keys()])

    s1_exprs_only = s1_exprs - s2_exprs
    s2_exprs_only = s2_exprs - s1_exprs
    common_exprs = s1_exprs.intersection(s2_exprs)
    # Read the combined timeseries
    combined_ts = json.load(open(combined_fname, "r"))
    combined_typical, _ = last_snapshot(combined_ts)
    combined_exprs = set([k for k in combined_typical.keys()])
    novel_exprs = combined_exprs - s1_exprs - s2_exprs
    # Identify expressions unique to seed 1 and seed 2

    # Find unique expressions from seed 1 in the combined timeseries
    coarse_ts = dict()
    for t, counts in combined_ts.items():
        s1_counts = {k:v for k,v in counts.items() if k in s1_exprs_only}
        s2_counts = {k:v for k,v in counts.items() if k in s2_exprs_only}
        common_counts = {k:v for k,v in counts.items() if k in common_exprs}
        novel_counts = {k:v for k,v in counts.items() if k in novel_exprs}
        coarse_ts[t] = {"s1":sum(s1_counts.values()), 
                            "s2":sum(s2_counts.values()),
                            "common":sum(common_counts.values()),
                            "novel":sum(novel_counts.values())}
    
    return coarse_ts

def compare_ts(ts_1_fname, ts_2_fname):

    ts1 = json.load(open(ts_1_fname, "r"))
    ts2 = json.load(open(ts_2_fname, "r"))
    sim_ts = dict()
    for t in ts1.keys():
        j = jaccard_composition(ts1[t], ts2[t])
        ps = percent_similarity(ts1[t], ts2[t])
        ec = changed_expressions(ts1[t], ts2[t])
        sim_ts[t] = {"jaccard_composition": j,
                        "perc_sim":ps,
                        "changed_exprs":ec}
    return sim_ts

def autocorrelation_fixed(ts_1_fname, lag = 1):

    ts1 = json.load(open(ts_1_fname, "r"))
    sim_ts = dict()
    times = [int(k) for k in ts1.keys()]
    for t in times:
        if (t - lag) in times:
            t1 = str(t)
            t2 = str(t - lag)
            j = jaccard_composition(ts1[t1], ts1[t2])
            ps = percent_similarity(ts1[t1], ts1[t2])
            ec = changed_expressions(ts1[t1], ts1[t2])
            sim_ts[t1] = {"jaccard_composition": j,
                            "perc_sim":ps,
                            "changed_exprs":ec}
    return sim_ts

def compare_ts_to_fixed_composition(ts_fname, fixed_composition):
    ts1 = json.load(open(ts_fname, "r"))
    sim_ts = dict()
    for t in ts1.keys():
        sim_ts[t] = jaccard_composition(ts1[t], fixed_composition)
    return sim_ts

def composition_autocorrelation(ts_fname, lag = None):
    ts1 = json.load(open(ts_fname, "r"))
    sim_ts = dict()
    ntimes = len(ts1.keys())
    lines = []
    for t1 in range(ntimes):
        for t2 in range(t1):
            l = {"t1":t1, 
                    "t2":t2, 
                    "sim":jaccard_composition(ts1[str(t1)], ts1[str(t2)]), 
                    "changed_exprs": changed_expressions(ts1[str(t1)], ts1[str(t2)]),
                    "perc_sim": percent_similarity(ts1[str(t1)], ts1[str(t2)]),
                    "lag":t1-t2, 
                    "exprs": len(ts1[str(t1)].keys())}
            lines.append(l)
    sim_ts = pd.DataFrame(lines)
    return sim_ts


if __name__ == "__main__":
    # all_ts_df, daughter_parent_dict = analyze_repeated_perturbation("L0_seeds.csv")
    # all_ts_df.to_csv("L0_hunt_analyzed.csv")
    print("hello")