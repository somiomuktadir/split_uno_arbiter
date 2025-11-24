export class UI {
    constructor() {
        this.app = document.getElementById('app');
        this.mainMenu = document.getElementById('main-menu');
        this.gameBoard = document.getElementById('game-board');

        this.playerHand = document.getElementById('player-hand');
        this.playerActionHand = document.getElementById('player-action-hand');
        this.opponentSlot = document.getElementById('opponent-play-slot');
        this.playerSlot = document.getElementById('player-play-slot');

        this.messageArea = document.getElementById('message-area');
        this.roundInfo = document.getElementById('round-info');

        this.playActionBtn = document.getElementById('play-action-btn');

        this.game = null;
    }

    init(game) {
        this.game = game;
        this.bindEvents();
    }

    bindEvents() {
        document.getElementById('start-game-btn').addEventListener('click', () => {
            this.showScreen('game-board');
            this.game.start();
        });

        this.playActionBtn.addEventListener('click', () => {
            // Toggle action hand visibility or mode?
            // For now, let's just say it passes the action phase if clicked when empty?
            // Or maybe it's a "Pass" button during action phase.
            if (this.game.state === 'PLAY_ACTION') {
                this.game.passActionPhase();
            }
        });
    }

    showScreen(screenId) {
        document.querySelectorAll('.screen').forEach(s => {
            s.classList.add('hidden');
            s.classList.remove('active');
        });
        const screen = document.getElementById(screenId);
        screen.classList.remove('hidden');
        screen.classList.add('active');
    }

    updateGameState(game) {
        // Update Stats
        document.getElementById('player-num-count').textContent = game.player.numberCount;
        document.getElementById('player-action-count').textContent = game.player.actionCount;
        document.getElementById('ai-num-count').textContent = game.ai.numberCount;
        document.getElementById('ai-action-count').textContent = game.ai.actionCount;

        this.roundInfo.textContent = `Round ${game.roundCount}`;

        this.updateHand(game.player);
    }

    updateHand(player) {
        this.playerHand.innerHTML = '';
        player.numberHand.forEach(card => {
            const cardEl = this.createCardElement(card);
            cardEl.addEventListener('click', () => {
                if (this.game.state === 'PLAY_NUMBER') {
                    this.game.playNumberCard(card.id);
                }
            });
            this.playerHand.appendChild(cardEl);
        });

        this.playerActionHand.innerHTML = '';
        player.actionHand.forEach(card => {
            const cardEl = this.createCardElement(card);
            cardEl.addEventListener('click', () => {
                if (this.game.state === 'PLAY_ACTION') {
                    this.game.playActionCard(card.id);
                }
            });
            this.playerActionHand.appendChild(cardEl);
        });

        // Update Action Button Text
        if (this.game.state === 'PLAY_ACTION') {
            this.playActionBtn.disabled = false;
            this.playActionBtn.textContent = "Pass Action";
        } else {
            this.playActionBtn.disabled = true;
            this.playActionBtn.textContent = "Wait...";
        }
    }

    createCardElement(card) {
        const el = document.createElement('div');
        el.className = `card ${card.color}`;
        if (card.type === 'action') el.classList.add('action');

        el.innerHTML = `
            <div class="card-corner top">${card.displayValue}</div>
            <div class="card-value">${card.displayValue}</div>
            <div class="card-corner bottom">${card.displayValue}</div>
        `;
        return el;
    }

    showPlayedCards(playerCard, aiCard) {
        this.playerSlot.innerHTML = '';
        this.opponentSlot.innerHTML = '';

        if (playerCard) {
            this.playerSlot.appendChild(this.createCardElement(playerCard));
            this.playerSlot.classList.add('active');
        }

        if (aiCard) {
            this.opponentSlot.appendChild(this.createCardElement(aiCard));
            this.opponentSlot.classList.add('active');
        }
    }

    clearPlayArea() {
        this.playerSlot.innerHTML = '';
        this.opponentSlot.innerHTML = '';
        this.playerSlot.classList.remove('active');
        this.opponentSlot.classList.remove('active');
    }

    log(message) {
        this.messageArea.textContent = message;
        // Optional: Add to a scrollable log if needed
        console.log(message);
    }

    enableNumberInput(enabled) {
        if (enabled) {
            this.playerHand.classList.remove('disabled');
            this.game.state = 'PLAY_NUMBER';
        } else {
            this.playerHand.classList.add('disabled');
        }
    }

    enableActionInput(enabled) {
        if (enabled) {
            this.playerActionHand.classList.remove('disabled');
            this.game.state = 'PLAY_ACTION';
            this.playActionBtn.disabled = false;
        } else {
            this.playerActionHand.classList.add('disabled');
            this.playActionBtn.disabled = true;
        }
    }

    showGameOver(winner) {
        const modal = document.getElementById('modal-overlay');
        const content = document.getElementById('modal-content');
        modal.classList.remove('hidden');

        content.innerHTML = `
            <h2 class="modal-title">${winner.name} Wins!</h2>
            <p>Game Over</p>
            <div class="modal-actions">
                <button class="btn primary" onclick="location.reload()">Play Again</button>
            </div>
        `;
    }

    showColorPicker(callback) {
        const modal = document.getElementById('modal-overlay');
        const content = document.getElementById('modal-content');
        modal.classList.remove('hidden');

        content.innerHTML = `
            <h2 class="modal-title">Choose Color</h2>
            <div class="color-options" style="display: flex; gap: 1rem; justify-content: center;">
                <button class="btn" style="background: var(--accent-red); width: 60px; height: 60px;" data-color="red"></button>
                <button class="btn" style="background: var(--accent-blue); width: 60px; height: 60px;" data-color="blue"></button>
                <button class="btn" style="background: var(--accent-green); width: 60px; height: 60px;" data-color="green"></button>
                <button class="btn" style="background: var(--accent-yellow); width: 60px; height: 60px;" data-color="yellow"></button>
            </div>
        `;

        const buttons = content.querySelectorAll('button');
        buttons.forEach(btn => {
            btn.addEventListener('click', () => {
                const color = btn.getAttribute('data-color');
                modal.classList.add('hidden');
                callback(color);
            });
        });
    }
}
