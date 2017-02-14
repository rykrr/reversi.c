# Reversi.c
Copyright (c) Ryan Kerr 2017

A simple, single-player reversi game.

Features:
- Multiple difficulties (Algorithm using strongest, average, or weakest moves)
- Automatic Testing

### Controls
| Key       | Function              |
| --------- | --------------------- |
| F1        | Help Menu             |
| F2        | Cycle Difficulty      |
| q         | Quit                  |
| h,j,k,l   | Move Cursor           |
| arrows    | Move Cursor           |
| Spacebar  | Select                |
| 1         | Decrease Delay        |
| 2         | Increase Delay        |

### Compiler Flags
| Flag              | Description                       | Default Value |
| ----------------- | --------------------------------- | ------------- |
| MAX_BOARD_SIZE    | Set board size (prefer even)      | 16            |
| DEFAULT_MODE      | Set default difficulty            | 0 (easy)      |
| AUTO_TEST         | Game plays itself (sets delay)    | Not set       |

This program was written for fun (and to learn ncurses).
