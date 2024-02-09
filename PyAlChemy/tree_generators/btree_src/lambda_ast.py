from __future__ import annotations
import collections


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
                return f"\\{self.value}.{body}"
            case (_, None):
                body = self.left.tolambda()
                return f"\\{self.value}.{body}"
            case (_, _):
                l_lambda = self.left.tolambda()
                r_lambda = self.right.tolambda()
                return f"({l_lambda})({r_lambda})"

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

class AST:
    def __init__(self):
        pass
