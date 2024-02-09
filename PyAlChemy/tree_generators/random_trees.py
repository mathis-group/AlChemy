from btree_src.fontana_generator import FontanaGen
from btree_src.btree_generator import BtreeGen

import sys
sys.path.insert(0, '..')

from PyAlChemy

# Function to generate random binary trees from Devansh
def btree_lambda_gen(n, max_depth, ):
    """
    Generate random lambda expressions from Devansh's code
    """
    expressions = []
    for i in range(n):
        expressions.append(BtreeGen(max_depth).random_lambda())
    #return BTreeGenerator(n, max_depth).generate()

# Function to generate random binary trees from Walter 
def walter_random_gen(n, max_depth):
    """
    Generate random lambda expression from Walter's code
    """
    expressions = []
    for i in range(n):
        expressions.append(FontanaGen().random_lambda())
    return expressions
# Function to reduce a tree to normal form

# Function to use trees as input to AlChemy sim 

# Function to compute topological properties of trees

# Function to validate expressions
def validate_expressions(exprs_lis):

    return None

def expressions_to_file(exprs_list, fname):
    """
    Write expressions to file
    """
    with open(fname, "w") as f:
        for expr in exprs_list:
            f.write(expr + "\n")

if __name__ == "__main__":
    walter_exprs = walter_random_gen(100, 8)
    expressions_to_file(walter_exprs, "test_exprs.txt")