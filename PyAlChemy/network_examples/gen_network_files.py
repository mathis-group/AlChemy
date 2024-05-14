import sys
sys.path.insert(0, '..')

import pandas as pd
from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim, merge_two_expression_files, write_expressions_to_file
from PyAlchemy import check_reaction_graph, merge_json_timeseries, parse_single_file
import glob
import os
import json
import random
import matplotlib.pyplot as plt
import networkx as nx


DIRNAME = "network_gen"
RSEED = 1337 #random.randrange(sys.maxsize)

HEAP_SIZE = 80000
MAX_STEPS = 50000

if not os.path.exists("run_data"):
    os.mkdir("run_data")

def last_snapshot(ts_dict, normed = False):
    """Grab the last snapshot from a timeseries"""
    times = [int(k) for k in ts_dict.keys()]
    last_time = max(times)
    last_counts = ts_dict[str(last_time)]
    ids = {v:k+1 for k,v in enumerate(last_counts)}
    return last_counts,ids


def get_network(ts_fname, output_name):
    # Prepare some file anmes
    input_exprs_fname = os.path.basename(ts_fname).split(".json")[0] 
    # Write the counts to file
    with open(test_fname, "r") as f:
        ts_dict = json.load(f)
    counts, ids = last_snapshot(ts_dict)
    write_expressions_to_file(counts, ids, input_exprs_fname)
    # Prepare Simulation
    randomizer = LambdaRandomizer(max_depth=4, bind_all_free_vars=True)
    reducer = LambdaReducer()
    pairwise_sim = Simulation(input_exprs_fname + "_pairwise", DIRNAME, 
                                LambdaReducer(heap = HEAP_SIZE, max_steps=MAX_STEPS),
                                randomizer, RSEED, 100, 100, 100, copy_allowed=False, 
                                input_file=input_exprs_fname)
    pairwise_sim.write_sim()
    # Get the graph from the simulation 
    rxns, rxn_graph = check_reaction_graph(pairwise_sim, output_name)

    return rxns, rxn_graph

def draw_network(graph, node_colors):

    # Define the size in millimeters
    width_mm = 183.0/2.0  # example width in millimeters
    height_mm = 247/3.0  # example height in millimeters
    # Convert size to inches
    width_in = width_mm / 25.4
    height_in = height_mm / 25.4
    plt.figure(figsize=(width_in, height_in))
    # Get nodes and edges
    node_list = nx.nodes(graph)
    edge_list = nx.edges(graph)
    rxn_nodes = [n for n in node_list if "rxn_" in n]
    expr_nodes= [n for n in node_list if "rxn_" not in n]
    expr_pos = {"1": [-1.0, -1.0],
                "2": [-1.0, 1.0],
                "3": [1.0, -1.0],
                "4": [1.0, 1.0]}
    rxn_pos = {r: [2.0*(random.random()-0.5), 2.0*(random.random()-0.5)] for r in rxn_nodes}
    init_pos = {**expr_pos, **rxn_pos}
    # Get the layout of the graph
    kk_pos = nx.kamada_kawai_layout(graph, pos = init_pos)#, seed=3113794652)  # positions for all nodes
    expanded_expr_pos = {k: 2.0*v for k,v in kk_pos.items() if k in expr_nodes}
    rxn_pos = {k: v for k,v in kk_pos.items() if k in rxn_nodes}
    expanded_pos = {**expanded_expr_pos, **rxn_pos}
    expanded_pos = nx.kamada_kawai_layout(graph, pos=expanded_pos)
    # Define some defaults
    options = {"edgecolors": "tab:gray", "alpha": 1.0}
    # Draw the expr nodes and the rxn nodes
    nx.draw_networkx_nodes(graph, expanded_pos, nodelist=rxn_nodes, node_color="whitesmoke", node_size= 150, **options)
    nx.draw_networkx_nodes(graph, expanded_pos, nodelist=expr_nodes, node_color=[node_colors[n] for n in expr_nodes], node_size=450, **options)
    
    # edges
    edge_colors = {}
    for e in edge_list:
        if e[0] in expr_nodes:
            e_color = node_colors[e[0]]
        elif e[1] in expr_nodes:
            e_color = node_colors[e[1]]
        edge_colors[e] = e_color
    nx.draw_networkx_edges(
        graph,
        expanded_pos,
        edgelist=edge_list,
        width=1.5,
        alpha = 0.5,
        edge_color=[edge_colors[e] for e in edge_list],
    )
    # some math labels
    expr_labels = {}
    expr_labels["1"] = "A"
    expr_labels["2"] = "B"
    expr_labels["3"] = "C"
    expr_labels["4"] = "D"
    rxn_labels = {}
    for n in node_list:
        if "rxn_" in n:
            rxn_labels[n] = n.split("_")[-1]
    
    nx.draw_networkx_labels(graph, expanded_pos, expr_labels, font_size=14, font_color="whitesmoke")
    nx.draw_networkx_labels(graph, expanded_pos, rxn_labels, font_size=8)
    plt.tight_layout()
    plt.axis("off")
    plt.savefig("Network_Fig.svg")

def gather_example_ts(fname):
    # Get the directory
    all_files = glob.glob(fname + "*")
    # Drop the log file
    all_outputs = [a for a in all_files if "." not in a]
    timeseries = dict()
    for out_file in all_outputs:
        # Time-stamp (this isn't the time, its an integer multiple of output_freq)
        time_stamp = int(out_file.split("_")[-1])
        time_data, _ = parse_single_file(out_file)
        timeseries[int(time_stamp)] = time_data
    return timeseries

if __name__ == "__main__":
    test_fname = "run_data/2486741155478383424.json"
    output_name = "First_Example_Graph.json"
    # '#1FC9DDFF''#AEFA37FF''#FA7A1FFF''#7A0403FF'
    node_colors = {"1": "#1FC9DDFF", #008080",
                    "2": "#AEFA37FF", # "#DC143C",
                    "3": "#FA7A1FFF", #"#808000",
                    "4": "#7A0403FF"} #4B0082",}
    rxns, rxn_graph  = get_network(test_fname, output_name)

    draw_network(rxn_graph, node_colors)

    ts_dict = gather_example_ts("network_gen/L0_seed_6_depth7_")
    # with open(test_fname, "r") as f:
    #     ts_dict = json.load(f)
    all_rows = []
    for t in ts_dict.keys():
        this_time_values = ts_dict[t]
        for expr, count in this_time_values.items():
            this_row = {"time": t, "expression": expr, "count": count}
            all_rows.append(this_row)
    #print(ts_dict.keys())
    ts_df = pd.DataFrame.from_records(all_rows)
    ts_df.to_csv("example_timeseries.csv")