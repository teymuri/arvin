
"""
SMT Programming Language
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

builtins_env = {
    "*": lambda *args: reduce(lambda x, y: x*y, args),
    "+": lambda *args: sum(args),
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
        return f"{self.label}"

def resolve_token(t):
    try:
        return int(t.label)
    except ValueError:
        try:
            return float(t.label)
        except ValueError:
            try:
                return builtins_env[t.label]
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
                t.value = builtins_env[t.label]
            except KeyError:
                pass

    

def tokisfun(t): return t.label in builtins_env

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
    return (not tokisfun(tok)) and isinblock(tok, kw) and (not tok.allocated)

def isinblock(tk, kw):
    return tk.start > kw.start and tk.line >= kw.line

def nextindent(i, indents):
    for idx in sorted(set(indents)):
        if idx > i:
            return idx
    
def allocate_atomics(toks):
    """
    [operator . . . . . . . .]
    """
    L = []
    maxline = max(toks, key=lambda t: t.line).line
    for i in range(maxline, -1, -1):
        kwtoks = linekws(tokensatline(i, toks))
        if kwtoks:
            for kw in sorted(kwtoks, key=lambda t: t.start, reverse=True):
                set_token_value_ip(kw)
                x=[kw]
                for a in [t for t in toks if is_atomic_subordinate(t, kw)]:
                    a.allocated = True
                    set_token_value_ip(a)
                    x.append(a)
                L.append(x)
    return L

def ast(atomic_lists):
    print(">", atomic_lists)
    while len(atomic_lists) > 1:
        lst = atomic_lists.pop(0)
        # print(">",lst)
        for x in atomic_lists:
            if isinblock(lst[0], x[0]):
            # if x[0].start < lst[0].start and x[0].line <= lst[0].line:
                x.append(lst)
                break
    return atomic_lists[0]


def evalexp(x):
    try: # a token obj?
        return x.value
    except AttributeError: # a list?
        try:
            fn, *args = x
            return evalexp(fn)(*[evalexp(a) for a in args])
        except KeyError:
            pass

toks = tokenize_source(s)
print((ast(allocate_atomics(tokenize_source(s)))))







# for x in allocate_atomics(tokenize_source(s)):
    # print([a.label for a in x])
