export class Card {
    constructor(type, color, value, id) {
        this.type = type; // 'number' or 'action'
        this.color = color; // 'red', 'blue', 'green', 'yellow', 'wild'
        this.value = value; // 0-9 for numbers, 'block', 'reverse', 'draw2', 'wild', 'wild_draw4', 'truth', 'dare'
        this.id = id; // Unique ID for DOM tracking
    }

    get displayValue() {
        if (this.type === 'number') return this.value;
        switch (this.value) {
            case 'block': return '⊘';
            case 'reverse': return '⇄';
            case 'draw2': return '+2';
            case 'wild': return '🌈';
            case 'wild_draw4': return '+4';
            case 'truth': return '?';
            case 'dare': return '!';
            default: return '';
        }
    }

    toString() {
        return `${this.color} ${this.value}`;
    }
}
