from __future__ import annotations

import numpy as np
import random
import collections
from typing import Any


class Tree:
    def __init__(self):
        self.left: Tree = None
        self.right: Tree = None
        self.value: Any = None
        self.id: int = 0

    def insert(self, value: int) -> Tree:
        if self.value is None:
            self.value = value
            return self
        if value <= self.value:
            if self.left is None:
                self.left = Tree().insert(value)
            else:
                self.left.insert(value)
        else:
            if self.right is None:
                self.right = Tree().insert(value)
            else:
                self.right.insert(value)
        return self

    def traverse(self):
        yield self
        if self.left is not None:
            yield from self._l.traverse()
        if self.right is not None:
            yield from self._r.traverse()

    def __str__(self) -> str:
        left = "" if self.left is None else str(self.left)
        right = "" if self.right is None else str(self.right)
        return f"({left}{',' if left and right else ''}{right})"

    # display() and _display_aux() copied from
    # https://stackoverflow.com/a/54074933
    def display(self):
        lines, *_ = self._display_aux()
        for line in lines:
            print(line)

    def _display_aux(self):
        """Returns list of strings, width, height, and horizontal coordinate
        of the root."""
        # No child.
        if self.right is None and self.left is None:
            line = f"{self.value}"
            width = len(line)
            height = 1
            middle = width // 2
            return [line], width, height, middle

        # Only left child.
        if self.right is None:
            lines, n, p, x = self.left._display_aux()
            s = f"{self.value}"
            u = len(s)
            first_line = (x + 1) * ' ' + (n - x - 1) * '_' + s
            second_line = x * ' ' + '/' + (n - x - 1 + u) * ' '
            shifted_lines = [line + u * ' ' for line in lines]
            return [first_line, second_line] + shifted_lines, n + u, p + 2, n + u // 2

        # Only right child.
        if self.left is None:
            lines, n, p, x = self.right._display_aux()
            s = f"{self.value}"
            u = len(s)
            first_line = s + x * '_' + (n - x) * ' '
            second_line = (u + x) * ' ' + '\\' + (n - x - 1) * ' '
            shifted_lines = [u * ' ' + line for line in lines]
            return [first_line, second_line] + shifted_lines, n + u, p + 2, u // 2

        # Two children.
        left, n, p, x = self.left._display_aux()
        right, m, q, y = self.right._display_aux()
        s = f"{self.value}"
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

    def n_applications(self):
        match self.left, self.right:
            case (None, None):
                return 0
            case (_, None):
                return self.left.n_applications() + 1
            case (None, _):
                return self.right.n_applications() + 1
            case (_, _):
                return self.right.n_applications() + self.left.n_applications()

    def n_abstractions(self):
        match self.left, self.right:
            case (None, None):
                return 0
            case (_, None):
                return self.left.n_abstractions()
            case (None, _):
                return self.right.n_abstractions()
            case (_, _):
                return self.right.n_abstractions() + self.left.n_abstractions() + 1


class BtreeGen:
    def __init__(self, max_free_vars=6, n_nodes=20):
        self.max_free_vars = max_free_vars
        self.n_nodes = n_nodes

    def set_max_free_vars(self, n: int) -> BtreeGen:
        self.max_free_vars = n
        return self

    def set_node_count(self, n: int) -> BtreeGen:
        self.n_nodes = n
        return self

    def tolambda(self, tree: Tree, depth: int) -> str:
        match tree.left, tree.right:
            case (None, None):
                max_label = self.max_free_vars if depth == 0 else depth
                return f"{chr(97 + random.randint(0, max_label))}"
            case (None, _):
                body = self.tolambda(tree.right, depth + 1)
                return f"\\{chr(97 + depth + 1)}.{body}"
            case (_, None):
                body = self.tolambda(tree.left, depth + 1)
                return f"\\{chr(97 + depth + 1)}.{body}"
            case (_, _):
                l_lambda = self.tolambda(tree.left, depth)
                r_lambda = self.tolambda(tree.right, depth)
                return f"({l_lambda})({r_lambda})"

    def random_lambda(self):
        permutation = np.random.permutation(self.n_nodes)
        tree = Tree()
        for i in permutation:
            tree.insert(i)
        return self.tolambda(tree, 0)

    def random_tree(self):
        permutation = np.random.permutation(self.n_nodes)
        tree = Tree()
        for i in permutation:
            tree.insert(i)
        return tree


def main():
    gen = BtreeGen()
    for i in range(1000):
        print(gen.random_lambda())


if __name__ == '__main__':
    main()
