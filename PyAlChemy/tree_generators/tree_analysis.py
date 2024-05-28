import sys
sys.path.insert(0, '..')
import analysis
import pickle
import glob
import lambda_parse 
import pandas as pd


def check_inputs():
    # Check BTree inputs
    with open("btree_output.txt", "r") as f:
        all_btree_exprs =f.readlines()
    all_btree_exprs = [expr.strip("\n") for expr in all_btree_exprs]
    all_btree_exprs = [expr.strip() for expr in all_btree_exprs[1:]]
    
    all_btree_props = [lambda_parse.lambda_to_net_props(expr) for expr in all_btree_exprs]

    btree_df = pd.DataFrame(all_btree_props)
    print("Mean depth: ", btree_df["max_depth"].mean())
    btree_df.to_csv("btree_props.csv")

    # Check Fontana Inputs
    files = glob.glob("random_trees/control_run_*_*_0")
    all_exprs = []
    print(files)
    input("")
    for file in files:
        with open(file, "r") as f:
            this_exprs = f.readlines()
            this_exprs = [expr.split(' ')[0] for expr in this_exprs]
            this_exprs = [expr.strip("\n") for expr in this_exprs]
            this_exprs = [expr.strip() for expr in this_exprs]
            all_exprs.extend(this_exprs)
    all_exprs = list(set(all_exprs))
    all_expr_props = [lambda_parse.lambda_to_net_props(expr) for expr in all_exprs]
    exprs_df = pd.DataFrame(all_expr_props)
    exprs_df.to_csv("fontana_props.csv")

if __name__ == "__main__":
    check_inputs()
    # all_ts_df = analysis.analyze_runs("random_trees.csv", vars = ["count",
    #                                                               "entropy", 
    #                                                               "lengths",
    #                                                               "n_vars",
    #                                                               "n_novel",
    #                                                               "tree_props",
    #                                                               "I_count"])
    # all_ts_df.to_csv("random_trees_analyzed.csv")
