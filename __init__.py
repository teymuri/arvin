
"""
BUBU Parser
"""

import re
import operator as op
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



def group_case_clauses(clauses, g):
    if clauses:
        g.append(clauses[:2])
        return group_case_clauses(clauses[2:], g)
    return g

NAMING_BLOCK_BUILDERS = ("defun", "defvar", "funlet")
NONNAMING_BLOCK_BUILDERS = ("case",)
LEXICAL_BLOCK_BUILDERS = ("varlet", "funlet", "defun")
def isnaming(tok): return tok.label in NAMING_BLOCK_BUILDERS
def is_lexenv_builder(tok): return tok.label in LEXICAL_BLOCK_BUILDERS

######### builtin functions
def sub(*args): return reduce(op.sub, args)
def mul(*args): return reduce(op.mul, args)
def add(*args): return sum(args)
def eq(*args): return args.count(args[0]) == len(args)
def pret(thing):
    """Prints and returns the thing."""
    print(thing)
    return thing
def list_(*args): list(args)

def _builtin_funcs():
    return {
        "*": mul, "+": add, "-": sub, "=": eq, "pret": pret,
        "list": list_,
    }

class Env:
    def __init__(self, parenv=None):
        self.funcs = _builtin_funcs()
        self.vars = {}
        self.consts = {}
        self.parenv = parenv
    
    def isblockbuilder(self, tok):
        if tok.label in self.funcs:
            return True
        else:
            if self.parenv:
                return self.parenv.isblockbuilder(tok)
            else:
                return tok.label in NAMING_BLOCK_BUILDERS + NONNAMING_BLOCK_BUILDERS
    
    def getenv(self, s):
        if s in self.funcs or s in self.vars or s in self.consts:
            return self
        elif self.parenv:
            return self.parenv.getenv(s)
        else:
            raise KeyError
    
    # def isblockbuilder(self, s):
        # return s in tuple(self.funcs.keys()) + NAMING_BLOCK_BUILDERS + NONNAMING_BLOCK_BUILDERS


# specialforms = ("case",)

# def isfun(s, env): return s in env["fun"]

# def iskw(tok): 
    # return tok.label in specialforms or isfun(tok.label)

class Token:
    def __init__(self, label, start, end, line):
        self.label = label
        self.start = start
        self.end = end
        self.line = line
        self.allocated = False
        self.value = None
    
    def __repr__(self):
        return f"{self.label}.L{self.line}.S{self.start}"
        # return f"{self.label}"


# def tokisfun(t): return t.label in funcenv

def lines(src): return src.strip().splitlines()

# decimal numbers
DECPATT = r"[+-]?((\d+(\.\d*)?)|(\.\d+))"

def tokenize_source(src):
    toks = []
    for i, line in enumerate(lines(src)):
        for match in re.finditer(r"([*=+-]|\w+|{})".format(
                DECPATT
            ), line):
            toks.append(Token(label=match.group(), start=match.start(), end=match.end(), line=i)
            )
    return toks




def is_token_in_block(tk, bl):
    """Is tk inside of the kw's block?"""
    return tk.start > bl.kw.start and tk.line >= bl.kw.line


def evaltoplevel(x, e):
    if isinstance(x, list):
        kw, *args = x
        # if e.isspecialform(kw):
        if kw == "case":
            for pred, form in group_case_clauses(args, []):
                if evaltoplevel(pred, e): return evaltoplevel(form, e)
            return False
        # elif isfun(kw, e):
        elif e.isfunction(kw):
            return e.getfunction(kw)(*[evaltoplevel(a, e) for a in args])
        else:
            raise SyntaxError
    else:
        try:
            return int(x)
        except ValueError:
            try:
                return float(x)
            except ValueError:
                try: # resolving in env
                    return e.variables[x]
                except KeyError:
                    return e.constants[x]


    
"""
rightmost left-side function gets things,
if no rightmost left-side, then TOP rightmost leftside etc


"""


s="""
case 
  = 2 2
  + 2 2
  = 4 4 + 2 2
  defun FOO
    x y
     r t
    case = + x y
           * r t
         FOO r t x y
  FOO 1 2 3 4



"""

toks = tokenize_source(s)
# print(toks)

class Block:
    counter = 0
    def __init__(self, kw, env):
        self.kw = kw
        self.content = [self.kw]
        self.env = env
        self.nth = Block.counter
        Block.counter += 1
    
    def __repr__(self): return f"B{self.nth}"
    def append(self, t):
        self.content.append(t)


def ast(block, L):
    for x in block.content:
        if isinstance(x, Token):
            L.append(x)
        else:
            L.append(ast(x, []))
    return L
# varlet, funlet

# def currenv(envslist): return envslist[-1]



# def parse(toks):
    # """Converts tokens to an AST"""
    # envs = [Env(parenv=toplevel_env)]
    # blocktracker = []
    # nametok = None
    
    # for i, t in enumerate(toks):
        # if env.isblockbuilder(t.label):
            # B = Block(t)
            # blocktracker.append(B)
        
        # if env.isnaming(t.label): # next token MUST BE the name of this thing!
            # nametok = toks[i+1]
        
        # enclosing_blocks = [b for b in blocktracker if is_token_in_block(t, b)]
        # if enclosing_blocks: # i!=0
            # maxline = max(enclosing_blocks, key=lambda b: b.kw.line).kw.line
            # bottommost_blocks = [b for b in enclosing_blocks if b.kw.line == maxline]
            # rightmost_block = max(bottommost_blocks, key=lambda b: b.kw.start)
            # rightmost_block.append(B if env.isblockbuilder(t.label) else t)
        
        # try:
            # if t.label == nametok.label:
                # if toks[i-1].label == "defun":
                    # env.block_builders[t.label] = None # actual binding happens later while evaling
                # nametok = None
        # except AttributeError: pass
    
    # try:
        # return ast(blocktracker[0], [])
    # except IndexError: # If there was no kw, no blocks have been built
        # pass

def bottom_rightmost_enclosing_block(enclosing_blocks):
    # Find the max line
    maxline = max(enclosing_blocks, key=lambda b: b.kw.line).kw.line
    # all bottommost blocks
    bottommost_blocks = [b for b in enclosing_blocks if b.kw.line == maxline]
    # return the rightmost of them
    return max(bottommost_blocks, key=lambda b: b.kw.start)

# global env has no parent env
toplevel_block = Block(kw=None, env=Env())
###########################
###########################
#if not blockbuilder , also cant be naming!
def parse_toplevel(toks):
    """Converts tokens to an AST"""
    # global toplevel_block
    nametok = None
    currblock = toplevel_block
    blocktracker = []
    # print(currblock)
    for i, t in enumerate(toks):
        # print(t, currblock)
        if currblock.env.isblockbuilder(t):
            if is_lexenv_builder(t):
                # blocktracker.append(Block(t, Env(currblock.env)))
                B = Block(t, Env(currblock.env))
            else:
                # blocktracker.append(Block(t, currblock.env))
                B = Block(t, currblock.env)
        blocktracker.append(B)
        # If this token is a naming kw, the next token MUST BE the name of it!
        if isnaming(t):
            nametok = toks[i+1]
        enclosing_blocks = [b for b in blocktracker if is_token_in_block(t, b)]
        if enclosing_blocks: # i!=0
            # maxline = max(enclosing_blocks, key=lambda b: b.kw.line).kw.line
            # bottommost_blocks = [b for b in enclosing_blocks if b.kw.line == maxline]
            # rightmost_block = max(bottommost_blocks, key=lambda b: b.kw.start)
            # currblock = rightmost_block
            # # print("<<", t, rightmost_block)
            # rightmost_block.append(B if currblock.env.isblockbuilder(t.label) else t)
            currblock = bottom_rightmost_enclosing_block(enclosing_blocks)
            currblock.append(B if currblock.env.isblockbuilder(t) else t)
        try:
            if t.label == nametok.label:
                # Create placeholders, so that the parser knows about these names while parsing
                # coming tokens.
                # The actual bindings to the objects happen later during evaluation.
                if toks[i-1].label == "defun":
                    toplevel_block.env.funcs[t.label] = None
                elif toks[i-1].label == "funlet":
                    currblock.env.funcs[t.label] = None
                nametok = None
        # If nametok is None
        except AttributeError: pass
    try:
        return ast(blocktracker[0], [])
    except IndexError: # If there was no kw, no blocks have been built
        pass




# def repl(prompt='lis.py> '):
    # "A prompt-read-eval-print loop."
    # while True:
        # val = eval(parse(raw_input(prompt)))
        # if val is not None: 
            # print(schemestr(val))

# def schemestr(exp):
    # "Convert a Python object back into a Scheme-readable string."
    # if isinstance(exp, List):
        # return '(' + ' '.join(map(schemestr, exp)) + ')' 
    # else:
        # return str(exp)
s="""
defun fact
 n
 case = n 1
    1
  true
    * n
     fact
      - n
       1
"""
s="""
defun F0 :x y z
  a b c
  :defun F1
    F0
"""
toks = tokenize_source(s)
print(parse_toplevel(toks))
