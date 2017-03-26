# Reversi.c
Copyright (c) Ryan Kerr 2017

A simple, single-player reversi game.

Features:
- Multiple difficulties (Algorithm using strongest (w/ corners), average, or weakest moves)
- Automatic Testing

### Controls
| Key       | Function              |
| --------- | --------------------- |
| F1        | Help Menu             |
| p         | hint                  |
| q         | Quit                  |
| h,j,k,l   | Move Cursor           |
| arrows    | Move Cursor           |
| Spacebar  | Select                |

### Compiler Flags (For old version)
| Flag              | Description                       | Default Value |
| ----------------- | --------------------------------- | ------------- |
| MAX_BOARD_SIZE    | Set board size (prefer even)      | 50            |
| DEFAULT_MODE      | Set default difficulty            | 0 (easy)      |
| AUTO_TEST         | Game plays itself (sets delay)    | Not set       |

This program was written for fun (and to learn ncurses).
