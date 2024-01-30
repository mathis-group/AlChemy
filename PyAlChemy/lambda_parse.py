""" This was shamelessly ripped from Devansh's lambda-btree repo."""
from __future__ import annotations
import re

from enum import Enum
import collections
import networkx as nx
import pickle
import os
import numpy as np

TREE_DICT_SAVENAME = "tree_dict.pickle"
TREE_DICT = pickle.load(open(TREE_DICT_SAVENAME, "rb")) if os.path.exists(TREE_DICT_SAVENAME) else dict()

class ASTNode:
    def __init__(self, left: ASTNode, right: ASTNode):
        self.left: ASTNode = left
        self.right: ASTNode = right
        self.value: str = None
        self.id: int = 0
        self.depth: int = 0

    def set_value(self, value: str) -> ASTNode:
        self.value = value
        return self

    def set_depth(self, depth: int) -> ASTNode:
        self.depth = depth
        return self

    def set_id(self, id: int) -> ASTNode:
        self.id = id
        return self

    def __str__(self) -> str:
        left = "" if self.left is None else str(self.left)
        right = "" if self.right is None else str(self.right)
        return f"({self.value}{left}{',' if left and right else ''}{right})"

    # display() and _display_aux() copied from
    # https://stackoverflow.com/a/54074933
    def display(self):
        lines, *_ = self._display_aux()
        for line in lines:
            print(line)

    def _display_aux(self, ob=lambda x: x.value):
        """Returns list of strings, width, height, and horizontal coordinate
        of the root."""
        # No child.
        if self.right is None and self.left is None:
            line = f"{ob(self)}"
            width = len(line)
            height = 1
            middle = width // 2
            return [line], width, height, middle

        # Only left child.
        if self.right is None:
            lines, n, p, x = self.left._display_aux()
            s = f"{ob(self)}"
            u = len(s)
            first_line = (x + 1) * ' ' + (n - x - 1) * '_' + s
            second_line = x * ' ' + '/' + (n - x - 1 + u) * ' '
            shifted_lines = [line + u * ' ' for line in lines]
            return [first_line, second_line] + shifted_lines, n + u, p + 2, n + u // 2

        # Only right child.
        if self.left is None:
            lines, n, p, x = self.right._display_aux()
            s = f"{ob(self)}"
            u = len(s)
            first_line = s + x * '_' + (n - x) * ' '
            second_line = (u + x) * ' ' + '\\' + (n - x - 1) * ' '
            shifted_lines = [u * ' ' + line for line in lines]
            return [first_line, second_line] + shifted_lines, n + u, p + 2, u // 2

        # Two children.
        left, n, p, x = self.left._display_aux()
        right, m, q, y = self.right._display_aux()
        s = f"{ob(self)}"
        u = len(s)
        first_line = (x + 1) * ' ' + (n - x - 1) * \
            '_' + s + y * '_' + (m - y) * ' '
        second_line = x * ' ' + '/' + \
            (n - x - 1 + u + y) * ' ' + '\\' + (m - y - 1) * ' '
        if p < q:
            left += [n * ' '] * (q - p)
        elif q < p:
            right += [m * ' '] * (p - q)
        zipped_lines = zip(left, right)
        lines = [first_line, second_line] + \
            [a + u * ' ' + b for a, b in zipped_lines]
        return lines, n + m + u, max(p, q) + 2, n + u // 2

    def edges_breadth(self):
        queue = collections.deque([self])
        while queue:
            parent = queue.popleft()
            for child in (parent.left, parent.right):
                if child:
                    yield ((parent.id, child.id))
                    queue.append(child)

    def vertices_breadth(self):
        queue = collections.deque([self])
        while queue:
            parent = queue.popleft()
            yield ((parent.id, parent.value))
            for child in (parent.left, parent.right):
                if child:
                    queue.append(child)

    def tolambda(self) -> str:
        match self.left, self.right:
            case (None, None):
                return f"{self.value}"
            case (None, _):
                body = self.right.tolambda()
                return f"\\{self.value}.({body})"
            case (_, None):
                body = self.left.tolambda()
                return f"\\{self.value}.({body})"
            case (_, _):
                l_lambda = self.left.tolambda()
                r_lambda = self.right.tolambda()
                return f"({l_lambda})({r_lambda})"

class AST:
    def __init__(self):
        pass

class TokenType(Enum):
    LBRACE = 0
    RBRACE = 1
    LAMBDA = 2
    DOT = 3
    VAR = 4
    EOF = 5


class Token:
    def __init__(self, t, lexeme=""):
        self.tok_type = t
        self.lexeme = lexeme

    def __str__(self) -> str:
        return f"({self.tok_type}" + (")" if self.lexeme == "" else f", {self.lexeme})")

    def __repr__(self) -> str:
        return str(self)


class LambdaLexer:
    def __init__(self, input: str):
        self.input = input
        self.tokens = []
        self.pos = 0

        n = 0

        while n < len(input):
            while input[n].isspace() and n < len(input):
                n += 1
            match input[n]:
                case "(":
                    self.tokens.append(Token(TokenType.LBRACE))
                    n += 1
                case ")":
                    self.tokens.append(Token(TokenType.RBRACE))
                    n += 1
                case "\\":
                    self.tokens.append(Token(TokenType.LAMBDA))
                    n += 1
                case ".":
                    self.tokens.append(Token(TokenType.DOT))
                    n += 1
                case _:
                    match = re.match(r"[a-z]+\d*", input[n:])
                    if match is not None:
                        self.tokens.append(Token(TokenType.VAR, match[0]))
                        n += len(match[0])
                    else:
                        print("lexer error")
                        return

    def peek(self, n: int) -> Token:
        peek_index = self.pos + n - 1
        if peek_index >= len(self.tokens):
            return Token(TokenType.EOF)
        return self.tokens[peek_index]

    def eat(self, tok: TokenType) -> Token:
        peek = self.peek(1)
        if peek.tok_type != tok:
            print("snytax eorrr")
        self.pos += 1
        return peek


class LambdaParser:
    def __init__(self, lex: LambdaLexer):
        self.lexer = lex

    # We use the following grammar:
    # abs := \ id . term
    # term := lambda | lambda term
    # lambda := abs | ( term ) | id
    # main := lambda EOF
        self.index = 0

    def parse(self) -> ASTNode:
        self.index = 0
        expr = self.parse_term()
        self.lexer.eat(TokenType.EOF)
        self.index = 0
        return expr

    def parse_abstraction(self) -> ASTNode:
        self.lexer.eat(TokenType.LAMBDA)
        lvar = self.lexer.eat(TokenType.VAR)
        self.lexer.eat(TokenType.DOT)
        ltree = self.parse_term()

        self.index += 1

        node = ASTNode(ltree, None).set_value(lvar.lexeme).set_id(self.index)
        return node

    def parse_term(self) -> ASTNode:
        ltree = self.parse_lambda()
        rtree = None
        if self.lexer.peek(1).tok_type in {TokenType.LAMBDA, TokenType.LBRACE, TokenType.VAR}:
            rtree = self.parse_term()
            self.index += 1
            return ASTNode(ltree, rtree).set_id(self.index)
        else:
            return ltree

    def parse_lambda(self) -> ASTNode:
        match self.lexer.peek(1).tok_type:
            case TokenType.LAMBDA:
                return self.parse_abstraction()
            case TokenType.LBRACE:
                self.lexer.eat(TokenType.LBRACE)
                term = self.parse_term()
                self.lexer.eat(TokenType.RBRACE)
                return term
            case TokenType.VAR:
                lvar = self.lexer.eat(TokenType.VAR)
                self.index += 1
                node = ASTNode(None, None).set_value(lvar.lexeme).set_id(self.index)
                return node
            case _:
                # "snytax rrrrrr" is a reference to Prof. Rida Bazzi
                print("snytax rrrrrr")

def lambda_to_nx(lambda_exp: str) -> nx.DiGraph:
    lexer = LambdaLexer(lambda_exp)
    parser = LambdaParser(lexer)
    ast = parser.parse()
    # ast.display()
    edges = [e for e in ast.edges_breadth()]

    ditree = nx.DiGraph()
    ditree.add_edges_from(edges)

    return ditree

def compute_network_props(ditree: nx.DiGraph()) -> dict:
    nodes = ditree.nodes()
    n_nodes = len(nodes)
    root = max(nodes)
    depths = nx.shortest_path_length(ditree, root)
    degrees = ditree.out_degree(nodes)
    c_factor = np.log2(n_nodes) / max(depths.values())

    props = {
        # "diameter": nx.diameter(nx.Graph(ditree)),
        "n_nodes": n_nodes,
        "c_factor": c_factor,
        "med_depth": np.median([v for _,v in depths.items() if _ != root]),
        "branching_factor": np.mean([d[1] for d in degrees if d[1] > 0]),
        "tree": ditree
    }

    return props

def lambda_to_net_props(lambda_expr):
    if TREE_DICT:
        ditree_props = TREE_DICT.get(lambda_expr, None)
        if ditree_props:
            return ditree_props
        else:
            ditree = lambda_to_nx(lambda_expr)
            ditree_props = compute_network_props(ditree)
            TREE_DICT[lambda_expr] = compute_network_props(ditree)
            pickle.dump(TREE_DICT, open(TREE_DICT_SAVENAME, "wb"))
            return ditree_props
    else:
        ditree = lambda_to_nx(lambda_expr)
        return compute_network_props(ditree)

# def main():
#     ex_string = r"\x1.\x2.\x3.\x4.\x5.\x6.\x7.\x8.(((((x4)\x9.\x10.\x11.\x12.\x13.\x14.((x10)x10)(x14)x9)\x15.\x16.\x17.\x18.\x19.\x20.((x16)x16)(x20)x15)\x21.\x22.\x23.\x24.\x25.\x26.((x22)x22)(x26)x21)(((x4)\x27.\x28.\x29.\x30.\x31.\x32.((x28)x28)(x32)x27)\x33.\x34.\x35.\x36.\x37.\x38.((x34)x34)(x38)x33)\x39.\x40.\x41.\x42.\x43.\x44.((x40)x40)(x44)x39)(x8)\x45.\x46.\x47.\x48.\x49.\x50.((x46)x46)(x50)x45"
#     lexer = LambdaLexer(ex_string)
#     parser = LambdaParser(lexer)
#     ast = parser.parse()
#     ast.display()
#     print(ast.tolambda())
#     print([e for e in ast.edges_breadth()])
#     print([v for v in ast.vertices_breadth()])


if __name__ == '__main__':
    # main()
    ex_string = r"\x1.\x2.\x3.\x4.\x5.\x6.\x7.\x8.(((((x4)\x9.\x10.\x11.\x12.\x13.\x14.((x10)x10)(x14)x9)\x15.\x16.\x17.\x18.\x19.\x20.((x16)x16)(x20)x15)\x21.\x22.\x23.\x24.\x25.\x26.((x22)x22)(x26)x21)(((x4)\x27.\x28.\x29.\x30.\x31.\x32.((x28)x28)(x32)x27)\x33.\x34.\x35.\x36.\x37.\x38.((x34)x34)(x38)x33)\x39.\x40.\x41.\x42.\x43.\x44.((x40)x40)(x44)x39)(x8)\x45.\x46.\x47.\x48.\x49.\x50.((x46)x46)(x50)x45"
    props = lambda_to_net_props(ex_string)
    print(props)