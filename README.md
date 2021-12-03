# Let Anatomie


1.  Es ist wichtig zu beachten dass NUR Block-builders einen Block generieren. Diese sind u.a.
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

3. Lambda Parameter werden niemals als normales Item angesehen. Die haben immer _nur_ die Funktion eines Parameter Block-builders, d.h. ein Parametername kann nicht als Defaultargument vom vorigen Parameter fehlinterpretiert werden, auch wenn diese beide auf der gleichen Zeile vorkommen.

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
