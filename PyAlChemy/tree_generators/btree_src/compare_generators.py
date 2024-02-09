from btree_generator import BtreeGen
from fontana_generator import FontanaGen

from lambda_parse import LambdaLexer, LambdaParser
from lambda_ast import ASTNode

import matplotlib.pyplot as plt
import matplotlib as mpl


def average_degree(tree):
    # The average degree of a graph is related to its order and size by
    # d(G) = 2 * ||G|| / |G|
    # [Die17]

    ord = len([e for e in tree.edges_breadth()])
    size = len([v for (v, _) in tree.vertices_breadth()])

    if size == 0:
        return 0
    return 2 * ord / size


def r_app_abs(tree):
    n_abs = tree.n_abstractions()
    n_app = tree.n_applications()

    if n_abs == 0:
        return 0
    return n_app / n_abs


def plot(fn):
    print(fn.__name__)
    fig, axs = plt.subplots(2, 1, sharex=True, tight_layout=True)
    cmap = mpl.colormaps['viridis']

    for i in range(30, 2, -1):
        data = []
        gen = FontanaGen(max_depth=i)
        for j in range(10000):
            tree = gen.random_tree()
            adeg = fn(tree)
            data.append(adeg)
        axs[0].hist(data, alpha=0.5, bins=100, label=i, color=cmap(i / 30))

    for i in range(2, 50):
        data = []
        gen = BtreeGen(n_nodes=i)
        for j in range(10000):
            tree = gen.random_tree()
            adeg = fn(tree)
            data.append(adeg)
        axs[1].hist(data, alpha=0.5, bins=100, label=i, color=cmap(i / 50))

    plt.savefig(f'./img/{fn.__name__}.png')


def main():
    plot(average_degree)
    plot(r_app_abs)


if __name__ == "__main__":
    main()
