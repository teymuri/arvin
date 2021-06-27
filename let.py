

"""
This is a prototype for the let programming language.
"""


print("(((((( let it be ... ))))))")



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

FUNCOBJ_IDENTIFIER = "'"
class Env:
    counter = 0
    def __init__(self, parenv=None, id_=None):
        self.funcs = builtin_funcs()
        # self.funcs = builtin_funcs()
        self.vars = consts()
        self.parenv = parenv
        self.id = id_ if id_ else self.id_()
        # if parenv:
        #     self.funcs.update(parenv.funcs)
        #     self.vars.update(parenv.vars)
    def __repr__(self):
        return f"(ENV {self.id})"
    def id_(self):
        x = Env.counter
        Env.counter += 1
        return x
    
    def isfunc(self, tok): return tok.string in self.funcs
    
    def resolve_token(self, tok):
        if tok.string.startswith("'"): # return the function object
            return self.funcs[tok.string[1:]]
        else:
            try:
                return self.funcs[tok.string]
            except KeyError:
                try:
                    return self.vars[tok.string]
                except KeyError:
                    if self == tlenv: # if already at the top, token couldn't be resolved!
                        raise NameError(f"name {tok.string} is not defined")
                    else:
                        return self.parenv.resolve_token(tok)
    
    def isblockbuilder(self, tok):
        if tok.string in self.funcs:
            return True
        else:
            if self.parenv:
                return self.parenv.isblockbuilder(tok)
            else:
                return tok.string in SINGLE_NAMING_BLOCK_BUILDERS + \
                    NONNAMING_BLOCK_BUILDERS + MULTIPLE_VALUE_BINDERS + \
                    HIGHER_ORDER_FUNCTIONS + LEXICAL_BLOCK_BUILDERS
    
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

    # params is a block of one or more name blocks
    def __init__(self, params, actions, enclosing_env):
        # Create a fresh env at definition time!
        self.env = Env(parenv=enclosing_env)
        self.params = []
        self.count_obligargs = 0
        self.init_params_ip(params)
        self.actions = actions
    
    def init_params_ip(self, params): # init params at definition time
        for b in params:
            _, name = b.body
            _, *params = name.body
            for p in params:
                nmtok, *defarg = p.body # name token and default argument
                self.params.append(nmtok)
                if defarg:      # If default arguments given at definition time
                    self.env.vars[nmtok.string] = eval_(defarg[0], self.env)
                else:
                    self.env.vars[nmtok.string] = None
                    self.count_obligargs += 1
    
    def __call__(self, args):
        assert len(args) >= self.count_obligargs, \
            f"passed {len(args)} args to a lambda with min arity {self.count_obligargs}"
        callenv = Env(self.env) # The run-time env
        for i, a in enumerate(args):
            if is_param_naming_arg(a):
                _, name, *vals = a.body
                for v in vals[:-1]:
                    eval_(v, self.env)
                callenv.vars[name.string] = eval_(vals[-1], self.env)
            else:               # schau nach der Reihenfolge der Parameter, jetzt sind wir
                # index i von parameter liste
                # for v in vals[:-1]:
                #     pass        # Braucht ein block function zum multi-action
                callenv.vars[self.params[i].string] = eval_(a, self.env)
        for action in self.actions[:-1]:
            eval_(action, callenv)
        return eval_(self.actions[-1], callenv)



HIGHER_ORDER_FUNCTIONS = ("call", "map")

# def ishigherorder(tok): return tok.string in HIGHER_ORDER_FUNCTIONS
SINGLE_NAMING_BLOCK_BUILDERS = ("block","defun", )
NONNAMING_BLOCK_BUILDERS = ("case","call")
LEXICAL_BLOCK_BUILDERS = ("block", "defun", "name", "lambda", "defvar")
MULTIPLE_VALUE_BINDERS = ("defvar", "define")

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
TOKENS_PATT = r"({}|{}|{}|{}|[\w\d.+\-*=]+)".format(
    "\(", "\)", PARSER_BLOCKCUT_IDENT, FUNC_PARAM_IDENT)

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




def token_isin_block(tk, bl):
    """Is tk inside of the head's block?"""
    return tk.start > bl.head.start and tk.line >= bl.head.line



class Block:
    counter = 0
    def __init__(self, head, env, id_=None):
        self.head = head
        self.body = [self.head]
        self.env = env
        self.id = id_ if id_ else self.id()

    def id(self):
        i = Block.counter
        Block.counter += 1
        return i
    def __repr__(self): return f"<Block{self.id} HEAD:{self.head}>"
    def append(self, t): self.body.append(t)


# def ast(parsed_block, tree=[]):
#     for x in parsed_block.body:
#         if isinstance(x, Token):
#             tree.append(x)
#         else: # Block?
#             tree.append(ast(x, []))
#     return tree



def bottommost_blocks(enclosing_blocks):
    # Find the bottom-most line
    maxline = max(enclosing_blocks, key=lambda b: b.head.line).head.line
    # filter all bottommost blocks
    return [b for b in enclosing_blocks if b.head.line == maxline]
# def bottommost_blocks(idx_blocks):
#     # Find the bottom-most line
#     maxline = max(idx_blocks, key=lambda b: b[1].head.line)[1].head.line
#     # filter all bottommost blocks
#     # print([ib for ib in idx_blocks if ib[1].head.line == maxline])
#     return [ib for ib in idx_blocks if ib[1].head.line == maxline]

def rightmost_block(bottommosts):
    # get the rightmost one of them
    return max(bottommosts, key=lambda b: b.head.start)
# def rightmost_block(bottommosts):
#     # get the rightmost one of them
#     return max(bottommosts, key=lambda b: b[1].head.start)

def enclosing_block(tok, blocks): # blocks is a list
    """Returns token's enclosing block."""
    return rightmost_block(bottommost_blocks([b for b in blocks if token_isin_block(tok, b)]))
# def enclosing_block(tok, blocks): # blocks is a list
#     """Returns token's enclosing block."""
#     return rightmost_block(
#         bottommost_blocks(
#             [ib for ib in blocks.items() if token_isin_block(tok, ib[1])]))

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
            if enblock.env.isblockbuilder(t) or (t.string==FUNC_PARAM_IDENT):
                if is_lexenv_builder(t):                
                    B = Block(t, Env(parenv=enblock.env)) # give it a new env
                else:
                    B = Block(t, enblock.env)
                blocktracker.append(B)
            elif enblock.head.string == "name": # Is token an arg to name?
                # This is only a pseudo-block, the idea is to pack
                # pairs of args to name into blocks instead of pairing them
                # into lists.
                B = Block(t, env=enblock.env)
                blocktracker.append(B)
            ################################  and enblock.head.string == "lambda"
            if enblock.env.isblockbuilder(t) or (t.string == FUNC_PARAM_IDENT):
                enblock.append(B)
            elif enblock.head.string == "name": # args to name
                enblock.append(B)
            else:               # Just a simple lonely token!
                enblock.append(t)
    return tlblock

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
                    if vals: # There are any values to be assigned to the name?
                        for val in vals:
                            retval = eval_(val, x.env)
                    else: # No values => Null
                        retval = Void()
                    # write in funcs or in variables?
                    if isinstance(retval, Function):
                        write_env.funcs[b.head.string] = retval
                    else:
                        write_env.vars[b.head.string] = retval
            if access_from_parenv:
                access_from_parenv.vars.update(write_env.vars)
            return retval

        # elif car.string == "defvar": # toplevel var
        #     for var, val in pair(cdr):
        #         tlenv.vars.update([(var.string, eval_(val, e))])
        #     return var.string

        # Higher order functions
        elif car.string == "call": # call a function
            fn, *args = cdr
            # snapshot
            if isinstance(fn, Block): # Calling fresh anonymus function definition
                fnobj = eval_(fn, e)
                return fnobj(*[eval_(a, e) for a in args])
            else: # Token = saved function name
                fnobj = e.funcs[fn.string]
                return fnobj(args)
        
        elif car.string == "map":
            fn, *args = cdr
            return e.funcs["map"](eval_(fn, e), *[eval_(a, e) for a in args])
        
        elif car.string == "lambda": # create a function object
            params_blocks, actions_blocks = extract_params_actions(cdr)
            fn = Function(params_blocks, actions_blocks, e)
            return fn
            # try:
            #     # params_blocks = extract_params_block(cdr)
            #     params, actions = extract_params_actions(cdr)
            # except IndexError:
            #     params = None
            # if params:
            #     params_toks = []
            #     for name_block in params.body[1:]:
            #         params_toks.extend(name_block.body[1:])
            #         eval_(name_block, e, x.env)
            #     F = Function([ptok for ptok in params_toks], actions, e)
            #     return F
            
            # the first block is a block of params
            # params_blocks, *body = cdr
            # params = params_blocks.body[1:]
            # print(x.env.vars)
            # return Function([p.string for p in params], body, e)
        
        elif e.isfunc(car):
            return eval_(car, e)(*[eval_(b, e) for b in cdr])
        
        else:
            raise SyntaxError(f"{(x, x.body)} not known")
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


# import argparse
# argparser = argparse.ArgumentParser(description='Process Source.')
# argparser.add_argument("-s", nargs="+", required=True)
# args = argparser.parse_args()
# # Eval lang-core first
# for src in ["toplevel.let"]:
#     with open(src, "r") as s:
#         interpstr(s.read())
# # Jetzt das _Zeug vom user
# for src in args.s:
#     with open(src, "r") as s:
#         interpstr(s.read())



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
name
  tmp1 10
  tmp2 name
         tmp3 * tmp1 20
         tmp4 name
	        tmp5 name tl ja
	               global 1
"""
s="""
lambda
  @ name
     x 100
     y x
    name c
      +L 1 2 3 4
  * x 8
name 
 &rest 1 2 3
"""
s="""
name tl ja
  fn
    lambda
      :name x
      :name y
     * x y 
      10

pret call fn 3 4
"""
# print(tokenize_str(s))
# print(parse(tokenize_str(s)).body[2].body)

interpstr(s)
