# Tila

Tila is an interpreted purely functional programming language.
**Note**: Streem is still in the design stage. It's not working yet. Stay tuned.

## Compiling
### Dependencies
    - gcc
    - glib
### Run make
    `make`

# LET Anatomie
- ## Single Expressivity
  - ist eine ganz zentrale Eigenschaft der Sprache und entscheidender Faktor deren Syntax/Semnatik. U.a. haben `lambda` und `let` Konstrukte diese Eigenschaft. 
- ## Lambda
  - ### Parameterreihenfolge beim Aufrufen
    - generally when no param is named when calling, the definition order of params is applied to the passed arguments. If any of the params is named, the **parameter definition order** is applied to args from there until the _last needed argument_ or up to the next named parameter. 
      ```
      (a 3-arity function called fn is defined with params .x, .y and .z)
      fn 1 2 3 4 (since non of parameters are named, arguments are passed in order,
      so x=1, y=2, z=3 and 4 is evaled but not used, this is exactly the same as if naming 
      only the first parameter:)
      fn .x 1 2 3 4

      (this same behavior is true for naming any of the params)
      fn 1 .y 2 3 4 (here the order is maintained from the first named parameter .y, so .z = 3 and 
      4 is not used)

      (named params can appear , switching the parameter-definition-order on and off, for example
       with the 6-arity function:)
      define fn
        lambda .a .b 2 .c .d 4 .e .f
          list a b c d e f
      fn 1 .c 3 .e 5 6 (=> LIST 1 2 3 4 5 6)
      ```
    - Es könnte auch den Fall geben, dass ich nur einem Parameter ein Argument übergeben möchte und für die restlichen Parameter auf deren Default-Argumente zurückgreifen will. Dafür müss ich wieder auf die Blocke und die Formatierungen Acht geben:
      ```
      (Bspl. Eine 3-arity fn)
      define fn
        lambda .x 1 .y 2 .z 3
          list x y z
      (möchte nur y ein Argument geben)
      fn .y 3 4 5 6 7 8 ... (=> es darf einfach nach dem .y nix auf der selben 
      Zeile kommen, sonst wird .z auch 4 zugewiesen! (So eine Konstellation wäre z.B. beim .& Parameter
      sinnvoll, da wären die restlichen Werte auf der Zeile in den .&-Parameter gepackt))
      (Mit sauber schreiben ist das Problem gelöst:)
      fn .y 3
      4 5 6 7 8 etc...
      ``` 
  - Return Statement (final expression)
    - every lambda needs (at least) a return expression. _LET does **not** try to guess your return expressions_. There might be errors if we try to write sloppy one-liners. Care must be taken to specify the _final expression_ clearly (i.e. coming into the lambda's block itself). Here is an example: the one-liner `lambda .x .y .z + x y` will lead to an error, because the last expression `+ x y` is interpreted as the default-argument for the `z` parameter. If you intended it as the final expression, then put it under the block of `lambda`:
      ```
      lambda .x .y .z
        + x y
      ```
      Note that the same problem (lack of the final expression) will exist in the following contexts as _parameters look eagerly for their default arguments_:
      ```
      (+ x y still resides in the .z parameter block)
      lambda .x .y .z
                     + x y (=> error: lambda missing it's evaluation expression)
      
      (+ x y resides in other parameters (.y) blocks)
      lambda .x .y .z
                  + x y (=> runtime env-error: lookup failed for y)

      (+ x y resides in other parameters (.x) blocks)
      lambda .x .y .z
               + x y (=> runtime env-error: lookup failed for x (arg eval right to left???))
      ```
- ## Conditional Evaluation
  Bedingte Anweisungen werden in LET mit `case` und seinen `if then else` Konstrukten gemacht. Die allgemeine Syntax ist
  ```
  case
    if expr1 then action1
    .
    .
    .
    if expr-k then action-k
    else default-action
  ```
  Im Kontext von `case` sind die Wörter `if then else` reserviert und generieren bzw. schließen Blocks von Expressions und Actions. Diese reservierten Wörter ermöglichen auch einen eindeutigen one-liner: `case if expr1 then action1 if expr2 then action2 else default-action`. Die `else` Verzweigung ist dabei optional.


1. Es ist wichtig zu beachten dass NUR Block-builders einen Block generieren. Diese sind u.a.
  - Lambdas
  - Lambda Parameter Namen
  - Cond Teile `IF` THEN ELSE
2. Jeder Block-builder saugt **maximal** soviel Elements (Bricks oder Blocks) auf wie für ihn definiert. Es ist für Funktionen z.B. Anzahln der Argumente + 1 Return-Expression, für Lambda Parameter mit Defaultargumenten nur ein einziges Item etc. So wird eine unnötige block-artige Schreibweise vermieden, wenn es z.B. natürlicher und intuitiver erscheint wenn man etwas kurz auf eine Zeile zusammenfassen könnte. Ein Beispiel:
folgender Block:
```
define
  meine-funktion
    lambda
      .arg1 (mandatory argument)
      .arg2 + 1 1
      .arg3 5
      * arg1 arg2 arg3
```

könnte dann als einer Einzeiler so geschrieben werden:

```
define meine-funktion lambda .arg1 .arg2 + 1 1 .arg3 5 * arg1 arg2 arg3
```
Obwohl auf einer Zeile zusammengefasst, ist es eindeutig dass ab `.arg2` nicht im Block `.arg1` reingehört. Auch `.arg3` liegt nach obiger Definition eindeitig außerhalb der `+`-Block, da Lambda-Parameter Block-builder sind die jeweils _nur das nächste nicht-parametrige_ (s. hierzu auch den 3. Punkt unten) Item einsaugen, heisst `.arg2` hat schon bekommen was es gebraucht hat (ein einziges Item, hier `+ 1 1`) und ist damit absolut zufrieden und glücklich (eigentlich wird es ein Error sein einem Parameter mehr als ein Expression als Defaultargument zu vergeben!) Ähnliches gilt auch für das Return-Expression vom Lambda oben.

3. Lambda Parameter werden niemals als normales Item angesehen. Die haben immer _nur_ die Funktion eines Parameter Block-builders, d.h. ein Parametername kann nicht als Defaultargument vom vorigen Parameter fehlinterpretiert werden, auch wenn diese beide auf der gleichen Zeile und unmittelbar nacheinander notiert werden.

- Eine Lambda Expression hat mindestens ihr Return Statement. Das heisst die lambd awird so gelesen (von LET):
  - nach dem alle Blocks schon da sind wird das letzte nicht Parameter-ähnlicher Block/Atom als Returnstatement weg geschnitten
  - alles andere wird versucht als PArameter und deren etwaigen default argumente runterzukriegen.
Dies ist besonders dann wichtig/auffälig wenn wir lambdas auf eine Zeile schreiben wollen:

```
(oneliner:)
lambda .x 4 (=> Hier wird 4 als Return Statement gelesen!)
(blocky:)
lambda
  .x 4 (=> das hier wird ein Parse Error, weil das Lambda kein Return hat)
(oder)
lambda
  .x
  4
```
Oben sind die letzten 2 Bsplie eindeutig, in dem man weisst was Parameter/default Argument ist und was Return Statement.

4. Einige Block Generatoren haben eine fest-definierte `max_absorp_capa` (maximale Aufnahmekapazität), e.g.

| Block Generator | maximale Aufnahmekapazität |
| ------ | ------ |
| Lambda Parameter | 0 or 1 (default argument) |
| IF THEN ELSE (COND) | 1 |

## The operator on line
The rightmost operator on line is applied, the the one before it and so on.
If there is a line **without** any operators, the operands are considered to be in the block of their operator 1, 2, 3, 4, ... lines above.
```
+ 2 3 + 4 5
  6 7 8 9
```
the second line consists of only operands (no own operators), so 9 is considered to be in the block of the 2d + line 1 `+ 4 5 9`

## Comments
Comments in Let are written using `(` and `)` tokens. Let will ignore
every thing between a left and it's corresponding right
parenthesis. There is no different comment syntax for inline or block
comments (spread over multiple lines).  Nested comment blocks are also
valid.

```
(this is a comment line)

(comments could as well 
be spread over 
multiple lines)

(also (nested (comment blocks) are) valid)
```


---
Copyright 2021 Amir Teymuri

This file is part of Let.

Let is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Let is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Let.  If not, see <https://www.gnu.org/licenses/>.

