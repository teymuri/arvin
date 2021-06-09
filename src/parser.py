"""
BUBU
"""

import re
import sys
import operator as op
from functools import reduce
from copy import deepcopy




_verbose_tokenrepr = False


def pair(L):
    pairs = []
    s, e = 0, 2
    for _ in range(len(L)//2):
        pairs.append(L[s:e])
        s += 2
        e += 2
    return pairs

######### builtin functions
def sub(*args): return reduce(op.sub, args) if args else 0
def mul(*args): return reduce(op.mul, args) if args else 1
def floordiv(*args): return reduce(op.floordiv, args)
def truediv(*args): return reduce(op.truediv, args)
def add(*args): return sum(args)
def eq(*args): return args.count(args[0]) == len(args)
def pret(thing):
    """Prints and returns the thing."""
    print(thing)
    return thing

def list_(*args): return list(args)
# def pylist(*args): return "[{}]".format(", ".join(args))

def map_(fn, *args): return list(map(fn, *args))

def builtin_funcs():
    return {
        "*": mul, "+": add, "-": sub, "=": eq, 
        "//": floordiv, "/": truediv,
        "pret": pret,
        "list": list_, "map": map_
    }
def consts(): return {"true": True, "false": False}

FUNCOBJ_IDENTIFIER = "'"
class Env:
    def __init__(self, parenv=None):
        self.funcs = builtin_funcs()
        # self.funcs = builtin_funcs()
        self.vars = consts()
        self.parenv = parenv
        # if parenv:
        #     self.funcs.update(parenv.funcs)
        #     self.vars.update(parenv.vars)
        
    def isfunc(self, tok): return tok.label in self.funcs
    def resolvetok(self, tok):
        if tok.label.startswith(FUNCOBJ_IDENTIFIER):
            return self.funcs(tok.label[1:])
        else:
            try:
                return self.funcs[tok.label]
            except KeyError:
                return self.vars[tok.label]
    
    def resolve_token(self, tok):
        if tok.label.startswith("'"): # return the function object
            return self.funcs[tok.label[1:]]
        else:
            try:
                return self.funcs[tok.label]
            except KeyError:
                try:
                    return self.vars[tok.label]
                except KeyError:
                    print(">>>>>>", self.parenv.vars)
                    return self.parenv.resolve_token(tok)
    
    def isblockbuilder(self, tok):
        if tok.label in self.funcs:
            return True
        else:
            if self.parenv:
                return self.parenv.isblockbuilder(tok)
            else:
                return tok.label in SINGLE_NAMING_BLOCK_BUILDERS + \
                    NONNAMING_BLOCK_BUILDERS + MULTIPLE_VALUE_BINDERS + \
                    HIGHER_ORDER_FUNCTIONS + LEXICAL_BLOCK_BUILDERS
    
    def getenv(self, s):
        if s in self.funcs or s in self.vars or s in self.consts:
            return self
        elif self.parenv:
            return self.parenv.getenv(s)
        else:
            raise KeyError

toplevelenv = Env()

class Function:
    
    def __init__(self, params, body, enclosing_env):
        self.params = params
        self.body = body
        # self.enclosing_env = enclosing_env
        # self.env = Env(enclosing_env)
        # Create a fresh env at definition time!
        self.env = Env(parenv=enclosing_env)
    
    def __call__(self, *args):
        assert (len(self.params) == len(args)), f"function expected {len(self.params)} arguments, but got {len(args)}"
        self.env.vars.update(zip(self.params, args))
        for b in self.body[:-1]:
            eval_(b, self.env)
        return eval_(self.body[-1], self.env)






def group_case_clauses(clauses, g):
    if clauses:
        g.append(clauses[:2])
        return group_case_clauses(clauses[2:], g)
    return g

HIGHER_ORDER_FUNCTIONS = ("call", "map")

# def ishigherorder(tok): return tok.label in HIGHER_ORDER_FUNCTIONS
SINGLE_NAMING_BLOCK_BUILDERS = ("block","defun", )
NONNAMING_BLOCK_BUILDERS = ("case","call")
LEXICAL_BLOCK_BUILDERS = ("block", "defun", "name", "lambda", "defvar")
MULTIPLE_VALUE_BINDERS = ("defvar", "define")

def is_multiple_value_binder(tok): return tok.label in MULTIPLE_VALUE_BINDERS

def is_singlename_builder(tok): return tok.label in SINGLE_NAMING_BLOCK_BUILDERS

def is_lexenv_builder(tok): return tok.label in LEXICAL_BLOCK_BUILDERS


class Token:
    def __init__(self, label="_TOPLEVEL", start=-1, end=sys.maxsize, line=-1):
        self.label = label
        self.start = start # start position in the line
        self.end = end
        self.line = line # line number
    
    def __repr__(self):
        if _verbose_tokenrepr:
            return f"{self.label}.L{self.line}.S{self.start}"
        else:
            return f"{self.label}"






# decimal numbers
# DECPATT = r"[+-]?((\d+(\.\d*)?)|(\.\d+))"
STRPATT = re.compile(r'"[^"]*"')
# def lex(src):
    # """
    # """
    # src = src.strip()
    # str_matches = list(STRPATT.finditer(src))
    # spans = [m.span() for m in str_matches]
    # indices = [0] + [i for s in spans for i in s] + [len(src)]
    # tokens = []
    # for x in list(zip(indices[:-1], indices[1:])):
        # if x in spans: # str match?
            # tokens.append(src[x[0]:x[1]])
        # else:
            # tokens.extend(src[x[0]:x[1]].replace(LPAR, f" {LPAR} ").replace(RPAR, f" {RPAR} ").split())
    # return tokens

def lines(src): return src.strip().splitlines()

def lex(s):
    """Converts the string into a list of tokens."""
    toks = []
    for i, line in enumerate(lines(s)):
        # for match in re.finditer(r"([*=+-]|\w+|{})".format(DECPATT), line):
        for match in re.finditer(r"\S+", line):
            toks.append(Token(label=match.group(), start=match.start(), end=match.end(), line=i)
            )
    return toks




def token_isin_block(tk, bl):
    """Is tk inside of the kw's block?"""
    return tk.start > bl.kw.start and tk.line >= bl.kw.line



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


def ast(parsed_block, tree=[]):
    for x in parsed_block.cont:
        if isinstance(x, Token):
            tree.append(x)
        else: # Block?
            tree.append(ast(x, []))
    return tree



def bottom_rightmost_enclosing_block(enclosing_blocks):
    # Find the max line
    maxline = max(enclosing_blocks, key=lambda b: b.kw.line).kw.line
    # all bottommost blocks
    bottommost_blocks = [b for b in enclosing_blocks if b.kw.line == maxline]
    # return the rightmost of them
    return max(bottommost_blocks, key=lambda b: b.kw.start)

# global env has no parent env

toplevelblock = Block(kw=Token(), env=toplevelenv)

###########################
###########################
# The listify is passed to eval
def parse(toks):
    """Converts tokens of the source file to an AST of Tokens/Blocks"""
    nametok = None
    tlblock = deepcopy(toplevelblock)
    enclosingblock = tlblock
    blocktracker = [enclosingblock]
    # multivarbind = None
    
    for i, t in enumerate(toks):
        # make a block??
        if enclosingblock.env.isblockbuilder(t):
            if is_lexenv_builder(t):
                B = Block(t, Env(enclosingblock.env)) # give it a new env
                # if is_multiple_value_binder(t): multivarbind = B
            else:
                B = Block(t, enclosingblock.env)
            blocktracker.append(B)
        
        
        # Monitor next coming token
        if is_singlename_builder(t): # eg defun
            nametok = toks[i+1]
            
        enclosing_blocks = [b for b in blocktracker if token_isin_block(t, b)]
        enclosingblock = bottom_rightmost_enclosing_block(enclosing_blocks)
        enclosingblock.append(B if enclosingblock.env.isblockbuilder(t) else t)
        
        # if token_isin_block(t, multivarbind):
            # if enclosingblock.env.isblockbuilder(t):
                # print(multivarbind, B)
            # else:
                # print(multivarbind, t)
        
        # if enclosing_blocks: # If there are some enclosing blocks (Is this not always true?????????)
            # if enclosingblock.env.isblockbuilder(t):
                # enclosingblock.append(B)
            # else:
                # enclosingblock.append(t)
        # Adding to Env
        try:
            if t.label == nametok.label:
                # Create placeholders, so that the parser knows about these names while parsing
                # coming tokens.
                # The actual bindings to the objects happen later during evaluation.
                if toks[i-1].label == "define":
                    toplevelblock.env.funcs[t.label] = None
                elif toks[i-1].label == "funlet":
                    enclosingblock.env.funcs[t.label] = None
                elif toks[i-1].label == "block":
                    pass
                elif toks[i-1].label == "map": 
                    pass
                    
                nametok = None
        # If nametok is None
        except AttributeError: pass
    return tlblock
    # try:
        # return blocktracker[0]
    # except IndexError: # If there was no kw, no blocks have been built
        # pass




# def evalsrc(path):
    # with open(path, "r") as src:
        # eval_(parse(lex(src.read())), toplevelenv)



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
META = ("type", "lock", "tl")
def filtermeta(pairs):
    meta = {}
    nonmeta = []
    for tok, val in pairs:
        if tok.label in META:
            meta[tok.label] = val
        else:
            nonmeta.append((tok, val))
    return meta, nonmeta



def eval_(x, e):
    if isinstance(x, Block): # think of a Block as list!
        car, cdr = x.kw, x.cont[1:]
        # car, cdr = x.kw, x.cont
        if car.label == "_TOPLEVEL": # start processing the rest
            for i in cdr[:-1]:
                eval_(i, e)
            return eval_(cdr[-1], e)

        elif car.label == "name":
            meta, nonmeta = filtermeta(pair(cdr))
            # assignments go into the toplevel env
            if "tl" in meta and eval_(meta["tl"], toplevelenv):
                for vartok, val in nonmeta:
                    retval = eval_(val, toplevelenv)
                    toplevelenv.vars[vartok.label] = retval
                return retval
            else:
                for vartok, val in nonmeta:
                    retval = eval_(val, x.env)
                    x.env.vars[vartok.label] = retval
                return retval
        
        # elif car.label == "case":
            # for pred, form in pair(cdr):
                # if eval_(pred, e): return eval_(form, e)
            # return False
        elif car.label == "defvar": # toplevel var
            for var, val in pair(cdr):
                toplevelenv.vars.update([(var.label, eval_(val, e))])
            return var.label
        # Higher order functions
        elif car.label == "call": # call a function
            fn, *args = cdr
            # snapshot
            if isinstance(fn, Block): # Calling fresh anonymus function definition
                fnobj = eval_(fn, e)
                return fnobj(*[eval_(a, e) for a in args])
            else: # Token = saved function name
                fnobj = e.vars[fn.label]
                # return eval_(fnobj, fnobj.env)(*[eval_(a, fnobj.env) for a in args])
                return fnobj(*[eval_(a, fnobj.env) for a in args])
        elif car.label == "map":
            fn, *args = cdr
            return e.funcs["map"](eval_(fn, e), *[eval_(a, e) for a in args])
        elif car.label == "lambda": # create a function object
            # the first block is a block of params
            paramsblock, *body = cdr
            params = paramsblock.cont[1:]
            return Function([p.label for p in params], body, e)
        
        elif e.isfunc(car):
            # return e.funcs[car.label](*[eval_(b, e) for b in cdr])
            # print(car,cdr)
            return eval_(car, e)(*[eval_(b, e) for b in cdr])
        
        else:
            raise SyntaxError(f"{(x, x.cont)} not known")
    else: # x is a Token
        try:
            return int(x.label)
        except ValueError:
            try:
                return float(x.label)
            except ValueError:
                return e.resolve_token(x)
                # return e.resolvetok(x)
                
def interpretstr(s): return eval_(parse(lex(s)), toplevelenv)

s="""
name tl true x0 200 y0 * x0 x0
name tl true x1 10
      y1 + x1 10
  z1 + x0 y0 x1 y1
pret list x0 y0 x1 y1 z1
"""


# toks = lex(s)
# # print(STRPATT.findall(s))
# # print(parse(toks))
# # print(ast(parse(toks)))
# eval_(parse(toks), toplevelenv)
interpretstr(s)
