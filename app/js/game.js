import { Deck } from './deck.js';
import { Player } from './player.js';
import { AIPlayer } from './ai.js';

export class Game {
    constructor(ui) {
        this.ui = ui;
        this.numberDeck = new Deck('number');
        this.actionDeck = new Deck('action');
        this.player = new Player('Player');
        this.ai = new AIPlayer();

        this.roundCount = 0;
        this.state = 'MENU'; // MENU, PLAY_NUMBER, RESOLVE_NUMBER, PLAY_ACTION, GAME_OVER
        this.currentPhase = null;

        this.tempPlayedCards = { player: null, ai: null };
    }

    start() {
        this.state = 'PLAY_NUMBER';
        this.roundCount = 1;
        this.dealInitialCards();
        this.ui.updateGameState(this);
        this.startRound();
    }

    dealInitialCards() {
        for (let i = 0; i < 20; i++) {
            this.player.addCard(this.numberDeck.draw());
            this.ai.addCard(this.numberDeck.draw());
        }
        // No action cards initially
    }

    startRound() {
        this.ui.log(`--- Round ${this.roundCount} ---`);

        // Reset block status if it was for "next round"
        // Actually rules say "Blocked players skip the entire round".
        // So we check block status here.

        if (this.player.isBlocked && this.ai.isBlocked) {
            this.ui.log("Both players blocked! Round skipped.");
            this.player.isBlocked = false;
            this.ai.isBlocked = false;
            this.endRound();
            return;
        }

        this.ui.enableNumberInput(true);

        // If AI is not blocked, it chooses a card
        if (!this.ai.isBlocked) {
            this.tempPlayedCards.ai = this.ai.chooseNumberCard(this.player.numberCount);
        } else {
            this.tempPlayedCards.ai = null;
            this.ui.log("AI is blocked!");
        }

        // If Player is blocked, they can't play
        if (this.player.isBlocked) {
            this.ui.log("You are blocked!");
            this.ui.enableNumberInput(false);
            this.resolveRound(); // Proceed directly to resolution (AI wins by default if played)
        }
    }

    playNumberCard(cardId) {
        const card = this.player.removeCard(cardId);
        this.tempPlayedCards.player = card;
        this.ui.updateHand(this.player);
        this.resolveRound();
    }

    resolveRound() {
        const pCard = this.tempPlayedCards.player;
        const aiCard = this.tempPlayedCards.ai;

        this.ui.showPlayedCards(pCard, aiCard);

        setTimeout(() => {
            // Handle Special Cards (0 and 7) BEFORE winner determination
            // 0 - Steal
            if (pCard && pCard.value === 0) this.handleSteal(this.player, this.ai);
            if (aiCard && aiCard.value === 0) this.handleSteal(this.ai, this.player);

            // 7 - Penalty
            if (pCard && pCard.value === 7) this.handlePenalty7(this.ai);
            if (aiCard && aiCard.value === 7) this.handlePenalty7(this.player);

            // Determine Winner
            let winner = null;

            if (pCard && !aiCard) winner = 'player';
            else if (!pCard && aiCard) winner = 'ai';
            else if (pCard && aiCard) {
                if (pCard.value > aiCard.value) winner = 'player';
                else if (aiCard.value > pCard.value) winner = 'ai';
                else winner = 'tie';
            } else {
                winner = 'none'; // Both blocked
            }

            this.applyRoundResult(winner, pCard, aiCard);
        }, 1500);
    }

    handleSteal(thief, victim) {
        if (victim.numberHand.length > 0) {
            // Random steal
            const stolenIndex = Math.floor(Math.random() * victim.numberHand.length);
            const stolenCard = victim.numberHand.splice(stolenIndex, 1)[0];
            thief.addCard(stolenCard);
            this.ui.log(`${thief.name} stole a card from ${victim.name}!`);
            this.ui.updateGameState(this);
        }
    }

    handlePenalty7(victim) {
        this.ui.log(`${victim.name} hit by Penalty 7!`);
        this.drawCards(victim, 2, 'number');
        this.drawCards(victim, 1, 'action');
    }

    applyRoundResult(winner, pCard, aiCard) {
        if (winner === 'player') {
            this.ui.log("You Win the Round!");
            // Winner sheds card (already removed from hand)
            // Loser (AI) keeps card AND draws 1
            if (aiCard) {
                this.ai.addCard(aiCard); // Return played card
                this.drawCards(this.ai, 1, 'number');
            }
            this.player.consecutiveWins++;
            this.ai.consecutiveWins = 0;
            this.currentPhase = 'PLAYER_ACTION';
        } else if (winner === 'ai') {
            this.ui.log("AI Wins the Round!");
            // Winner sheds
            // Loser (Player) keeps card AND draws 1
            if (pCard) {
                this.player.addCard(pCard);
                this.drawCards(this.player, 1, 'number');
            }
            this.ai.consecutiveWins++;
            this.player.consecutiveWins = 0;
            this.currentPhase = 'AI_ACTION';
        } else if (winner === 'tie') {
            this.ui.log("It's a Tie!");
            // Both shed played cards (already gone)
            // Both draw 1
            this.drawCards(this.player, 1, 'number');
            this.drawCards(this.ai, 1, 'number');
            this.player.consecutiveWins = 0;
            this.ai.consecutiveWins = 0;
            this.currentPhase = 'NONE';
        }

        this.checkConsecutiveWins();
        this.checkWinCondition();

        if (this.state !== 'GAME_OVER') {
            setTimeout(() => this.startActionPhase(), 1500);
        }
    }

    drawCards(player, count, type) {
        const deck = type === 'number' ? this.numberDeck : this.actionDeck;
        for (let i = 0; i < count; i++) {
            if (deck.count === 0) {
                this.ui.log(`${type} Deck Empty!`);
                // In a real game, we might reshuffle discarded cards, but for now just stop.
                break;
            }
            player.addCard(deck.draw());
        }
        this.ui.updateGameState(this);
    }

    checkConsecutiveWins() {
        if (this.player.consecutiveWins >= 2) {
            this.ui.log("Bonus! 2 Consecutive Wins!");
            // Auto-choose bonus for simplicity: Draw Action Card
            this.drawCards(this.player, 1, 'action');
            this.player.consecutiveWins = 0;
        }
        if (this.ai.consecutiveWins >= 2) {
            this.ui.log("AI Bonus! 2 Consecutive Wins!");
            this.drawCards(this.ai, 1, 'action');
            this.ai.consecutiveWins = 0;
        }
    }

    checkWinCondition() {
        if (this.player.numberCount === 0) {
            this.endGame(this.player);
        } else if (this.ai.numberCount === 0) {
            this.endGame(this.ai);
        }
    }

    startActionPhase() {
        if (this.currentPhase === 'NONE') {
            this.endRound();
            return;
        }

        this.ui.log(`${this.currentPhase === 'PLAYER_ACTION' ? "Your" : "AI's"} Action Phase`);

        if (this.currentPhase === 'PLAYER_ACTION') {
            this.ui.enableActionInput(true);
        } else {
            this.ui.enableActionInput(false);
            // AI decides
            const aiAction = this.ai.chooseActionCard(this.player.numberCount, this.player.actionCount);
            if (aiAction) {
                setTimeout(() => {
                    this.ui.log(`AI plays ${aiAction.value}!`);
                    this.resolveAction(this.ai, this.player, aiAction);
                    this.ai.removeCard(aiAction.id);
                    this.ui.updateGameState(this);
                }, 1000);
            } else {
                setTimeout(() => {
                    this.ui.log("AI passes action.");
                    this.endRound();
                }, 1000);
            }
        }
    }

    playActionCard(cardId) {
        const card = this.player.removeCard(cardId);
        this.ui.log(`You played ${card.value}!`);

        // Check for Wild/Color needed
        if (card.value === 'wild' || card.value === 'wild_draw4') {
            this.ui.showColorPicker((color) => {
                card.color = color; // Temporarily set color for resolution
                this.resolveAction(this.player, this.ai, card);
                this.ui.updateGameState(this);
            });
        } else {
            this.resolveAction(this.player, this.ai, card);
            this.ui.updateGameState(this);
        }
    }

    resolveAction(user, target, card) {
        switch (card.value) {
            case 'block':
                target.isBlocked = true;
                this.ui.log(`${target.name} is blocked for next round!`);
                break;
            case 'draw2':
                this.drawCards(target, 2, 'number');
                break;
            case 'wild_draw4':
                this.ui.log(`Color changed to ${card.color}!`);
                this.drawCards(target, 4, 'number');
                break;
            case 'wild':
                this.ui.log(`Color changed to ${card.color}!`);
                // Wild just changes color, but in Split Uno it forces a color play?
                // Rules: "Choose a color that one opponent must play on their next turn."
                // Since we don't enforce color matching in Number Rounds (highest wins), 
                // this might be purely thematic or we need to enforce it.
                // "Color restriction is largely thematic (arbiter tracks intent, not enforcement)" -> OK, we ignore enforcement for now.
                break;
            case 'reverse':
                const tempNum = user.numberHand;
                const tempAct = user.actionHand;
                user.numberHand = target.numberHand;
                user.actionHand = target.actionHand;
                target.numberHand = tempNum;
                target.actionHand = tempAct;
                this.ui.log("Hands Swapped!");
                break;
            case 'truth':
                this.ui.log(`${user.name} asks a Truth!`);
                // Simulating interaction
                if (target.isAI) {
                    this.ui.log("AI answers truthfully (no penalty).");
                } else {
                    // For player, we assume they answer.
                    this.ui.log("You must answer the truth!");
                }
                // Playing player sheds 1 action (done) AND 1 number card
                // We need to remove a number card from user.
                if (user.numberHand.length > 0) {
                    const shed = user.numberHand.pop(); // Shed last card for simplicity
                    this.ui.log(`${user.name} sheds ${shed.displayValue}`);
                }
                break;
            case 'dare':
                this.ui.log(`${user.name} issues a Dare!`);
                if (target.isAI) {
                    this.ui.log("AI accepts the dare.");
                } else {
                    this.ui.log("You must complete the dare!");
                }
                if (user.numberHand.length > 0) {
                    const shed = user.numberHand.pop();
                    this.ui.log(`${user.name} sheds ${shed.displayValue}`);
                }
                break;
            // Implement other actions as needed
        }

        // After action resolution, end round (unless it was async color pick which calls this)
        // If it was AI, we end round here.
        // If it was Player, we end round here.
        if (this.state !== 'GAME_OVER') {
            setTimeout(() => this.endRound(), 1500);
        }
    }

    endRound() {
        // Reset round-specific flags
        this.player.isBlocked = false; // Reset block after round
        this.ai.isBlocked = false;

        this.roundCount++;
        this.ui.clearPlayArea();
        this.startRound();
    }

    endGame(winner) {
        this.state = 'GAME_OVER';
        this.ui.showGameOver(winner);
    }

    passActionPhase() {
        this.ui.log("You passed action.");
        this.endRound();
    }
}
