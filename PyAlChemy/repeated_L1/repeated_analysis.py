
import sys
sys.path.insert(0, '..')
import analysis

import os
import pickle
import pandas as pd
import json

def compare_all_pairs(repeated_df):
    compares = {"seed":dict(), "perturb":dict()}

    for _, row in repeated_df.iterrows():
        seed, mode = parse_run_name(row["name"])
        this_dict = compares[mode]
        these_compares = this_dict.get(seed, [])
        these_compares.append(row["savename"])
        this_dict[seed] = these_compares
        compares[mode] = this_dict
    
    all_dfs = []
    for mode_name, mode in compares.items():
        for seed, savenames in mode.items():
            n = len(savenames)
            if n < 2:
                continue
            for i in range(n):
                for j in range(i+1,n):
                    savename1 = savenames[i]
                    savename2 = savenames[j]
                    perc_sim_ts = analysis.compare_ts(savename1, savename2)
                    this_df = pd.DataFrame(list(zip(perc_sim_ts.keys(), perc_sim_ts.values())),
                            columns=['time', 'prec_similiar'])
                    this_df["savename1"] = savename1
                    this_df["savename2"] = savename2
                    this_df["seed"] = seed
                    this_df["mode"] = mode_name
                    all_dfs.append(this_df)
    all_data = pd.concat(all_dfs)
    return all_data

def get_individual_distributions(savename):
    """Convert JSON data to dataframe"""
    with open(savename, "r") as f:
        ts_data = json.load(f)
    all_time_dfs = []
    for time, values in ts_data.items():
        time = int(time)
        this_time_df = pd.DataFrame(values.items(), columns = ["expression", "count"])
        this_time_df["time"] = time
        all_time_dfs.append(this_time_df)
    all_df = pd.concat(all_time_dfs)

    return all_df

def convert_all_ts_to_df(fname):
    """Convert all timeseries to dataframes"""
    metadata_df = pd.read_csv(fname)
    df_list = []
    for i, row in metadata_df.iterrows():
        savename = row["savename"]
        this_df = get_individual_distributions(savename)
        csv_savename = savename.replace(".json", ".csv")
        this_df.to_csv(csv_savename)
        df_list.append(this_df)
    return df_list


def parse_run_name(name):
    splits = name.split("_")
    return splits[0], splits[1]

if __name__ == "__main__":
    all_ts_df = analysis.analyze_runs("repeated_L1_runs.csv", ["count", "entropy", "lengths", "n_vars", "n_novel"])
    all_ts_df.to_csv("repeated_L1_runs_analyzed.csv")
    
    compare_df = compare_all_pairs(pd.read_csv("repeated_L1_runs.csv"))
    compare_df.to_csv("repeated_L1_runs_comparisons.csv")

    df_list = convert_all_ts_to_df("repeated_L1_runs.csv")
    print("analyze")