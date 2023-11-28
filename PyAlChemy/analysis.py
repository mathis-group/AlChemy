import pandas as pd
import tqdm
import pickle
import json
from numpy import log10, mean, std, median, quantile
import re
import glob


def get_variables(lambda_str):

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

    return {"free": free_vars, "bound":bound_vars}


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


def make_ts_df(ts_data, vars = ["count", "entropy", "lengths", "n_vars", "n_novel"]):

    count_ts = get_unique_expressions(ts_data)
    entropy_ts = get_pop_entropy(ts_data)
    novel_ts = get_new_expressions(ts_data)

    if "lengths" in vars:
        lengths_ts = get_expression_lengths(ts_data)
        ave_lengths = {k:mean(v) for k,v in lengths_ts.items()}
        std_lengths = {k:std(v) for k,v in lengths_ts.items()}
        med_lengths = {k:median(v) for k,v in lengths_ts.items()}
    if "n_vars" in vars:
        var_ts = get_expression_vars(ts_data)
        ave_vars = {k:mean(v) for k,v in var_ts.items()}
        std_vars = {k:std(v) for k,v in var_ts.items()}
        med_vars = {k:median(v) for k,v in var_ts.items()}
    

    ts_df = pd.DataFrame([count_ts, entropy_ts, novel_ts]).T
    ts_df.columns = ["count", "pop_entropy", "new_exprs"]
    ts_df["time_step"] = ts_df.index

    ts_df["ave_num_vars"] = ts_df["time_step"].map(ave_vars)
    ts_df["std_num_vars"] = ts_df["time_step"].map(std_vars)
    ts_df["med_num_vars"] = ts_df["time_step"].map(med_vars)

    ts_df["ave_expr_len"] = ts_df["time_step"].map(ave_lengths)
    ts_df["std_expr_len"] = ts_df["time_step"].map(std_lengths)
    ts_df["med_expr_len"] = ts_df["time_step"].map(med_lengths)
    
    ts_df.sort_index(inplace=True)
    return ts_df


def analyze_runs(input_files):

    input_runs = pd.read_csv(input_files,index_col=0)
    all_files = input_runs["savename"]
    all_dfs = []
    for fname in tqdm.tqdm(all_files):
        with open(fname, "r") as f:
            ts_data = json.load(f)
        ave_ts_data = make_ts_df(ts_data)
        ave_ts_data["savename"] = fname
        all_dfs.append(ave_ts_data)
    combined_df = pd.concat(all_dfs)
    merged_df = combined_df.join(input_runs.set_index("savename"), on = "savename")
    merged_df.to_csv("RerunDF.csv")


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
    # all_perturbed_df.to_csv("BigPerturbed_DF.csv")
    return all_perturbed_df

def analyze_repeated_perturbation(seed_fname, p_motif = "p_"):
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

    all_ts_dfs = dict()
    all_combined_dfs = []

    all_perturbation_fnames.append(seed_fname)
    for run_fname in all_perturbation_fnames:
        runs_df = pd.read_csv(run_fname)
        for i, row in runs_df.iterrows():
            ts_file = row["savename"]
            with open(ts_file, "r") as f:
                ts_data = json.load(f)
            ts_df = make_ts_df(ts_data)
            all_ts_dfs[ts_file] = ts_df
    last_run = all_perturbation_fnames[0]
    last_run_df = pd.read_csv(last_run, index_col=0)
    for i, row in last_run_df.iterrows():
        print(row["bind_all_free_vars"])
        ts_fname = row["savename"]
        print(ts_fname)
        daughter_df = all_ts_dfs[ts_fname]
        parent_fname = daughter_parent[ts_fname]
        while parent_fname is not None:
            parent_df = all_ts_dfs[parent_fname]
            max_time = max(parent_df["time_step"])
            daughter_df["time_step"] = daughter_df["time_step"] + max_time
            combined_ts_df = pd.concat([parent_df, daughter_df])
            ts_fname = parent_fname
            last_parent = parent_fname
            parent_fname = daughter_parent.get(ts_fname, None)
            daughter_df = combined_ts_df
        combined_ts_df["savename"] = last_parent#row["savename"]
        all_combined_dfs.append(combined_ts_df)
    all_perturbed_df = pd.concat(all_combined_dfs, ignore_index=True)
    # all_perturbed_df.to_csv("BigPerturbed_DF.csv")
    return all_perturbed_df


if __name__ == "__main__":
    all_ts_df = analyze_repeated_perturbation("L0_seeds.csv")
    all_ts_df.to_csv("L0_hunt_analyzed.csv")

    all_ts_df = analyze_repeated_perturbation("L1_seeds.csv")
    all_ts_df.to_csv("L1_hunt_analyzed.csv")