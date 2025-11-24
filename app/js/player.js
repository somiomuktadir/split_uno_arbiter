export class Player {
    constructor(name, isAI = false) {
        this.name = name;
        this.isAI = isAI;
        this.numberHand = [];
        this.actionHand = [];
        this.consecutiveWins = 0;
        this.isBlocked = false;
    }

    addCard(card) {
        if (card.type === 'number') {
            this.numberHand.push(card);
            this.sortHand(this.numberHand);
        } else {
            this.actionHand.push(card);
            this.sortHand(this.actionHand);
        }
    }

    removeCard(cardId) {
        let index = this.numberHand.findIndex(c => c.id === cardId);
        if (index !== -1) {
            return this.numberHand.splice(index, 1)[0];
        }

        index = this.actionHand.findIndex(c => c.id === cardId);
        if (index !== -1) {
            return this.actionHand.splice(index, 1)[0];
        }
        return null;
    }

    sortHand(hand) {
        hand.sort((a, b) => {
            if (a.color !== b.color) return a.color.localeCompare(b.color);
            // Handle non-numeric values for sorting
            const valA = typeof a.value === 'number' ? a.value : 100;
            const valB = typeof b.value === 'number' ? b.value : 100;
            return valA - valB;
        });
    }

    get numberCount() {
        return this.numberHand.length;
    }

    get actionCount() {
        return this.actionHand.length;
    }

    hasCard(cardId) {
        return this.numberHand.some(c => c.id === cardId) || this.actionHand.some(c => c.id === cardId);
    }
}
