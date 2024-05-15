import sys
sys.path.insert(0, '..')
import analysis
import pickle

if __name__ == "__main__":
    # Analyze the repeated perturbations and keep a dictionary that maps the perturbed experiment (daughter) to the source (experiment)
    all_ts_df, daughter_parent_dict = analysis.analyze_repeated_perturbation("L0_seeds.csv", 
                                                                                vars = ["identity_count", "count", 
                                                                                        "entropy", "lengths", 
                                                                                        "n_vars", "n_novel"])
    # Save the giant timeseries to a CSV for analysis and plotting
    all_ts_df.to_csv("L0_hunt_analyzed_identity.csv")
    # Save the dictionary to a pickle
    pickle.dump(daughter_parent_dict, open("L0_daughter_parent_map.pickle", "wb"))