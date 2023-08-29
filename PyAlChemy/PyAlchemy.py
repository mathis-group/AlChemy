import subprocess
import os
import glob
import json
import pandas as pd

LAMBDA_PATH = "/home/colemathis/AlChemy/LambdaReactor"

class Simulation:

    def __init__(self, name, directory, lambda_reducer, lambda_randomizer, random_seed,
                 max_objects, n_collisions, output_freq, input_file = None):
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

        self.lambda_reducer = lambda_reducer
        self.lambda_randomizer = lambda_randomizer

    def write_sim_params(self):
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

        if not self.input_file:
            input_file = f"NULL-{self.max_objects}"
        else:
            input_file = self.input_file
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

                    "\tacceptance frequency for copy actions = 1.0\n"

                    "\t+- laws -----------------------------------+\n"
                    "\t+------------------------------------------+\n"

                    "\tnumber of interaction laws =  1\n"
                    "\tlaw (lambda expression) =  \\f.\\g.(f)g\n"
                    "\tprobability of law =  1.0\n")
        return output_str

    def write_sim(self):

        # Check if directory exists, if not make it
        if not os.path.exists(os.path.join(LAMBDA_PATH,self.directory)):
            os.mkdir(os.path.join(LAMBDA_PATH,self.directory))
        # write each piece.
        output_str = "\n>>>>>>>>>>>>>>>>>>>>> simulation parameters\n"
        output_str += self.write_sim_params()
        output_str += "\n>>>>>>>>>>>>>>>>>>>>> lambda reduction machine parameters\n"
        output_str += self.lambda_reducer.write()
        output_str += "\n>>>>>>>>>>>>>>>>>>>>> random expression generator parameters\n"
        output_str += self.lambda_randomizer.write()
        output_str += self.write_sys_params()

        with open(os.path.join(LAMBDA_PATH, self.directory, self.name+ ".inp"), "w") as f:
            f.write(output_str)
    
    def get_sim_params(self, non_basic_vars = ["lambda_reducer", "lambda_randomizer"]):

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

    def __init__(self, max_depth = 10, n_vars = 6, bind_all_free_vars = False,
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

def read_results(sim):

    # Get the directory
    dir = os.path.join(LAMBDA_PATH, sim.directory, sim.name)
    all_files = glob.glob(dir + "*")
    # Drop the log file
    all_outputs = [a for a in all_files if "." not in a]
    timeseries = dict()
    for out_file in all_outputs:
        # Time-stamp (this isn't the time, its an integer multiple of output_freq)
        time_stamp = int(out_file.split(sim.name)[-1])
        time_data = parse_single_file(out_file)
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
    return count_dict

def run_sim(this_sim):

    
    this_sim.write_sim()
    savename = os.path.join("run_data", str(hash(this_sim)) + '.json')
    sim_params = this_sim.get_sim_params()
    sim_params["savename"] = savename

    # Run the simulation
    # Get the relative path to input
    run_cmd = ["./ALCHEMY", "-f", os.path.join(this_sim.directory, this_sim.name+ ".inp") ]
    subprocess.run(run_cmd, cwd = LAMBDA_PATH)

    # TODO add flag about success to return params 

    # Read/re-write the outputs
    timeseries = read_results(this_sim)
    with open(savename, "w") as f:
        json.dump(timeseries, f)

    return sim_params

if __name__ == "__main__":
    print("Ran from main")
    # randomizer = LambdaRandomizer()
    # reducer = LambdaReducer()

    # name = "test_py"
    # directory = "test"
    # max_obs = 1000
    # n_collisions = 500000
    # output_freq = 5000

    # this_sim = Simulation(name, directory, reducer, randomizer,
    #            1337, max_obs, n_collisions, output_freq)
    # run_sim()