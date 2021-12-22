# Let Anatomie


1. Es ist wichtig zu beachten dass NUR Block-builders einen Block generieren. Diese sind u.a.
  - Lambdas
  - Lambda Parameter Namen
  - Cond Teile `IF` THEN ELSE
2. Jeder Block-builder saugt **maximal** soviel Items (Cells oder Blocks) auf wie für ihn definiert. Es ist für Funktionen z.B. Anzahln der Argumente + 1 Return-Expression, für Lambda Parameter mit Defaultargumenten nur ein einziges Item etc. So wird eine unnötige block-artige Schreibweise vermieden, wenn es z.B. natürlicher und intuitiver erscheint wenn man etwas kurz auf eine Zeile zusammenfassen könnte. Ein Beispiel:
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
  1. Bzl. dessen, wie der letzte Lambda Parameter geparsed und in einen Block gepackst wird, da beim letzten Parameter uneindeutig erscheinen mag, ob es sich beim letzten Expression um dessen default Argument oder das Return-Statement der Funktion handelt: die Lambdas werden so gepased dass als erstes das letzte Statement abgeschnitten wird und dann die Parameter mit deren evtl. default Argumenten assoziert werden. Dies bedeutet....????????

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

