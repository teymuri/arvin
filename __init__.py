
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

funcenv = {
    "*": lambda *args: reduce(lambda x, y: x*y, args),
    "+": lambda *args: sum(args),
    "=": lambda *args: args.count(args[0]) == len(args),
    "pret": pret,
    "list": lambda *args: list(args),
    "case": None
}

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

def resolve_token(t):
    try:
        return int(t.label)
    except ValueError:
        try:
            return float(t.label)
        except ValueError:
            try:
                return funcenv[t.label]
            except KeyError:
                pass

def set_token_value_ip(t):
    try:
        t.value = int(t.label)
    except ValueError:
        try:
            t.value = float(t.label)
        except ValueError:
            try:
                t.value = funcenv[t.label]
            except KeyError:
                pass

    

def tokisfun(t): return t.label in funcenv

def lines(src): return src.strip().splitlines()

# decimal numbers
DECPATT = r"[+-]?((\d+(\.\d*)?)|(\.\d+))"

def tokenize_source(src):
    toks = []
    for i, line in enumerate(lines(src)):
        for match in re.finditer(r"([*+]|\w+|{})".format(DECPATT), line):
            toks.append(Token(label=match.group(), start=match.start(), end=match.end(), line=i)
            )
    return toks

def tokensatline(line, toks):
    return [t for t in toks if t.line == line]



def linekws(linetoks):
    """Returns a list of kw tokens at current line."""
    return [t for t in linetoks if tokisfun(t)]

def is_atomic_subordinate(tok, kw):
    """Is token inside of the kw's block, eg an arg etc
    """
    return (not tokisfun(tok)) and is_token_in_block(tok, kw) and (not tok.allocated)

def is_token_in_block(tk, kw):
    """Is tk inside of the kw's block?"""
    return tk.start > kw.start and tk.line >= kw.line

def is_composite_in_block(c1, c2):
    return is_token_in_block(c1[0], c2[0])


"""
defn meine-funktion
  p1 p2 p3
    p4 p5 p6
    p7 p8 p9
  block
    pret list p7 p8
    map
      fn
        p1
        * p1 1000
      list 1 2 3 4 5 6
"""

def evalexp(x):
    try: # a token obj?
        return x.value
    except AttributeError: # a list?
        try:
            fn, *args = x
            return evalexp(fn)(*[evalexp(a) for a in args])
        except KeyError:
            pass
"""
rightmost left-side function gets things,
if no rightmost left-side, then TOP rightmost leftside etc

"""
s="""
list 1 2 3
    list 4 5 6
        list 3 2 1
    44 list 500 500 list 5 4 3 list 9 8
                10    11
      1008
"""
s="""
4
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
        if tokisfun(t):
            B = Block(t)
            blocks_tracker.append(B)
        wrapping_blocks = [b for b in blocks_tracker if is_token_in_block(t, b.kw)]
        if wrapping_blocks:
            maxline = max(wrapping_blocks, key=lambda b: b.kw.line).kw.line
            bottommost_blocks = [b for b in wrapping_blocks if b.kw.line == maxline]
            rightmost_block = max(bottommost_blocks, key=lambda b: b.kw.start)
            rightmost_block.append(B if tokisfun(t) else t)
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

ast(toks)
# print(listify(ast(toks), []))
