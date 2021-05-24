
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
        return f"<{self.label}> L{self.line} S{self.start}"

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
    return (not tokisfun(tok)) and is_token_in_block(tok, kw) and (not tok.allocated)

def is_token_in_block(tk, kw):
    """Is tk inside of the kw's block?"""
    return tk.start > kw.start and tk.line >= kw.line

def is_composite_in_block(c1, c2):
    return is_token_in_block(c1[0], c2[0])

def nextindent(i, indents):
    for idx in sorted(set(indents)):
        if idx > i:
            return idx
    
def composite_exprs(toks):
    """
    Returns a list of composite expressions in the form
    [kw, p1, p2, p3, ...]
    """
    L = []
    D={}
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
                D[(kw.line, kw.start)] = x
    return L
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
def ast1(L):
    i=0
    while len(L) > 1:
        # print(">>", i,L[i])
        block = []
        for j, compexp in enumerate(L):
            if is_composite_in_block(compexp, L[i]):
                block.append(L.pop(j))
        if block:
            print(block)
            block=sorted(block, key=lambda c: c[0].line)
            block=sorted(block, key=lambda c: c[0].start)
            L[i].append(list(block))
        else: i += 1
    return L

def sort_composite_args(L):
    print([x for x in L if isinstance(x, list)])
    
def ast(atomic_lists):
    print(">>>>", atomic_lists)
    while len(atomic_lists) > 1:
        lst = atomic_lists.pop(0)
        # print(">",lst)
        for x in atomic_lists:
            if is_token_in_block(lst[0], x[0]):
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
"""
rightmost left-side function gets things,
if no rightmost left-side, then TOP rightmost leftside etc

"""
s="""
list 1 2 3
    list 4 5 6
        list 3 2 1
    44 list 500 500 list 5 4 3 list 9 8
                list 10 11
"""
# s="""
# list 1 2 list 3 4
# """
toks = tokenize_source(s)
# c=composite_exprs(tokenize_source(s))
# print(ast(c))
# sort_composite_args(ast(c))
# print(tokensatline(3, toks))
# print(toks)

class Block:
    def __init__(self, head):
        self.head = head
        self.tail = [self.head]
    def add(self, t):
        self.tail.append(t)
blocks=[]

for t in toks:
    if tokisfun(t):
        B=Block(t)
        blocks.append(B)
    x=[b for b in blocks if is_token_in_block(t, b.head)]
    if x:
        maxline=max(x, key=lambda b:b.head.line).head.line
        L=[b for b in x if b.head.line==maxline]
        maxstart=max(L,key=lambda b:b.head.start)
        if tokisfun(t):
            maxstart.add(B)
        else:
            maxstart.add(t)

def listify(block, L):
    for x in block.tail:
        if isinstance(x, Token):
            L.append(x.label)
        else:
            L.append(listify(x, []))
    return L
print(listify(blocks[0], []))
# print(blocks[0].tail)

# for b in blocks:
    # print(b.tail)

# print((ast(composite_exprs(tokenize_source(s)))))







# for x in composite_exprs(tokenize_source(s)):
    # print([a.label for a in x])
