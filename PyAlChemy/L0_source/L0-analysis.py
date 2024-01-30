import sys
sys.path.insert(0, '..')
import analysis
import pickle

if __name__ == "__main__":
    all_ts_df, daughter_parent_dict = analysis.analyze_repeated_perturbation("L0_seeds.csv", vars = ["identity_count", "count", "entropy", "lengths", "n_vars", "n_novel"])
    all_ts_df.to_csv("L0_hunt_analyzed_identity.csv")
    pickle.dump(daughter_parent_dict, open("L0_daughter_parent_map.pickle", "wb"))
