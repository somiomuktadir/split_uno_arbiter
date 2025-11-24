import { Card } from './card.js';

export class Deck {
    constructor(type) {
        this.type = type; // 'number' or 'action'
        this.cards = [];
        this.reset();
    }

    reset() {
        this.cards = [];
        const colors = ['red', 'blue', 'green', 'yellow'];

        if (this.type === 'number') {
            // Number Deck: 0-9 in four colors
            // Standard Uno has one 0 and two 1-9 per color.
            // Split Uno rules say 68 cards remaining after distribution (20xN players).
            // Let's stick to standard distribution for now and adjust if needed.
            // 1 zero, 2 of 1-9 per color = 19 cards per color * 4 = 76 cards.
            // This is close enough for a standard deck.

            colors.forEach(color => {
                this.cards.push(new Card('number', color, 0, this.generateId()));
                for (let i = 1; i <= 9; i++) {
                    this.cards.push(new Card('number', color, i, this.generateId()));
                    this.cards.push(new Card('number', color, i, this.generateId()));
                }
            });
        } else if (this.type === 'action') {
            // Action Deck: 32 cards total
            // Block/Skip (8), Reverse (8), Draw Two (8), Wild (4), Wild Draw Four (4)
            // Truth/Dare are mentioned in rules but not in the count breakdown in "Deck Preparation"
            // Wait, "Deck Preparation" says:
            // - Block/Skip cards (8 total) -> 2 per color
            // - Reverse cards (8 total) -> 2 per color
            // - Draw Two cards (8 total) -> 2 per color
            // - Wild cards (4 total)
            // - Wild Draw Four cards (4 total)
            // Total = 8+8+8+4+4 = 32.
            // Truth and Dare are mentioned later in "Action Cards". I should probably add them?
            // The rules say "Truth" and "Dare" are cards. Maybe they replace some or are added?
            // "Speed UNO" variant removes them.
            // Let's add them as extra wild-like cards or colored?
            // Rules don't specify color for Truth/Dare. Let's assume they are special actions.
            // For now, I will stick to the 32 count listed in "Deck Preparation" to be safe, 
            // but I'll add Truth/Dare as a variant or if I find more info.
            // Actually, let's stick to the explicit list first.

            colors.forEach(color => {
                for (let i = 0; i < 2; i++) {
                    this.cards.push(new Card('action', color, 'block', this.generateId()));
                    this.cards.push(new Card('action', color, 'reverse', this.generateId()));
                    this.cards.push(new Card('action', color, 'draw2', this.generateId()));
                }
            });

            for (let i = 0; i < 4; i++) {
                this.cards.push(new Card('action', 'wild', 'wild', this.generateId()));
                this.cards.push(new Card('action', 'wild', 'wild_draw4', this.generateId()));
            }
        }

        this.shuffle();
    }

    shuffle() {
        for (let i = this.cards.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [this.cards[i], this.cards[j]] = [this.cards[j], this.cards[i]];
        }
    }

    draw() {
        return this.cards.pop();
    }

    get count() {
        return this.cards.length;
    }

    generateId() {
        return Math.random().toString(36).substr(2, 9);
    }
}
