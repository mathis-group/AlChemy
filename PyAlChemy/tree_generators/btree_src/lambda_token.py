from enum import Enum


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
