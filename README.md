# Split UNO - Game Arbiter

A command-line arbiter application for **Split UNO**, a creative variant of the classic UNO card game that separates number cards and action cards into distinct decks.

## Overview

Split UNO adds a new dimension to regular UNO by:
- **Supporting 2-6 players** in competitive gameplay
- **Separating number cards (0-9) and action cards** into different decks
- Adding **Truth and Dare cards** for enhanced gameplay
- Requiring players to **shed all 20 number cards** to win
- Giving special functions to cards **0 and 7**

The arbiter app helps track game state, resolve disputes, and maintain fair play without constant manual bookkeeping.

## Features

✅ **Multi-Player Support** - Supports 2-6 players simultaneously  
✅ **Robust Input Validation** - Prevents crashes from invalid inputs  
✅ **Comprehensive Error Handling** - Graceful handling of edge cases  
✅ **Game State Tracking** - Monitors card counts, blocks, consecutive wins  
✅ **Action Card Support** - BLOCK, REVERSE, COLOR CHANGE, +2, +4, TRUTH, DARE  
✅ **Special Number Cards** - Card 0 (steal), Card 7 (draw penalty)  
✅ **Consecutive Win Bonuses** - Rewards for winning streaks  
✅ **Win Condition Challenges** - +2/+4 last-turn defense mechanic  
✅ **Manual Adjustments** - Arbiter can correct mistakes  

## Requirements

- **C++ Compiler** with C++17 support (g++, clang++, etc.)
- **Make** (optional, for easier building)

## Installation & Compilation

### Using Make (Recommended)

```bash
# Build the arbiter
make

# Build and run
make run

# Build with debug symbols
make debug

# Clean build artifacts
make clean
```

### Manual Compilation

```bash
# Release build
g++ -std=c++17 -Wall -Wextra -O2 -o split_uno_arbiter arbiter.cpp

# Debug build
g++ -std=c++17 -Wall -Wextra -g -O0 -o split_uno_arbiter arbiter.cpp
```

## Usage

Run the arbiter application:

```bash
./split_uno_arbiter
```

### Main Menu Options

1. **Play Number Card Round** - All players play number cards (0-9)
2. **Play Action Card** - A player uses an action card
3. **Display Game State** - View current card counts and status
4. **Manual Adjustment** - Correct errors in game state
5. **End Game** - Terminate the current game

### Example Gameplay Flow

```
1. Select option 1 (Play Number Card Round)
2. Enter Player A's card: 7
3. Enter Player B's card: 3
>>> Player A played 7! B must draw 2 Number Cards and 1 Action Card.
>>> Player A WINS the round! (7 > 3)

4. Game state updates automatically
5. Continue playing rounds until a player reaches 0 cards
```

## Game Rules

For detailed game rules, see [ruleset.pdf](ruleset.pdf).

### Quick Reference

- **Win Condition**: First player to shed all 20 number cards
- **Card 0**: Steal 1 random card from opponent
- **Card 7**: Opponent draws 2 number + 1 action card
- **Consecutive Wins**: 2 wins grants a bonus (action card or opponent penalty)
- **BLOCK**: Opponent skips next round
- **REVERSE**: Players swap entire hands
- **COLOR CHANGE**: Both shed 1 card, choose opponent's color
- **+2/+4**: Opponent draws cards (can counter with own +2/+4)
- **TRUTH**: Opponent answers question or faces penalty
- **DARE**: Opponent completes dare or forfeits game

## Version History

### Version 3.0 (Current) - Multi-Player Support
- ✅ **Refactored for N Players** - Now supports 2-6 players (previously 2-player only)
- ✅ Enhanced player selection system with indexed menus
- ✅ Improved game state display for multiple players
- ✅ Updated ruleset documentation for multi-player gameplay
- ✅ Maintained all Version 2.0 code quality improvements

### Version 2.0 - Code Quality Improvements
- ✅ Added comprehensive input validation
- ✅ Added error handling and bounds checking
- ✅ Removed code duplication (refactored draw cards, win conditions)
- ✅ Added 200+ lines of documentation and comments
- ✅ Replaced magic numbers with named constants
- ✅ Created Makefile for easy building
- ✅ Improved user prompts and error messages

### Version 1.0 - Initial Release
- Basic game arbiter functionality
- Support for all card types and game rules

## Testing Status

✅ Compilation with strict warnings (gcc/clang)  
✅ Input validation edge cases  
✅ Basic game flow scenarios  
🔄 Extensive multi-game stress testing (in progress)  

## Known Limitations

- No save/load game state feature
- No AI opponent for solo play
- Terminal-based interface only (no GUI)
- Requires manual input for all actions

## Contributing

This is a personal project, but suggestions and improvements are welcome!

## License

Free to use and modify for personal and educational purposes.

## Author

Muktadir Somio

---

**Have fun playing Split UNO! 🎮🃏**
