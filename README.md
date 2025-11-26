# Split UNO - Game Arbiter

A command-line arbiter for **Split UNO**, a strategic variant of UNO where number and action cards are in separate decks.

## Key Features
- **2-6 Player Support**: Competitive gameplay for groups.
- **Dual Deck System**: Separate Number (0-9) and Action decks.
- **Strategic Depth**: Truth/Dare cards, special 0/7 rules, and win streaks.
- **Arbiter Tool**: Tracks game state, scores, and rules automatically.

## Quick Start

### Prerequisites
- C++17 Compiler (g++, clang++)
- Make (optional)

### Build & Run
```bash
# Using Make
make run

# Manual Compilation
g++ -std=c++17 -O2 -o split_uno_arbiter arbiter.cpp
./split_uno_arbiter
```

## Usage
The arbiter tracks the game state. Follow the on-screen menu to:
1. Play Number Rounds (0-9 cards).
2. Play Action Cards (Block, Reverse, +2, etc.).
3. View Game State.

**Note**: This tool tracks state; players must still physically play cards (or use a virtual deck).

## Rules
See [ruleset.md](ruleset.md) for the complete official rules.
