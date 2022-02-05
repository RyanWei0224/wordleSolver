# wordleSolver
A C++ program that give suggestions to solve the wordle game at https://wordlegame.org/

This solver has a success rate of `99.7%` and uses `4.265` rounds on average among the 12660 5-letter words at wordlegame.org

This is an example of solving a wordle game. The five numbers `X X X X X` after `Color:` is the color of the suggested word. Where:

+ 0 means `the letter is not in the answer (gray)`
+ 1 means `the letter is at the right place (green)`
+ 2 means `the letter is in the answer but at the wrong place (yellow)`

```
12660
Start
serai
Color:0 0 0 2 0
70/673
nyala
Color:2 0 2 0 0
6/47
cagot
Color:0 1 0 0 1
4/5
hadji
Color:1 1 0 0 0
Found: haunt
```