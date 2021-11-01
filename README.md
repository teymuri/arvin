# Let
---
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

