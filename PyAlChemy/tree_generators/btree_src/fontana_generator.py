from __future__ import annotations

from .lambda_ast import ASTNode

import random


class Urn:
    # RNG used in Fontana's original generator. This is currently unused.
    A = 48271
    M = 2147483647
    Q = M // A
    R = M % A
    temp = 1 / M

    def __init__(self):
        self.seed = 123456789

    def urn(self):
        hi = self.seed // self.Q
        lo = self.seed % self.Q
        test = self.A * lo - self.R * hi
        self.seed = self.test if test > 0 else test + self.M
        return self.seed * self.temp


class FontanaGen:
    def __init__(self,
                 max_depth=10,
                 max_nvars=6,
                 application_prange=(0.3, 0.5),
                 abstraction_prange=(0.5, 0.3)):
        #  self.variables = list("abcdefghijklmnopqrstuvwzyz")
        self.variables = [f"x{i}" for i in range(26)]
        self.max_depth = max_depth
        self.max_nvars = max_nvars
        self.application_prange = application_prange
        self.abstraction_prange = abstraction_prange
        self.application_incr = self.get_application_incr()
        self.abstraction_incr = self.get_abstraction_incr()

    def set_application_prange(self, start: float, end: float) -> FontanaGen:
        self.application_prange = (start, end)
        self.application_incr = self.get_application_incr()
        self.abstraction_incr = self.get_abstraction_incr()
        return self

    def set_abstraction_prange(self, start: float, end: float) -> FontanaGen:
        self.abstraction_prange = (start, end)
        self.application_incr = self.get_application_incr()
        self.abstraction_incr = self.get_abstraction_incr()
        return self

    def set_max_expr_depth(self, depth: int) -> FontanaGen:
        self.max_depth = depth
        self.application_incr = self.get_application_incr()
        self.abstraction_incr = self.get_abstraction_incr()
        return self

    def set_max_nvars(self, nvars: int) -> FontanaGen:
        assert nvars <= 26
        self.nvars = nvars
        return self

    def get_application_incr(self) -> float:
        (start, end) = self.application_prange
        return (end - start) / (self.max_depth - 1)

    def get_abstraction_incr(self) -> float:
        (start, end) = self.abstraction_prange
        return (end - start) / (self.max_depth - 1)

    def random_lambda_helper(self,
                             depth: int,
                             p_abstraction: float,
                             p_application: float) -> ASTNode:
        if depth > self.max_depth:
            var = self.variables[random.randint(0, self.max_nvars)]
            return ASTNode(None, None).set_value(var)

        coin = random.random()

        n_abst = p_abstraction + self.application_incr
        n_appl = p_application + self.abstraction_incr

        if coin <= p_abstraction:
            left_child = self.random_lambda_helper(depth + 1, n_abst, n_appl)
            var = self.variables[random.randint(0, self.max_nvars)]
            return ASTNode(left_child, None).set_value(var)

        elif coin <= p_abstraction + p_application:
            left_child = self.random_lambda_helper(depth + 1, n_abst, n_appl)
            right_child = self.random_lambda_helper(depth + 1, n_abst, n_appl)
            return ASTNode(left_child, right_child)

        else:
            var = self.variables[random.randint(0, self.max_nvars)]
            return ASTNode(None, None).set_value(var)

    def random_lambda(self):
        init_p_abst = self.abstraction_prange[0]
        init_p_appl = self.application_prange[0]
        ast = self.random_lambda_helper(0, init_p_abst, init_p_appl)
        return ast.tolambda()

    def random_tree(self):
        init_p_abst = self.abstraction_prange[0]
        init_p_appl = self.application_prange[0]
        ast = self.random_lambda_helper(0, init_p_abst, init_p_appl)
        return ast


def main():
    gen = FontanaGen()
    for i in range(1000):
        print(gen.random_lambda())


if __name__ == "__main__":
    main()
