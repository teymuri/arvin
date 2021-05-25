
"""
BUBU Parser
"""

import re
from functools import reduce



s="""
list 1 2 3 * 10 10
 4 5
"""
s="""
list 1 2 3
    list 4 5 6
        list 3 2 1
    list 500 500 list 5 4 3 list 9 8
                list 10 11
"""

def pret(thing):
    """Prints and returns the thing."""
    print(thing)
    return thing

def group_case_clauses(clauses, g):
    if clauses:
        g.append(clauses[:2])
        return group_case_clauses(clauses[2:], g)
    return g


funcenv = {
    "*": lambda *args: reduce(lambda x, y: x*y, args),
    "+": lambda *args: sum(args),
    "=": lambda *args: args.count(args[0]) == len(args),
    "pret": pret,
    "list": lambda *args: list(args),
}
specialforms = ("case",)

def isfunc(x): return x in funcenv
def iskw(tok): 
    return tok.label in specialforms or tok.label in funcenv

class Token:
    def __init__(self, label, start, end, line):
        self.label = label
        self.start = start
        self.end = end
        self.line = line
        self.allocated = False
        self.value = None
    
    def __repr__(self):
        return f"<{self.label}> L{self.line} S{self.start}"


def tokisfun(t): return t.label in funcenv

def lines(src): return src.strip().splitlines()

# decimal numbers
DECPATT = r"[+-]?((\d+(\.\d*)?)|(\.\d+))"

def tokenize_source(src):
    toks = []
    for i, line in enumerate(lines(src)):
        for match in re.finditer(r"([*=+]|\w+|{})".format(
                DECPATT
            ), line):
            toks.append(Token(label=match.group(), start=match.start(), end=match.end(), line=i)
            )
    return toks




def is_token_in_block(tk, kw):
    """Is tk inside of the kw's block?"""
    return tk.start > kw.start and tk.line >= kw.line


def resolve_token(t):
    try:
        return int(t)
    except ValueError:
        try:
            return float(t)
        except ValueError:
            return funcenv[t]


    
def eval_(x):
    if isinstance(x, list):
        kw, *args = x
        if kw == "case":
            for pred, form in group_case_clauses(args, []):
                if eval_(pred): return eval_(form)
            return False
        elif isfunc(kw):
            return funcenv[kw](*[eval_(a) for a in args])
    else: return resolve_token(x)


    
"""
rightmost left-side function gets things,
if no rightmost left-side, then TOP rightmost leftside etc


"""


s="""
case 
  = 2 2
  + 2 2
  = 4 4 + 2 2
  * 10 pret 10
"""

toks = tokenize_source(s)
# print(toks)

class Block:
    def __init__(self, kw):
        self.kw = kw
        self.content = [self.kw]
    def append(self, t):
        self.content.append(t)

def ast(toks):
    blocks_tracker = []
    for t in toks:
        # if tokisfun(t):
        if iskw(t):
            B = Block(t)
            blocks_tracker.append(B)
        wrapping_blocks = [b for b in blocks_tracker if is_token_in_block(t, b.kw)]
        if wrapping_blocks:
            maxline = max(wrapping_blocks, key=lambda b: b.kw.line).kw.line
            bottommost_blocks = [b for b in wrapping_blocks if b.kw.line == maxline]
            rightmost_block = max(bottommost_blocks, key=lambda b: b.kw.start)
            rightmost_block.append(B if iskw(t) else t)
    try:
        return blocks_tracker[0]
    except IndexError: # If there was no kw, no blocks have been built
        pass


def listify(block, L):
    """Just for debugging!"""
    for x in block.content:
        if isinstance(x, Token):
            L.append(x.label)
        else:
            L.append(listify(x, []))
    return L
# print(ast(toks))
print(eval_(listify(ast(toks), [])))
