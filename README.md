# Let

- Es ist wichtig zu beachten dass NUR Block-builders einen Block generieren. Diese sind u.a.
  - Lambdas
  - Lambda Parameter Namen
  - Cond Teile `IF` THEN ELSE
- Jeder Block-builder saugt NUR soviel Items (Cells oder Blocks) wie für ihn definiert. Es ist für Funktionen z.B. Anzahln der Argumente, für Lambda Parameter mit Defaultargumenten nur ein einziges Item etc. So wird eine unnötige block-artige Schreibweise vermieden, wenn es z.B. natürlicher und intuitiver aussieht wenn man etwas kurz auf eine Zeile zusammenfassen könnte. Ein Beispiel:

```
define meine-funktion lambda .arg1 .arg2 + 1 1 .arg3 5 + arg1 arg2 arg3
```
Obwohl auf einer Zeile zusammengefasst, ist es eindeutig dass ab `.arg2` nicht im Block `.arg1` reingehört. Auch `.arg3` liegt nach obiger Definition eindeitig außerhalb der `+`-Block, da Lambda-Parameter Block-builder sind die jeweils *das nächste* Item (das selbst kein Parameter ist) einsaugen.  

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
