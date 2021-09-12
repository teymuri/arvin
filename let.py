

"""
This is a first prototype for the Let programming language.
It has been archived on Sat 11 Sep 2021 08:22:13 AM CEST.

"""

print("=======================")
print("((((((((( LET )))))))))")
print("=======================")



import re
import sys
import operator as op
from functools import reduce
from copy import deepcopy




_verbose_tokenrepr = False


# def pair(L):
#     assert len(L)%2 == 0, f"""List can't be arranged in pairs:
#     {L}"""
#     pairs = []
#     s, e = 0, 2
#     for _ in range(len(L)//2):
#         pairs.append(L[s:e])
#         s += 2
#         e += 2
#     return pairs

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


def map_(fn, *args): return list(map(fn, *args))

def builtin_funcs():
    return {
        "*": mul, "+": add, "-": sub, "=": eq, 
        "//": floordiv, "/": truediv,
        "pret": pret,
        "list": list_, "map": map_
    }
def consts(): return {"true": True, "false": False, "ja": True, "ne": False,
                      "null": []}

FUNCOBJ_IDENT = "'"
class Env:
    counter = 0
    def __init__(self, parenv=None, id_=None):
        # self.funcs = builtin_funcs()
        self.builtins = builtin_funcs()
        self.user_defined = {}   # seperate user defined from builtins
        self.vars = consts()
        self.parenv = parenv
        self.id = id_ if id_ else self.id_()

    def __repr__(self):
        return f"(ENV {self.id})"
    
    def id_(self):
        x = Env.counter
        Env.counter += 1
        return x
    
    # def isfunc(self, tok): return tok.string in self.funcs
    def isbuiltin(self, tok): return tok.string in self.builtins
    def isuserdefined(self, tok): return tok.string in self.user_defined
    def resolve_token(self, tok):
        if tok.string.startswith(FUNCOBJ_IDENT): # return the function object
            try:                                 # builtin?
                return self.builtins[tok.string[1:]]
            except KeyError:    # # maybe userdefined?
                return self.user_defined[tok.string[1:]]
        else:
            try:
                return self.builtins[tok.string]
            except KeyError:
                try:
                    return self.user_defined[tok.string]
                except KeyError:
                    try:
                        return self.vars[tok.string]
                    except KeyError:
                        if self == tlenv: # if already at the top, token couldn't be resolved!
                            raise NameError(f"name {tok.string} is not defined")
                        else:
                            return self.parenv.resolve_token(tok)
    
    def isblockbuilder(self, tok):
        if (tok.string in self.builtins) or (tok.string in self.user_defined): # a function?
            return True
        else:
            if self.parenv:
                return self.parenv.isblockbuilder(tok)
            else:
                return tok.string in HIGHER_ORDER_FUNCTIONS + LEXICAL_BLOCK_BUILDERS
    
    def getenv(self, s):
        if s in self.funcs or s in self.vars or s in self.consts:
            return self
        elif self.parenv:
            return self.parenv.getenv(s)
        else:
            raise KeyError

# The Toplevel Environment
tlenv = Env(id_="TL")

class Function:
    """user-defined functions"""
    # params is a block of one or more name blocks
    def __init__(self, params, actions, enclosing_env):
        # Create a fresh env at definition time!
        self.env = Env(parenv=enclosing_env)
        self.params = []
        self.count_obligargs = 0
        self.init_params_ip(params)
        self.actions = actions
    
    def init_params_ip(self, params): # init params at definition time
        for b in params:              # multiple :names possible
            _, name = b.body
            _, *params = name.body
            for p in params:            # inside each :name
                nmtok, *defarg = p.body # name token and default argument
                self.params.append(nmtok)
                if defarg:      # If default arguments given at definition time
                    self.env.vars[nmtok.string] = eval_(defarg[0], self.env)
                else:
                    self.env.vars[nmtok.string] = None
                    self.count_obligargs += 1
    
    def __call__(self, arglst):
        assert len(arglst) >= self.count_obligargs, \
            f"""passed {len(arglst)} args to a lambda with min arity {self.count_obligargs}:
{[p.string for p in self.params]}"""
        callenv = Env(self.env) # The run-time environment
        for i, a in enumerate(arglst):
            if is_param_naming_arg(a):
                _, name, *vals = a.body
                for v in vals[:-1]:
                    eval_(v, self.env)
                callenv.vars[name.string] = eval_(vals[-1], self.env)
            else:               # schau nach der Reihenfolge der Parameter, jetzt sind wir
                callenv.vars[self.params[i].string] = eval_(a, self.env)                    
        Ret = None
        for action in self.actions:
            Ret = eval_(action, callenv)
        return Ret




HIGHER_ORDER_FUNCTIONS = ("call", "map")

# def ishigherorder(tok): return tok.string in HIGHER_ORDER_FUNCTIONS
# SINGLE_NAMING_BLOCK_BUILDERS = ("block","defun", )
# NONNAMING_BLOCK_BUILDERS = ("case","call")
LEXICAL_BLOCK_BUILDERS = ("block", "name", "lambda")


def is_multiple_value_binder(tok): return tok.string in MULTIPLE_VALUE_BINDERS

def is_singlename_builder(tok): return tok.string in SINGLE_NAMING_BLOCK_BUILDERS

def is_lexenv_builder(tok): return tok.string in LEXICAL_BLOCK_BUILDERS

TL_STR = "TL"
class Token:
    id_ = 0
    def __init__(self, string=TL_STR, start=-1, end=sys.maxsize, line=-1):
        self.string = string
        self.start = start # start position in the line
        self.end = end
        self.line = line # line number
        self.id = Token.id_
        self.commidx = None # Comment index, will be set only if token is a comment start/end
        Token.id_ += 1
    def __repr__(self):
        if _verbose_tokenrepr:
            return f"{self.string}.L{self.line}.S{self.start}"
        else:
            return f"<Token{self.id} {self.string}>"


def lines(src): return src.strip().splitlines()


LCOMM = "("
RCOMM = ")"
def set_commidx_ip(tokens):
    """
    Mark openings and closings of lists, eg [[]] ->
    [open0, open1, close1, close0]
    """
    i = 0
    for tok in tokens:
        if tok.string == LCOMM:
            tok.commidx = i
            i += 1
        elif tok.string == RCOMM:
            i -= 1
            tok.commidx = i
        else:
            pass

PARSER_BLOCKCUT_IDENT = ";" # in tokpatt damit!??
FUNC_PARAM_IDENT = ":"      # Das hier ist kein kw!!
# Handle (&) as single tokens, anything else as one token
# All valid tokens
# TOKPATT = r"(\(|\)|;|:|[\w\d.+\-*=]+)"
TOKENS_PATT = r"({}|{}|{}|{}|[\w\d.+\-*={}]+)".format(
    "\(", "\)", PARSER_BLOCKCUT_IDENT, FUNC_PARAM_IDENT,
    FUNCOBJ_IDENT
)

# Lexer
def tokenize_str(s):
    """Converts the string into a list of tokens."""
    toks = []
    for i, line in enumerate(lines(s)):
        # for match in re.finditer(r"\S+", line):
        for match in re.finditer(TOKENS_PATT, line):
            toks.append(Token(string=match.group(), start=match.start(), end=match.end(), line=i)
            )
    return toks


class Block:
    counter = 0
    def __init__(self, head, env, enblock=None, id_=None):
        self.head = head
        self.body = [self.head]
        self.env = env
        self.id = id_ if id_ else self.id()
        self.enblock = enblock     # The enclosing block of this block

    def id(self):
        i = Block.counter
        Block.counter += 1
        return i
    def __repr__(self): return f"<Block{self.id} HEAD:{self.head}>"
    def append(self, t): self.body.append(t)



def token_isin_block(tk, bl):
    """Is tk inside of the head's block?"""
    return tk.start > bl.head.start and tk.line >= bl.head.line

def bottommost_blocks(enclosing_blocks):
    # Find the bottom-most line
    maxline = max(enclosing_blocks, key=lambda b: b.head.line).head.line
    # filter all bottommost blocks
    return [b for b in enclosing_blocks if b.head.line == maxline]

def rightmost_block(bottommosts):
    # get the rightmost one of them
    return max(bottommosts, key=lambda b: b.head.start)

def enclosing_block(tok, blocks): # blocks is a list
    """Returns token's enclosing block."""
    bs = [b for b in blocks if token_isin_block(tok, b)]
    # print(blocks)
    # print("?", tok, bs)
    # print("000000000000")
    return rightmost_block(bottommost_blocks(bs))

# The Toplevel Block
tlblock = Block(head=Token(), env=tlenv, id_=TL_STR)

###########################
###########################
# Return an AST
def parse(toks):
    """Converts tokens of the source file into an AST of Tokens/Blocks"""
    blocktracker = [tlblock]
    for i, t in enumerate(toks):
        if t.string == PARSER_BLOCKCUT_IDENT: #close the block of THE PREVIOUS token (not of the blockcut token itself!!!)
            enblock = enclosing_block(toks[i-1], blocktracker)            
            for j, b in enumerate(blocktracker):
                if b.id == enblock.id:
                    # by removing the block in question from the blocktracker list,
                    # we prevent it from becoming an enclosing block for anything else.
                    del blocktracker[j]
        ########### Making of a new BLOCK #################
        else: # create a new block if ...
            enblock = enclosing_block(t, blocktracker)

            # if enclosing block is NAME, and the next token is a lambda
            # this token is going to be the name of a funtion.
            if enblock.head.string=="name" and enblock.enblock.head.string != FUNC_PARAM_IDENT and toks[i+1].string =="lambda":
                # A placeholder for the function object to come while
                # evaling This placeholder is there only so that the
                # parser handles the current token as a block builder
                # (it's a function after all!)
                if isname_toplevel(enblock):
                    tlblock.env.user_defined[t.string] = None
                    enblock.env.user_defined[t.string] = None
                else:
                    enblock.env.user_defined[t.string] = None
            
            if enblock.env.isblockbuilder(t) or t.string == FUNC_PARAM_IDENT:
                if is_lexenv_builder(t):
                    B = Block(t, Env(parenv=enblock.env), enblock=enblock) # give it a new env
                else:
                    B = Block(t, enblock.env, enblock=enblock)
                blocktracker.append(B)
            elif enblock.head.string == "name": # Is token an arg to name?
                # This is only a pseudo-block, the idea is to pack
                # pairs of args to name into blocks instead of pairing them
                # into lists.
                B = Block(t, env=enblock.env, enblock=enblock)
                blocktracker.append(B)
            ################################  and enblock.head.string == "lambda"
            if enblock.env.isblockbuilder(t) or t.string == FUNC_PARAM_IDENT:
                enblock.append(B)
            elif enblock.head.string == "name": # args to name
                enblock.append(B)
            else:               # Just a simple lonely token!
                enblock.append(t)
    return tlblock

def isname_toplevel(nameblock):
    for b in nameblock.body[1:]:
        if b.head.string == "tl": # Wrong!!!!!!!!!!!!
            return True
    return False

META = ("type", "lock", "tl")

def filtermeta(pairs):
    meta = {}
    nonmeta = []
    for tok, val in pairs:
        if tok.string in META:
            meta[tok.string] = val
        else:
            nonmeta.append((tok, val))
    return meta, nonmeta

def filtermeta(nameblocks):
    meta = {}
    nonmeta = []
    for b in nameblocks:
        if b.head.string in META:
            meta[b.head.string] = b.body[1:]
        else:
            nonmeta.append(b)
    return meta, nonmeta



def tok_is_nondata(tok):
    """Returns true if token is a true identifier!"""
    try:
        int(tok.string)
        return False
    except ValueError:
        try:
            float(tok.string)
            return False
        except ValueError:
            return True
    
class Void:
    def __init__(self):
        self.value = []

def is_param_naming_arg(x):
    return isinstance(x, Block) and x.head.string == FUNC_PARAM_IDENT

IMPLICIT_LIST_IDENTIFIER = "+"

def eval_(x, e, access_from_parenv=None):
    if isinstance(x, Block): # think of a Block as list!
        car, cdr = x.head, x.body[1:]
        # car, cdr = x.head, x.body
        if car.string == TL_STR: # start processing the rest
            for i in cdr[:-1]:
                eval_(i, e)
            return eval_(cdr[-1], e)

        elif car.string == "name":
            # meta, nonmeta = filtermeta(pair(cdr))
            meta, nonmeta = filtermeta(cdr)
            # All x are evaled in THEIR OWN envs!
            # We only differentiate btwn where names should be written (tl or lexical)!!
            if "tl" in meta and all([eval_(x, e) for x in meta["tl"]]):
                write_env = tlenv
            else:
                write_env = x.env # name env
            for b in nonmeta:                
                if not tok_is_nondata(b.head):
                    raise NameError(f"name {b.head} is number")                
                if b.head.string.startswith(IMPLICIT_LIST_IDENTIFIER):
                    # implicit_list
                    retval = []
                    for val in b.body[1:]:
                        retval.append(eval_(val, x.env))
                    write_env.vars[b.head.string[1:]] = retval                
                else:
                    vals = b.body[1:]
                    if vals: # There are any values to be assigned to names?
                        for val in vals:
                            retval = eval_(val, x.env)
                    else: # No values => Null
                        retval = Void()
                    # write in funcs or in variables?
                    if isinstance(retval, Function):
                        # Ist schnell getan , ist wahrscheinlich scheiße!
                        # Für toplevel deklarierte funktionen, funktion ist auch
                        # im lexical trotzdem drin!!!!??????????
                        if "tl" in meta and all([eval_(x, e) for x in meta["tl"]]):
                            tlenv.user_defined[b.head.string] = retval
                            x.env.user_defined[b.head.string] = retval
                        else:
                            x.env.user_defined[b.head.string] = retval
                    else:
                        write_env.vars[b.head.string] = retval
            if access_from_parenv:
                access_from_parenv.vars.update(write_env.vars)
            return retval

        # Higher order functions
        elif car.string == "call": # call a function
            fn, *args = cdr
            return eval_(fn, e)(args)
            # if args:
            #     return eval_(fn, e)([eval_(a, e) for a in args])
            # else:
            #     return eval_(fn, e)(args)

            # print("->", cdr)
            # # snapshot
            # if isinstance(fn, Block): # Calling fresh anonymus function definition
            #     fnobj = eval_(fn, e)
            #     return fnobj(*[eval_(a, e) for a in args])
            # else: # Token = saved function name
            #     if args:
            #         return eval_(fn, e)(*[eval_(a, e) for a in args])
            #     else:
            #         return eval_(fn, e)(args)
            #     # fnobj = e.funcs[fn.string]
            #     # return fnobj(args)

        # Diese beide müssen zu einem ELIF branch zusammengetan werden!!!
        elif e.isbuiltin(car):
            return e.builtins[car.string](*[eval_(b, e) for b in cdr])
        elif e.isuserdefined(car):
            fnobj = e.user_defined[car.string]
            return fnobj(cdr)
        
        elif car.string == "map":
            fn, *args = cdr
            return e.funcs["map"](eval_(fn, e), *[eval_(a, e) for a in args])
        
        elif car.string == "lambda": # create a function objecT
            params_blocks, actions_blocks = extract_params_actions(cdr)
            fn = Function(params_blocks, actions_blocks, e)
            return fn
        else:
            raise SyntaxError(f"{x} not known")
    else: # x is a Token
        try:
            return int(x.string)
        except ValueError:
            try:
                return float(x.string)
            except ValueError:
                return e.resolve_token(x)

def extract_params_actions(blocks):
    """Returns a list of params  blocks and a list of action blocks."""
    param_blocks = []
    action_blocks = []
    for b in blocks:
        if b.head.string == FUNC_PARAM_IDENT: # kw ident has special meaning in the context of lambda
            param_blocks.append(b)
        else:
            action_blocks.append(b)
    return param_blocks, action_blocks


# def extract_params_actions(blocks):
#     params_idx = None
#     # Find the last kw block
#     for i, b in enumerate(blocks):
#         if b.head.string == FUNC_PARAM_IDENT:
#             params_idx = i
#     if params_idx is not None:
#         return blocks[params_idx], blocks[(params_idx+1):]
#     else:                       # Function has no parameters (a nullary)
#         return [], blocks

def rmcomm(toks):
    """Removes comments from tokens"""
    set_commidx_ip(toks)
    incomment = False
    noncomms = []
    for t in toks:
        if t.commidx == 0:
            if incomment: # End of outer block comment
                incomment = False
            else: # start of outer block comment
                incomment = True
        else:
            if not incomment:
                noncomms.append(t)
    return noncomms


def interpstr(s):
    """Interprets the input string"""
    # return eval_(parse(tokenize_str(s)), tlenv)
    return eval_(parse(rmcomm(tokenize_str(s))), tlenv)


import argparse
argparser = argparse.ArgumentParser(description='Process Source.')
argparser.add_argument("-s", nargs="+", required=True)
args = argparser.parse_args()
# Eval lang-core first
for src in ["toplevel.let"]:
    with open(src, "r") as s:
        interpstr(s.read())
# Jetzt das _Zeug vom user
for src in args.s:
    with open(src, "r") as s:
        interpstr(s.read())



s="""
name
  tmp1 10
  TMP name
        tmp2 * tmp1 20
        tmp3 name tl ja
               var + tmp2 10
pret var
"""

# s="""
# name
#   tmp1 10
#   tmp2 name tl ja
#          global 11
# """

# s="""
# name
#   tmp1 10
#   tmp2 name tl ja
#          global 1
# """

s="""
name tl ja
  f  lambda
        :name p1
        * p1 1000
name tl ja
  f2  lambda
        :name p1
        f p1
pret f2 2
"""

# print(tokenize_str(s))
# print(parse(tokenize_str(s)).body[2].body[2].body[1].body[1].body[1].body[1].body)
# interpstr(s)
