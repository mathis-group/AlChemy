import subprocess
import os
import glob
import json
import pandas as pd
import random
import networkx as nx
import pickle

LAMBDA_PATH = "/mnt/c/Users/cmathis6/Desktop/AlChemy/LambdaReactor"

class Simulation:
    """
    A class representing a simulation.

    Attributes:
        name (str): The name of the simulation.
        directory (str): The directory where the simulation is stored.
        lambda_reducer (LambdaReducer): The lambda reducer object used in the simulation.
        lambda_randomizer (LambdaRandomizer): The lambda randomizer object used in the simulation.
        random_seed (int): The random seed for the simulation and random lambda generator.
        max_objects (int): The maximum number of objects in the simulation.
        n_collisions (int): The number of collisions to perform.
        output_freq (int): The frequency of output snapshots.
        input_file (str, optional): The initial set of inputs. If None, it will be randomized.
        copy_allowed (bool): Flag indicating whether copy actions are allowed.

    Methods:
        write_sim_params(): Writes the simulation parameters as a formatted string.
        write_sys_params(): Writes the system parameters as a formatted string.
        write_sim(): Writes the simulation parameters, lambda reducer parameters, lambda randomizer parameters,
                    and system parameters to a file.
        get_sim_params(non_basic_vars): Returns a dictionary of simulation parameters, including the parameters
                                        of lambda reducer and lambda randomizer objects.

    """

    def __init__(self, name, directory, lambda_reducer, lambda_randomizer, random_seed,
                 max_objects, n_collisions, output_freq, input_file = None, copy_allowed=True):
        # Where to store this stuff?
        self.directory = directory
        # What about a name?
        self.name = name
        # Random seed for simulation and for random lambda generator
        self.random_seed = random_seed
        # Maximum number of objects in the simulation
        self.max_objects = max_objects
        # Number of collisions to perform
        self.n_collisions = n_collisions
        # How many collisions between outputs?
        self.output_freq = output_freq
        # What is the initial set of inputs? If None it will be randomized
        self.input_file = input_file
        # Are copy actions allowed?
        self.copy_allowed = copy_allowed

        self.lambda_reducer = lambda_reducer
        self.lambda_randomizer = lambda_randomizer

    def write_sim_params(self):
        """
        Writes the simulation parameters as a formatted string.

        Returns:
            str: The formatted string of simulation parameters.
        """
        output_str = (f"\n\tname of simulation =  {self.directory}/{self.name}"
                        f"\n\treaction scheme =  ORIGINAL"
                        f"\n\ttyping basis =  NULL"
                        f"\n\tstore types =  1"
                        f"\n\tnumber of systems =  1"
                        f"\n\tmaximum overall number of objects =  {self.max_objects}"
                        f"\n\tnumber of collisions to perform =  {self.n_collisions}"
                        f"\n\tsnapshot interval =  {self.output_freq}"
                        f"\n\trandom seed =  {self.random_seed}")
        return output_str

    def write_sys_params(self):
        """
        Writes the system parameters as a formatted string.

        Returns:
            str: The formatted string of system parameters.
        """
        if not self.input_file:
            input_file = f"NULL-{self.max_objects}"
        else:
            input_file = self.input_file
        
        if self.copy_allowed != True:
            copy_float = 0.0
        elif self.copy_allowed == True:
            copy_float = 1.0
        output_str = (f"\n>>>>>>>>>>>>>>>>>>>>> SYSTEM 1\n\n"

                    f"\tfile with initial objects = {input_file}\n\n"
                    "\tINITIAL filter expressions =  1\n"
                    "\tINITIAL filter regular expression =  + ^\\\\x[0-9]+\n\n"

                    "\tOPERATOR filter expressions =  1\n"
                    "\tOPERATOR filter regular expression = + ^\\\\x[0-9]+\n\n"

                    "\tARGUMENT filter expressions =  0\n"
                    "\tARGUMENT filter regular expression    - (a*)|(bc)\n\n"

                    "\tRESULT filter expressions =  1\n"
                    "\tRESULT filter regular expression =  + ^\\\\x[0-9]+\n\n"

                    "\t+- functional filters ---------------------+\n"
                    "\t+------------------------------------------+\n"

                    f"\tacceptance frequency for copy actions = {copy_float}\n"

                    "\t+- laws -----------------------------------+\n"
                    "\t+------------------------------------------+\n"

                    "\tnumber of interaction laws =  1\n"
                    "\tlaw (lambda expression) = \\f.\\g.(f)g\n" #
                    "\tprobability of law =  1.0\n")
        return output_str

    def write_sim(self):
        """
        Writes the simulation parameters, lambda reducer parameters, lambda randomizer parameters,
        and system parameters to a file.
        """
        # Check if directory exists, if not make it
        # if not os.path.exists(os.path.join(LAMBDA_PATH,self.directory)):
        #     os.mkdir(os.path.join(LAMBDA_PATH,self.directory))
        # write each piece.
        output_str = "\n>>>>>>>>>>>>>>>>>>>>> simulation parameters\n"
        output_str += self.write_sim_params()
        output_str += "\n>>>>>>>>>>>>>>>>>>>>> lambda reduction machine parameters\n"
        output_str += self.lambda_reducer.write()
        output_str += "\n>>>>>>>>>>>>>>>>>>>>> random expression generator parameters\n"
        output_str += self.lambda_randomizer.write()
        output_str += self.write_sys_params()

        with open(os.path.join(self.directory, self.name+ ".inp"), "w") as f:
            f.write(output_str)
    
    def get_sim_params(self, non_basic_vars=["lambda_reducer", "lambda_randomizer"]):
        """
        Returns a dictionary of simulation parameters, including the parameters of lambda reducer
        and lambda randomizer objects.

        Args:
            non_basic_vars (list, optional): A list of non-basic variables to exclude from the dictionary.
                                             Default is ["lambda_reducer", "lambda_randomizer"].

        Returns:
            dict: A dictionary of simulation parameters.
        """
        sim_class_params = vars(self)
        for var in non_basic_vars:
            value = sim_class_params.pop(var)
            sim_class_params = {**sim_class_params, **vars(value)}
        return sim_class_params

class LambdaReducer:

    def __init__(self, heap=4000, max_steps=10000, symbol_table_size=500,
                 stack_size = 2000, max_len_identifer=10, max_len_basis_types = 100,
                 var_prefix = "x", error_file = None):
            self.heap = heap
            self.max_steps = max_steps
            self.symbol_table_size = symbol_table_size
            self.stack_size =  stack_size
            self.max_len_identifer =  max_len_identifer
            self.max_len_basis_types =  max_len_basis_types
            self.var_prefix =  var_prefix
            self.error_file =  error_file

    def write(self):
        if not self.error_file:
             err_file = "NULL"
        else:
             err_file = self.error_file

        output_str = ""
        output_str = (f"\n\tsize of heap =  {self.heap}"
                      f"\n\tmaximum number of reduction steps =  {self.max_steps}"
                      f"\n\tsize of symbol table =  {self.symbol_table_size}"
                      f"\n\tsize of stack =  {self.stack_size}"
                      f"\n\tmax length of identifiers =  {self.max_len_identifer}"
                      f"\n\tmax length of basis types =  {self.max_len_basis_types}"
                      f"\n\tstandard variable prefix =  {self.var_prefix}"
                      f"\n\terror file =  {err_file}")
        return output_str

class LambdaRandomizer:

    def __init__(self, max_depth = 10, n_vars = 6, bind_all_free_vars = True,
                 p_range_app = (0.3, 0.5), p_range_abs= (0.5, 0.3)):

        self.max_depth = max_depth # maximum expression depth =  10
        self.n_vars = n_vars # number of variables to choose =  6
        self.bind_all_free_vars  = bind_all_free_vars # bind all free variables =  0
        self.p_range_app = p_range_app # probability range for application =  0.3   0.5
        self.p_range_abs = p_range_abs # probability range for abstraction =  0.5   0.3

    def write(self):
        output_str = ""
        output_str += f"\n\tmaximum expression depth =  {self.max_depth}"
        output_str += f"\n\tnumber of variables to choose =  {self.n_vars}"
        output_str += f"\n\tbind all free variables =  {int(self.bind_all_free_vars)}"
        output_str += f"\n\tprobability range for application =  {self.p_range_app[0]}  {self.p_range_app[1]}"
        output_str += f"\n\tprobability range for abstraction =  {self.p_range_abs[0]}  {self.p_range_abs[1]}"

        return output_str


def run_sim(this_sim):
    """
    Runs a simulation using the provided `this_sim` object. This is done by calling the AlChemy executable
    with subprocess and the appropriate input file. The output is then read and saved to a file.

    Parameters:
    - this_sim: The simulation object containing the necessary information.

    Returns:
    - sim_params: The simulation parameters used for running the simulation, including saved data location.

    Raises:
    - FileNotFoundError: If the directory specified in `this_sim` does not exist.
    - IOError: If there is an error writing the simulation file.
    - subprocess.CalledProcessError: If there is an error running the simulation command.
    - IOError: If there is an error reading the simulation results.
    - IOError: If there is an error writing the simulation results to a file.
    """
    # Make the directory if it doesn't exist
    if not os.path.exists(this_sim.directory):
        os.mkdir(this_sim.directory)
    # Write simulation 
    this_sim.write_sim()
    sim_params = this_sim.get_sim_params()
    savename = os.path.join("run_data", str(hash(str(sim_params))) + '.json')
    sim_params["savename"] = savename
    # Run the simulation
    # Get the relative path to input
    run_cmd = [os.path.join(LAMBDA_PATH, "ALCHEMY"), "-f", os.path.join(this_sim.directory, this_sim.name+ ".inp") ]
    subprocess.run(run_cmd)

    # TODO add flag about success to return params 

    # Read/re-write the outputs
    timeseries = read_results(this_sim)
    with open(savename, "w") as f:
        json.dump(timeseries, f)

    return sim_params


def check_reaction_graph(this_sim):
    """
    Check the reaction graph for a given simulation. This uses the functionality in the AlChemy executable
    to perform pairwise operations on the lambda expressions. The output is parsed, and then a reaction graph
    is constructed. The reaction list, and count of the count of each lambda expression is saved to a file,
    and then returned. 

    Args:
        this_sim (Simulation): The simulation object.

    Returns:
        tuple: A tuple containing the reaction list and the reaction graph.

    Raises:
        FileNotFoundError: If the input directory does not exist.

    """
    # Make the directory if it doesn't exist
    if not os.path.exists(this_sim.directory):
        os.mkdir(this_sim.directory)
    # Get the relative path to input
    run_cmd = [os.path.join(LAMBDA_PATH, "ALCHEMY"),"-p", "-f", os.path.join(this_sim.directory, this_sim.name+ ".inp") ]
    subprocess.run(run_cmd)
    ouput_file = this_sim.input_file + ".pairs"

    # Read/re-write the outputs
    reactions = parse_pairwise_file(ouput_file)
    rxn_graph = generate_reaction_graph(reactions)

    # Check the count data and expressions
    count_dict, lambda_id_dict = parse_single_file(this_sim.input_file)
    id_counts = {lambda_id_dict[k]:v for k,v in count_dict.items()}

    save_data = {'reaction_list': reactions, "counts": id_counts}
    save_fname = this_sim.input_file + "_rxn_graph.json"
    with open(save_fname, "w") as f:
        json.dump(save_data, f)

    return save_data, rxn_graph

def read_results(sim):
    """
    Read the results of a simulation.

    Args:
        sim (Simulation): The simulation object.

    Returns:
        dict: A dictionary containing the time-stamp as keys and the parsed data as values.

    """
    # Get the directory
    dir = os.path.join(sim.directory, sim.name)
    all_files = glob.glob(dir + "*")
    # Drop the log file
    all_outputs = [a for a in all_files if "." not in a]
    timeseries = dict()
    for out_file in all_outputs:
        # Time-stamp (this isn't the time, its an integer multiple of output_freq)
        time_stamp = int(out_file.split(sim.name)[-1])
        time_data, _ = parse_single_file(out_file)
        timeseries[int(time_stamp)] = time_data
    return timeseries

def parse_single_file(fname):
    count_dict = dict()
    lambda_id_dict= dict()
    with open(fname, "r") as f:
        all_lines = f.readlines()
        for l in all_lines:
            lambda_data = l.split(" {")
            lambda_str = lambda_data[0]
            remaining_data = lambda_data[1]
            remaining_data = remaining_data.strip("}")
            (id_str, count_str, lambda_type) = remaining_data.split()
            id = int(id_str)
            count = int(count_str)
            count_dict[lambda_str] = count
            lambda_id_dict[lambda_str] = id
    return count_dict, lambda_id_dict

def parse_pairwise_file(pair_fname):
    with open(pair_fname, "r") as f:
        all_lines = f.readlines()

    all_reactions = []
    for l in all_lines:
        (lhs,rhs) = l.split("=>")
        if rhs.strip() == "R":
            pass
        elif "*" in rhs:
            raise Warning("Reaction graph is not closed, try running again with longer collision time")
        else:
            product_split = rhs.split("[")
            product = product_split[0]
            product = product.strip()

            lhs_split = lhs.split(":")
            
            reactant_1 = lhs_split[0].split("[")[0]
            reactant_1 = reactant_1.strip()
            
            reactant_2 = lhs_split[1].split("[")[0]
            reactant_2 = reactant_2.strip()
            all_reactions.append([reactant_1, reactant_2, product])

    return all_reactions

def merge_two_expression_files(f1, f2, max_count = None):
    count_dict_1, id_dict_1 = parse_single_file(f1)
    count_dict_2, id_dict_2 = parse_single_file(f2)

    combined_counts = dict()
    combined_ids = dict()

    lambdas = list(id_dict_1.keys())
    lambdas_2 = list(id_dict_2.keys())
    lambdas.extend(lambdas_2)
    lambdas = list(set(lambdas))
    n = len(lambdas)
    for i in range(n):
        l = lambdas[i]
        combined_counts[l] = count_dict_1.get(l,0) + count_dict_2.get(l,0)
        combined_ids[l] = i
    if max_count:
        current_counts = list(combined_counts.values())
        current_total = sum(current_counts)
        if current_total > max_count:
            n_remove = current_total - max_count
            redo_count = n_remove
            while redo_count > 0:
                current_exprs = list(combined_counts.keys())
                weights = [float(c)/current_total for c in current_counts]
                random_exprs = random.choices(current_exprs, weights=weights)
                redo_count = 0
                for r in random_exprs:
                    if combined_counts[r] > 1:
                        combined_counts[r] -= 1
                    elif combined_counts[r] == 1:
                        combined_counts[r] = 0
                    elif combined_counts[r] == 0:
                        redo_count += 1
                combined_counts = {k:v for k,v in combined_counts.items() if v > 0}
                combined_ids = {k:v for k,v in combined_ids.items() if combined_counts[v] > 0}
    return combined_counts, combined_ids

def merge_json_timeseries(json_list):

    first_ts = json.load(open(json_list[0], "r"))
    time_stamps = first_ts.keys()
    time_stamps = [int(t) for t in time_stamps]
    max_time = max(time_stamps)
    for json_fname in json_list[1:]:
        next_ts = json.load(open(json_fname, "r"))
        next_ts_stamps = next_ts.keys()
        next_ts_stamps_dict = {t: str(int(t) + max_time) for t in next_ts_stamps}
        next_ts_updated = {next_ts_stamps_dict[t]:v for t,v in next_ts.items()}
        first_ts = {**first_ts, **next_ts_updated}
        max_time = max([int(t) for t in next_ts_updated.keys()])
    return first_ts

def write_expressions_to_file(counts, ids, fname):
    
    with open(fname, "w") as open_file:
        for l_expr, c in counts.items():
            id = ids[l_expr]
            line_to_write = l_expr + " {" + str(id) + " " + str(c) + " 0}\n" 
            open_file.write(line_to_write)

def generate_reaction_graph(rxns):

    all_expression_nodes = []
    all_rxn_nodes = []
    all_edges = []
    rxn_num = 0
    for r in rxns:
        all_expression_nodes.extend(r)
        all_rxn_nodes.append(f"rxn_{rxn_num}")

        these_edges = [(r[0],f"rxn_{rxn_num}" ), 
                       (r[1],f"rxn_{rxn_num}" ),
                       (f"rxn_{rxn_num}",r[2])]
        all_edges.extend(these_edges)
        rxn_num += 1


    all_expression_nodes = list(set(all_expression_nodes))
    #all_expression_nodes = [int(i) for i in all_expression_nodes]

    rxn_graph = nx.DiGraph()
    rxn_graph.add_nodes_from(all_expression_nodes, bipartite=0)
    rxn_graph.add_nodes_from(all_rxn_nodes, bipartite=1)
    rxn_graph.add_edges_from(all_edges)

    return rxn_graph

def validate_expressions(exprs_list):

    return None

if __name__ == "__main__":
    print("Ran from main")
    randomizer = LambdaRandomizer()
    reducer = LambdaReducer()

    name = "test_py"
    directory = "test"
    max_obs = 1000
    n_collisions = 100000
    output_freq = int(n_collisions/100.0)

    this_sim = Simulation(name, directory, reducer, randomizer,
               1001011337, max_obs, n_collisions, output_freq)
    run_data = run_sim(this_sim)
    last_file_input_file = f"{run_data['directory']}/{run_data['name']}100"
    check_sim = Simulation(name, directory, reducer, randomizer,
               1001011337, max_obs, n_collisions, output_freq, input_file= last_file_input_file)
    save_data, rxn_graph = check_reaction_graph(check_sim)
