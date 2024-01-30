import sys
sys.path.insert(0, '..')
import analysis
import pickle

if __name__ == "__main__":
    all_ts_df = analysis.analyze_runs("-2374329132411800447_copy_infections.csv", ["count", "entropy", "lengths", "n_vars", "n_novel", "identity_count"])
    all_ts_df.to_csv("-2374329132411800447_copy_infections_analyzed.csv")
    # ac = analysis.get_autocorrelations("L1_seeds.csv")
    # ac.to_csv("L1_autocorrelations.csv")