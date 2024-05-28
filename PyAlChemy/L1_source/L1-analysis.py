import sys
sys.path.insert(0, '..')
import analysis
import pickle

if __name__ == "__main__":
    # all_ts_df, daughter_parent_dict = analysis.analyze_repeated_perturbation("L1_seeds.csv")
    # all_ts_df.to_csv("L1_hunt_analyzed.csv")

    ac = analysis.get_autocorrelations("L1_interesting_runs.csv")
    ac.to_csv("L1_autocorrelations.csv")