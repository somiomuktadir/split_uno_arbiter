import { Player } from './player.js';

export class AIPlayer extends Player {
    constructor() {
        super('AI', true);
    }

    // Decision for Number Round
    chooseNumberCard(opponentNumCount) {
        // Simple Heuristic:
        // 1. If we have many cards, try to win to shed.
        // 2. If we have few cards, play conservatively?
        // Actually, winning always sheds a card. Losing makes you draw.
        // So we ALWAYS want to win if possible.
        // Highest number has best chance.

        // Strategy:
        // - If we have a high card (8, 9), play it to secure a win.
        // - If we only have low cards, play the lowest to "dump" it and save mid-cards? 
        //   No, if we lose we keep the card AND draw. So playing low is risky unless we accept loss.
        // - Special cards: 0 (Steal) and 7 (Penalty).
        //   - Play 7 if opponent has few cards (to punish).
        //   - Play 0 if we need cards or want to disrupt.

        // Let's filter playable cards (all number cards are playable).
        if (this.numberHand.length === 0) return null;

        // Check for 7s
        const sevens = this.numberHand.filter(c => c.value === 7);
        if (sevens.length > 0 && opponentNumCount < 5) {
            // Opponent is winning, punish them!
            return sevens[0];
        }

        // Check for 0s
        const zeros = this.numberHand.filter(c => c.value === 0);
        if (zeros.length > 0 && this.numberHand.length < 5) {
            // We are low on cards, maybe steal? Or if we just want to be annoying.
            // Priority is lower than winning usually.
            // Let's save 0 for when we have no high cards.
        }

        // Sort by value descending
        const sorted = [...this.numberHand].sort((a, b) => b.value - a.value);

        // Play highest card to try and win
        return sorted[0];
    }

    // Decision for Action Phase
    // Returns card to play or null (pass)
    chooseActionCard(opponentNumCount, opponentActionCount) {
        if (this.actionHand.length === 0) return null;

        // Strategy:
        // - Save actions for critical moments (opponent < 3 cards).
        // - Use Block if opponent is likely to win next round.
        // - Use +2/+4 if opponent is very close to winning.

        const critical = opponentNumCount <= 4;

        // Filter playable actions
        // In Split Uno, all actions are playable on your turn (no color matching required for actions themselves usually, 
        // but standard Uno requires matching. Split Uno rules say "Play an Action Card - One player uses a strategic card".
        // It implies no restriction on WHAT action card to play.

        let bestCard = null;

        if (critical) {
            // Priority: Wild Draw 4 > Draw 2 > Block > Reverse
            const draw4 = this.actionHand.find(c => c.value === 'wild_draw4');
            const draw2 = this.actionHand.find(c => c.value === 'draw2');
            const block = this.actionHand.find(c => c.value === 'block');

            if (draw4) bestCard = draw4;
            else if (draw2) bestCard = draw2;
            else if (block) bestCard = block;
        }

        // If no critical move, maybe dump a reverse or truth/dare if we have too many actions?
        // Or just pass to save cards.
        if (!bestCard && this.actionHand.length > 3) {
            bestCard = this.actionHand[Math.floor(Math.random() * this.actionHand.length)];
        }

        if (bestCard) {
            // If it's a wild, we need to set the color
            if (bestCard.value === 'wild' || bestCard.value === 'wild_draw4') {
                bestCard.color = this.chooseColor();
            }
            return bestCard;
        }

        return null;
    }

    // Decision for Color Change
    chooseColor() {
        // Choose color we have most of
        const counts = { red: 0, blue: 0, green: 0, yellow: 0 };
        this.numberHand.forEach(c => {
            if (counts[c.color] !== undefined) counts[c.color]++;
        });
        // Return color with max count
        return Object.keys(counts).reduce((a, b) => counts[a] > counts[b] ? a : b);
    }
}
