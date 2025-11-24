# Split Uno Web App

A modern, web-based implementation of the **Split Uno** card game variant.

## Features
- **Split Decks**: Separate Number and Action decks for strategic depth.
- **Smart AI**: A computer opponent that uses heuristics to challenge you.
- **Special Cards**: Implementation of unique Split Uno cards like `0` (Steal) and `7` (Penalty).
- **Action Phase**: Strategic phase to use powerful action cards like Block, Reverse, and Wilds.
- **Premium UI**: Dark mode aesthetic with smooth animations.

## How to Play
1.  **Objective**: Be the first to get rid of all your Number cards.
2.  **Number Round**:
    - Both players choose a number card.
    - Highest number wins and is discarded.
    - Loser keeps their card AND draws a penalty.
3.  **Action Phase**:
    - The winner of the Number Round gets priority to play an Action Card.
    - Action cards can block opponents, force draws, or swap hands.
4.  **Winning**: Reach 0 number cards to win!

## Development
- Built with Vanilla HTML, CSS, and JavaScript (ES Modules).
- No external dependencies.

## Running Locally
To run the game, you need a local web server (to handle ES Modules).

```bash
# From the root directory
python3 -m http.server 8000 --directory app
```

Then visit `http://localhost:8000`.
