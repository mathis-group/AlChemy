import json
import argparse

from PyAlchemy import Simulation, LambdaRandomizer, LambdaReducer
from PyAlchemy import run_sim

 
# Initialize parser
aparser = argparse.ArgumentParser()
aparser.add_argument("-n", "--n_unique", type=int,
                     help="Number of unique expressions to generate",
                     default=100)

aparser.add_argument("-d", "--depth", type=int,
                     help="Depth of the expressions",
                     default=5)

aparser.add_argument("-b", "--abstraction_p", nargs='+', type=float,
                     help="Probability Range for abstraction",
                     default = [0.5, 0.3])

aparser.add_argument("-p", "--application_p", nargs='+', type=float,
                     help="Probability Range for application",
                     default = [0.3, 0.5])

aparser.add_argument("-f", "--filename", type=str,
                     help= "Filename to store output, if no file is given, result will be printed",
                     default=None)

def generate_random_expressions(n, d, p_range_app, p_range_abs):
    randomizer = LambdaRandomizer(max_depth=d, 
                                  p_range_abs=p_range_abs,
                                  p_range_app=p_range_app)

    name = f"{n}_random_exprs_depth_{d}_"
    this_sim = Simulation(name, "random_expression_runner", LambdaReducer(), randomizer,
                          8085, n, 2, 2)
    this_sim_params = run_sim(this_sim)
    with open(this_sim_params["savename"], "r") as f:
        run_data = json.load(f)
    expressions = list(run_data["0"].keys())
    return expressions

if __name__ == "__main__":
    args = aparser.parse_args()

    n = args.n_unique
    d = args.depth
    p_abs = args.abstraction_p
    p_app = args.application_p

    expressions = generate_random_expressions(n, d, p_app, p_abs)

    if args.filename:
        with open(args.filename,"w") as f:
            for e in expressions:
                f.write(e + " \n")
    else:
        for e in expressions:
            print(e)