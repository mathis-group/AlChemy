import sys
sys.path.insert(0, '..')
import analysis

import look_for_L2
import os
import pickle
import pandas as pd
import json

def get_last_from_data(filename):
    """Get the last snapshot from a timeseries file"""
    ts_data = json.load(open(filename, "r"))
    last_snap,_ = look_for_L2.last_snapshot(ts_data)

    return last_snap

def get_original_compositions(fnames):

    og_compositions = dict()
    for f in fnames:
        original_runs = f.split("_")[:2]
        original_files = [os.path.join("run_data", f + ".json") for f in original_runs]
        original_compositions = {fo: get_last_from_data(fo) for fo in original_files}
        og_compositions[f] = original_compositions
    return og_compositions

def compare_l2_to_l1_comps(l1_compos_l2_savenames):
    
    sim_l2_dfs = []
    for l2_savename, l1_compos_dict in l1_compos_l2_savenames.items():
        these_sims = []
        for l1_comp in l1_compos_dict.values():
            percent_sim = analysis.compare_ts_to_fixed_composition(l2_savename, l1_comp)
            these_sims.append(percent_sim)
        this_df = pd.DataFrame(list(zip(these_sims[0].keys(), these_sims[0].values(), these_sims[1].values())),
                            columns=['time', 'prec_similiar_L', 'prec_similiar_R'])
        this_df["savename"] = l2_savename
        sim_l2_dfs.append(this_df)
    sim_l2_data = pd.concat(sim_l2_dfs)
    return sim_l2_data 

if __name__ == "__main__":
    all_meta = pd.read_csv("L2_combinations_last_snap.csv")
    #all_ts_df = analysis.analyze_runs("L2_combinations_last_snap.csv")
    l1_compos = get_original_compositions(all_meta["input_file"])
    l2_save_input = dict(all_meta[["savename", "input_file"]].itertuples(index=False, name=None))
    l1_compos_l2_savenames = {k: l1_compos[v] for k,v in l2_save_input.items()}
    comparison_df = compare_l2_to_l1_comps(l1_compos_l2_savenames)
    comparison_df.to_csv("L2_similarity_comparisons.csv")
    #all_ts_df.to_csv("L2_hunt_analyzed.csv")
