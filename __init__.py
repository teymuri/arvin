
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
_verbose_tokenrepr = False

def pair(L):
    pairs = []
    s, e = 0, 2
    for _ in range(len(L)//2):
        pairs.append(L[s:e])
        s += 2
        e += 2
    return pairs


def group_case_clauses(clauses, g):
    if clauses:
        g.append(clauses[:2])
        return group_case_clauses(clauses[2:], g)
    return g

SINGLE_NAMING_BLOCK_BUILDERS = ("block","defun", )
NONNAMING_BLOCK_BUILDERS = ("case",)
LEXICAL_BLOCK_BUILDERS = ("block", "defun")
MULTIPLE_VALUE_BINDERS = ("defvar",)

def is_multiple_value_binder(tok): return tok.label in MULTIPLE_VALUE_BINDERS

def is_singlename_builder(tok): return tok.label in SINGLE_NAMING_BLOCK_BUILDERS

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
    def isfunc(self, tok): return tok.label in self.funcs
    def resolve_token(self, tok):
        try:
            return self.funcs[tok.label]
        except KeyError:
            try:
                return self.vars[tok.label]
            except KeyError:
                try:
                    return self.consts[tok.label]
                except KeyError:
                    return self.parenv.resolve_token(tok)
    
    def isblockbuilder(self, tok):
        if tok.label in self.funcs:
            return True
        else:
            if self.parenv:
                return self.parenv.isblockbuilder(tok)
            else:
                return tok.label in SINGLE_NAMING_BLOCK_BUILDERS + \
                    NONNAMING_BLOCK_BUILDERS + MULTIPLE_VALUE_BINDERS
    
    def getenv(self, s):
        if s in self.funcs or s in self.vars or s in self.consts:
            return self
        elif self.parenv:
            return self.parenv.getenv(s)
        else:
            raise KeyError
            
            
class Token:
    def __init__(self, label, start, end, line):
        self.label = label
        self.start = start
        self.end = end
        self.line = line
        # self.allocated = False
        # self.value = None
    
    def __repr__(self):
        if _verbose_tokenrepr:
            return f"{self.label}.L{self.line}.S{self.start}"
        else:
            return f"{self.label}"


class Func:
    def __init__(self, params, body, env):
        self.params = params
        self.body = body
        self.env = env
    def __call__(self, *args):
        return evaltoplevel(self.body, )
        
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
        self.cont = [self.kw]
        self.env = env
        self.nth = Block.counter
        Block.counter += 1
    
    def __repr__(self): return f"B{self.nth}"
    def append(self, t): self.cont.append(t)


def listify(block, L):
    for x in block.cont:
        if isinstance(x, Token):
            L.append(x)
        else:
            L.append(listify(x, []))
    return L
# varlet, funlet



def bottom_rightmost_enclosing_block(enclosing_blocks):
    # Find the max line
    maxline = max(enclosing_blocks, key=lambda b: b.kw.line).kw.line
    # all bottommost blocks
    bottommost_blocks = [b for b in enclosing_blocks if b.kw.line == maxline]
    # return the rightmost of them
    return max(bottommost_blocks, key=lambda b: b.kw.start)

# global env has no parent env
toplevel_env = Env()
toplevel_block = Block(kw=None, env=toplevel_env)
###########################
###########################
# The listify is passed to eval
def parse(toks):
    """Converts tokens to an AST"""
    # global toplevel_block
    
    nametok = None
    vartok_monitor = None
    enclosingblock = toplevel_block
    blocktracker = []
    monitor_multiple_value_binder = None
    
    for i, t in enumerate(toks):
        
        # make a block??
        if enclosingblock.env.isblockbuilder(t):
            if is_lexenv_builder(t):
                B = Block(t, Env(enclosingblock.env)) # give it a new env
            else:
                B = Block(t, enclosingblock.env)
        blocktracker.append(B)

        if is_singlename_builder(t): # eg defun
            nametok = toks[i+1]
            
        enclosing_blocks = [b for b in blocktracker if is_token_in_block(t, b)]
        if enclosing_blocks: # i!=0
            enclosingblock = bottom_rightmost_enclosing_block(enclosing_blocks)
            if enclosingblock.env.isblockbuilder(t):
                enclosingblock.append(B)
            else:
                enclosingblock.append(t)
            # enclosingblock.append(B if enclosingblock.env.isblockbuilder(t) else t)
        # Adding to Env
        try:
            if t.label == nametok.label:
                # Create placeholders, so that the parser knows about these names while parsing
                # coming tokens.
                # The actual bindings to the objects happen later during evaluation.
                if toks[i-1].label == "defun":
                    toplevel_block.env.funcs[t.label] = None
                elif toks[i-1].label == "funlet":
                    enclosingblock.env.funcs[t.label] = None
                elif toks[i-1].label == "block":
                    pass
                    
                nametok = None
        # If nametok is None
        except AttributeError: pass
    try:
        return blocktracker[0]
    except IndexError: # If there was no kw, no blocks have been built
        pass


def eval_(x, e):
    if isinstance(x, Block): # think of a Block as list!

        kwtok, *args = x.cont

        if x.kw == "case":
            for pred, form in group_case_clauses(x.cont, []):
                if evaltoplevel(pred, e): return evaltoplevel(form, e)
            return False
        
        elif kwtok.label == "defvar":
            assert all([isinstance(a, Block) for a in args[1:]])
            for bind in args: # bind is a Block
                _, vartok, val = bind.cont
                # Nicht im globalenv???
                x.env.vars[vartok.label] = eval_(val, bind.env)
            return vartok
        
        elif e.isfunc(x.kw):
            return e.funcs[x.kw.label](*[eval_(t, x.env) for t in x.cont[1:]])
        
        else:
            raise SyntaxError
    else: # x is a Token
        try:
            return int(x.label)
        except ValueError:
            try:
                return float(x.label)
            except ValueError:
                return e.resolve_token(x)




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
defvar  block x 8
    block v + x 2
              3 * 2 2
    block jurij * 100 v x
"""
toks = tokenize_source(s)
# print(listify(parse(toks),[]))
# print(listify(parse(toks),[]))
print(eval_(parse(toks), toplevel_env))
# print(listify(parse(toks),[]))
