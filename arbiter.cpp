/*******************************************************************************
 * SPLIT UNO - ARBITER APPLICATION
 * 
 * A game arbiter/tracker for Split UNO, a custom variant of the classic UNO
 * card game that separates number cards and action cards into distinct decks.
 * 
 * Author: Muktadir Somio
 * Version: 3.0 (Refactored for N Players)
 * Language: C++17
 * 
 * Description:
 *   This application helps arbitrate games of Split UNO by tracking:
 *   - Player card counts (number and action cards separately)
 *   - Game state (blocks, consecutive wins, deck remaining)
 *   - Win conditions and special card effects
 * 
 * Compilation:
 *   g++ arbiter.cpp -o app
 * 
 * Usage:
 *   ./app
 ******************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <map>

using namespace std;

/*******************************************************************************
 * ENUMERATIONS & STRUCTS
 ******************************************************************************/

// Card types in Split UNO
enum class ActionType {
    BLOCK,
    SKIP,
    REVERSE,
    COLOR_CHANGE,
    WILD,
    DRAW_TWO,
    DRAW_FOUR,
    TRUTH,
    DARE,
    UNKNOWN
};

// Card colors
enum class Color {
    RED, YELLOW, GREEN, BLUE, WILD
};

// Player Structure
struct Player {
    string name;
    int numberCards;
    int actionCards;
    int consecutiveWins;
    bool isBlocked;

    Player(string n, int initialCards) 
        : name(n), numberCards(initialCards), actionCards(0), 
          consecutiveWins(0), isBlocked(false) {}
};

/*******************************************************************************
 * MAIN ARBITER CLASS
 ******************************************************************************/

class SplitUnoArbiter {
private:
    // Game Constants
    static constexpr int INITIAL_CARDS = 20;              // Starting number cards per player
    static constexpr int INITIAL_NUMBER_DECK = 68;        // Remaining number cards
    static constexpr int INITIAL_ACTION_DECK = 32;        // Action cards available
    static constexpr int CONSECUTIVE_WINS_THRESHOLD = 2;  // Wins needed for bonus
    static constexpr int MAX_CARD_NUMBER = 9;             // Highest number card
    static constexpr int MIN_CARD_NUMBER = 0;             // Lowest number card
    static constexpr int CARD_0_DRAW = 1;                 // Cards stolen by playing 0
    static constexpr int CARD_7_NUMBER_DRAW = 2;          // Number cards from card 7
    static constexpr int CARD_7_ACTION_DRAW = 1;          // Action cards from card 7
    
    // Game State
    vector<Player> players;        // List of players
    int numberDeckRemaining;       // Remaining number cards in deck
    int actionDeckRemaining;       // Remaining action cards in deck
    bool gameOver;                 // Has the game ended?
    string winner;                 // Game winner name

    /***************************************************************************
     * INPUT VALIDATION HELPERS
     ***************************************************************************/
    
    void clearInputBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    
    int getValidatedInt(const string& prompt, int min, int max) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                if (value >= min && value <= max) {
                    clearInputBuffer();
                    return value;
                } else {
                    cout << ">>> Error: Please enter a number between " 
                         << min << " and " << max << ".\n";
                }
            } else {
                cout << ">>> Error: Invalid input. Please enter a number.\n";
                clearInputBuffer();
            }
        }
    }
    
    string getValidatedString(const string& prompt, const vector<string>& validOptions) {
        string input;
        while (true) {
            cout << prompt;
            if (cin >> input) {
                input = toUpper(input);
                for (const auto& option : validOptions) {
                    if (input == toUpper(option)) {
                        clearInputBuffer();
                        return input;
                    }
                }
                cout << ">>> Error: Invalid option. Please try again.\n";
            } else {
                cout << ">>> Error: Invalid input. Please try again.\n";
                clearInputBuffer();
            }
        }
    }
    
    string toUpper(string s) const {
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    // Helper to get a player index by name or selection
    int getValidatedPlayerIndex(const string& prompt, int excludeIndex = -1) {
        cout << prompt << endl;
        for (size_t i = 0; i < players.size(); ++i) {
            if (static_cast<int>(i) == excludeIndex) continue;
            cout << "  (" << i + 1 << ") " << players[i].name << endl;
        }
        
        while (true) {
            int choice = getValidatedInt("Select Player: ", 1, players.size());
            int index = choice - 1;
            if (index == excludeIndex) {
                cout << ">>> Error: You cannot select yourself/excluded player.\n";
            } else {
                return index;
            }
        }
    }

    ActionType parseActionType(const string& actionStr) {
        if (actionStr == "BLOCK") return ActionType::BLOCK;
        if (actionStr == "SKIP") return ActionType::SKIP;
        if (actionStr == "REVERSE") return ActionType::REVERSE;
        if (actionStr == "COLOR") return ActionType::COLOR_CHANGE;
        if (actionStr == "WILD") return ActionType::WILD;
        if (actionStr == "+2") return ActionType::DRAW_TWO;
        if (actionStr == "+4") return ActionType::DRAW_FOUR;
        if (actionStr == "TRUTH") return ActionType::TRUTH;
        if (actionStr == "DARE") return ActionType::DARE;
        return ActionType::UNKNOWN;
    }
    
    int drawFromNumberDeck(int amount) {
        if (numberDeckRemaining <= 0) {
            cout << ">>> WARNING: Number deck is exhausted! No cards drawn.\n";
            return 0;
        }
        int actualDraw = min(amount, numberDeckRemaining);
        numberDeckRemaining -= actualDraw;
        return actualDraw;
    }
    
    int drawFromActionDeck(int amount) {
        if (actionDeckRemaining <= 0) {
            cout << ">>> WARNING: Action deck is exhausted! No cards drawn.\n";
            return 0;
        }
        int actualDraw = min(amount, actionDeckRemaining);
        actionDeckRemaining -= actualDraw;
        return actualDraw;
    }

    /***************************************************************************
     * GAME STATE DISPLAY
     ***************************************************************************/
    
    void displayGameState() const {
        cout << "\n" << string(60, '=') << endl;
        cout << "           SPLIT UNO - GAME STATE" << endl;
        cout << string(60, '=') << endl;
        
        for (const auto& p : players) {
            cout << left << setw(15) << p.name 
                 << ": " << setw(2) << p.numberCards << " Num | " 
                 << setw(2) << p.actionCards << " Act";
            if (p.isBlocked) cout << " [BLOCKED]";
            if (p.consecutiveWins > 0) cout << " (Wins: " << p.consecutiveWins << ")";
            cout << endl;
        }
        
        cout << "\nDeck Remaining: Numbers=" << numberDeckRemaining 
             << " | Actions=" << actionDeckRemaining << endl;
        cout << string(60, '=') << "\n" << endl;
    }

    /***************************************************************************
     * NUMBER CARD ROUND HANDLERS
     ***************************************************************************/
    
    void handleNumberRound() {
        vector<int> playedCards(players.size());
        int maxCard = -1;
        vector<int> winners;

        // 1. Collect cards from all non-blocked players
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i].isBlocked) {
                cout << ">>> " << players[i].name << " is BLOCKED and skips this round." << endl;
                players[i].isBlocked = false; // Unblock for next round
                playedCards[i] = -1; // Marker for no card
                continue;
            }

            playedCards[i] = getValidatedInt(
                "Enter " + players[i].name + "'s card (0-9): ", 
                MIN_CARD_NUMBER, MAX_CARD_NUMBER
            );

            if (playedCards[i] > maxCard) {
                maxCard = playedCards[i];
                winners.clear();
                winners.push_back(i);
            } else if (playedCards[i] == maxCard) {
                winners.push_back(i);
            }
        }

        // 2. Process Special Effects (0 and 7)
        for (size_t i = 0; i < players.size(); ++i) {
            if (playedCards[i] == 0) {
                cout << "\n>>> " << players[i].name << " played 0! Steal 1 card." << endl;
                int targetIdx = getValidatedPlayerIndex("Who to steal from?", i);
                if (players[targetIdx].numberCards > 0) {
                    players[i].numberCards++;
                    players[targetIdx].numberCards--;
                    cout << ">>> Stolen 1 card from " << players[targetIdx].name << "." << endl;
                } else {
                    cout << ">>> Target has no cards to steal!" << endl;
                }
            }
            if (playedCards[i] == 7) {
                cout << "\n>>> " << players[i].name << " played 7! Target draws penalty." << endl;
                int targetIdx = getValidatedPlayerIndex("Who draws penalty?", i);
                int numDrawn = drawFromNumberDeck(CARD_7_NUMBER_DRAW);
                int actDrawn = drawFromActionDeck(CARD_7_ACTION_DRAW);
                players[targetIdx].numberCards += numDrawn;
                players[targetIdx].actionCards += actDrawn;
                cout << ">>> " << players[targetIdx].name << " draws " 
                     << numDrawn << " Num and " << actDrawn << " Act cards." << endl;
            }
        }

        // 3. Resolve Winner
        if (winners.empty()) {
            cout << ">>> All players were blocked! No winner." << endl;
            return;
        }

        if (winners.size() == 1) {
            int winnerIdx = winners[0];
            cout << "\n>>> " << players[winnerIdx].name << " WINS the round with " << maxCard << "!" << endl;
            
            // Winner sheds 1 card
            players[winnerIdx].numberCards = max(0, players[winnerIdx].numberCards - 1);
            players[winnerIdx].consecutiveWins++;

            // Reset others' consecutive wins and make them draw penalty
            for (size_t i = 0; i < players.size(); ++i) {
                if (static_cast<int>(i) != winnerIdx && playedCards[i] != -1) {
                    players[i].consecutiveWins = 0;
                    int drawn = drawFromNumberDeck(1);
                    players[i].numberCards += drawn;
                }
            }
        } else {
            cout << "\n>>> TIE between ";
            for (size_t i = 0; i < winners.size(); ++i) {
                cout << players[winners[i]].name << (i < winners.size()-1 ? ", " : "");
                // Tied players shed 1 card
                players[winners[i]].numberCards = max(0, players[winners[i]].numberCards - 1);
                players[winners[i]].consecutiveWins = 0; // Reset consecutive on tie? Rules ambiguous, resetting for balance.
            }
            cout << "!" << endl;
            cout << ">>> Tied players shed 1 card. All players draw 1 card." << endl; // House rule for ties
            
            for (auto& p : players) {
                p.numberCards += drawFromNumberDeck(1);
            }
        }

        checkConsecutiveWins();
        checkWinCondition();
    }

    /***************************************************************************
     * ACTION CARD HANDLERS
     ***************************************************************************/
    
    void handleActionCard() {
        int playerIdx = getValidatedPlayerIndex("Who is playing an action card?");
        
        string actionStr = getValidatedString(
            "Enter action card type (BLOCK/REVERSE/COLOR/+2/+4/TRUTH/DARE): ",
            {"BLOCK", "SKIP", "REVERSE", "COLOR", "WILD", "+2", "+4", "TRUTH", "DARE"}
        );
        ActionType type = parseActionType(actionStr);

        switch (type) {
            case ActionType::BLOCK:
            case ActionType::SKIP:
                handleBlockCard(playerIdx);
                break;
            case ActionType::REVERSE:
                handleReverseCard(playerIdx);
                break;
            case ActionType::COLOR_CHANGE:
            case ActionType::WILD:
                handleColorChangeCard(playerIdx);
                break;
            case ActionType::DRAW_TWO:
                handleDrawCard(playerIdx, 2);
                break;
            case ActionType::DRAW_FOUR:
                handleDrawCard(playerIdx, 4);
                break;
            case ActionType::TRUTH:
                handleTruthCard(playerIdx);
                break;
            case ActionType::DARE:
                handleDareCard(playerIdx);
                break;
            default:
                cout << ">>> Error: Unknown action type." << endl;
        }
    }

    void handleBlockCard(int playerIdx) {
        cout << "\n>>> " << players[playerIdx].name << " plays BLOCK!" << endl;
        int targetIdx = getValidatedPlayerIndex("Who to BLOCK?", playerIdx);
        
        string counter = getValidatedString(
            "Did " + players[targetIdx].name + " play a BLOCK to counter? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );

        if (counter == "Y" || counter == "YES") {
            cout << ">>> Countered! Both shed 1 Number Card." << endl;
            players[playerIdx].numberCards = max(0, players[playerIdx].numberCards - 1);
            players[targetIdx].numberCards = max(0, players[targetIdx].numberCards - 1);
            players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
            players[targetIdx].actionCards = max(0, players[targetIdx].actionCards - 1);
        } else {
            cout << ">>> " << players[targetIdx].name << " is BLOCKED for next round!" << endl;
            players[targetIdx].isBlocked = true;
            players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
        }
    }

    void handleReverseCard(int playerIdx) {
        cout << "\n>>> " << players[playerIdx].name << " plays REVERSE (Swap Hands)!" << endl;
        int targetIdx = getValidatedPlayerIndex("Who to swap hands with?", playerIdx);
        
        cout << ">>> Swapping hands between " << players[playerIdx].name 
             << " and " << players[targetIdx].name << "!" << endl;
             
        swap(players[playerIdx].numberCards, players[targetIdx].numberCards);
        swap(players[playerIdx].actionCards, players[targetIdx].actionCards);
        
        // Player who played it sheds the card (from their NEW hand count? Or old? 
        // Usually you play then swap. So decrement first.)
        // Actually, if I swap counts, the card I played is gone from my old hand.
        // But now I have the opponent's hand. 
        // Let's assume the card is discarded BEFORE the swap.
        // But wait, I just swapped. So I need to decrement from the TARGET's hand 
        // (which is now my old hand) or just decrement my current hand?
        // Logic: I play card -> Count - 1. Then Swap.
        // So:
        // 1. Decrement player's action card count (the card played).
        players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
        // 2. Swap.
        swap(players[playerIdx].numberCards, players[targetIdx].numberCards);
        swap(players[playerIdx].actionCards, players[targetIdx].actionCards);
    }

    void handleColorChangeCard(int playerIdx) {
        cout << "\n>>> " << players[playerIdx].name << " plays COLOR CHANGE!" << endl;
        cout << ">>> All players shed 1 Number Card." << endl;
        
        for (auto& p : players) {
            p.numberCards = max(0, p.numberCards - 1);
        }
        
        string color = getValidatedString(
            "Enter chosen color (R/Y/G/B): ",
            {"R", "Y", "G", "B", "RED", "YELLOW", "GREEN", "BLUE"}
        );
        cout << ">>> Next player must play " << color << "." << endl;
        players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
    }

    void handleDrawCard(int playerIdx, int amount) {
        cout << "\n>>> " << players[playerIdx].name << " plays +" << amount << "!" << endl;
        int targetIdx = getValidatedPlayerIndex("Who to attack?", playerIdx);
        
        // Check for counter
        string hasCounter = getValidatedString(
            "Did " + players[targetIdx].name + " counter with +2/+4? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );

        if (hasCounter == "Y" || hasCounter == "YES") {
            string oppCard = getValidatedString("Enter counter card (+2/+4): ", {"+2", "+4"});
            int oppAmount = (oppCard == "+2") ? 2 : 4;
            
            int diff = abs(amount - oppAmount);
            int loserDraw = 1 + diff;

            if (amount > oppAmount) {
                cout << ">>> " << players[playerIdx].name << " wins counter! " 
                     << players[targetIdx].name << " draws " << loserDraw << "." << endl;
                players[targetIdx].numberCards += drawFromNumberDeck(loserDraw);
            } else if (oppAmount > amount) {
                cout << ">>> " << players[targetIdx].name << " wins counter! " 
                     << players[playerIdx].name << " draws " << loserDraw << "." << endl;
                players[playerIdx].numberCards += drawFromNumberDeck(loserDraw);
            } else {
                cout << ">>> Tie! Both shed action card and draw 1 Number Card." << endl;
                players[playerIdx].numberCards += drawFromNumberDeck(1);
                players[targetIdx].numberCards += drawFromNumberDeck(1);
            }
            // Both shed their action cards
            players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
            players[targetIdx].actionCards = max(0, players[targetIdx].actionCards - 1);
        } else {
            cout << ">>> " << players[targetIdx].name << " takes the hit! Draws " << amount << "." << endl;
            players[targetIdx].numberCards += drawFromNumberDeck(amount);
            players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
        }
    }

    void handleTruthCard(int playerIdx) {
        cout << "\n>>> " << players[playerIdx].name << " plays TRUTH!" << endl;
        int targetIdx = getValidatedPlayerIndex("Who to ask?", playerIdx);
        
        string response = getValidatedString(
            "Did " + players[targetIdx].name + " answer? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );

        if (response == "N" || response == "NO") {
            int choice = getValidatedInt(
                "Penalty Choice:\n1. Attacker gets 2 Action, Target gets 2 Number\n2. Target gets 5 Number\nChoice: ", 1, 2);
            
            if (choice == 1) {
                players[playerIdx].actionCards += drawFromActionDeck(2);
                players[targetIdx].numberCards += drawFromNumberDeck(2);
            } else {
                players[targetIdx].numberCards += drawFromNumberDeck(5);
            }
        }
        
        players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
        players[playerIdx].numberCards = max(0, players[playerIdx].numberCards - 1);
    }

    void handleDareCard(int playerIdx) {
        cout << "\n>>> " << players[playerIdx].name << " plays DARE!" << endl;
        int targetIdx = getValidatedPlayerIndex("Who to dare?", playerIdx);
        
        string response = getValidatedString(
            "Did " + players[targetIdx].name + " complete the dare? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );

        if (response == "N" || response == "NO") {
            cout << ">>> " << players[targetIdx].name << " FORFEITS! " << players[playerIdx].name << " WINS!" << endl;
            gameOver = true;
            winner = players[playerIdx].name;
        } else {
            players[playerIdx].actionCards = max(0, players[playerIdx].actionCards - 1);
            players[playerIdx].numberCards = max(0, players[playerIdx].numberCards - 1);
        }
    }

    /***************************************************************************
     * GAME FLOW LOGIC
     ***************************************************************************/
    
    void checkConsecutiveWins() {
        for (auto& p : players) {
            if (p.consecutiveWins >= CONSECUTIVE_WINS_THRESHOLD) {
                cout << "\n>>> " << p.name << " has " << CONSECUTIVE_WINS_THRESHOLD << " consecutive wins!" << endl;
                int choice = getValidatedInt(
                    "Choose: (1) Draw 1 Action Card OR (2) All opponents draw 2 Number Cards: ", 1, 2);
                
                if (choice == 1) {
                    p.actionCards += drawFromActionDeck(1);
                } else {
                    for (auto& opp : players) {
                        if (opp.name != p.name) {
                            opp.numberCards += drawFromNumberDeck(2);
                        }
                    }
                }
                p.consecutiveWins = 0;
            }
        }
    }
    
    void handleDrawChallenge(int winnerIdx) {
        // Check if any other player wants to challenge
        cout << "\n>>> " << players[winnerIdx].name << " has 0 cards! Checking for challenges..." << endl;
        
        string challenge = getValidatedString("Any challenges? (Y/N): ", {"Y", "N", "YES", "NO"});
        if (challenge == "N" || challenge == "NO") {
            gameOver = true;
            winner = players[winnerIdx].name;
            return;
        }

        int challengerIdx = getValidatedPlayerIndex("Who is challenging?", winnerIdx);
        
        // Logic: Challenger plays +2/+4. Winner must draw unless they have counter?
        // Simplified: If valid challenge, winner draws.
        string cardType = getValidatedString("Challenge card (+2/+4): ", {"+2", "+4"});
        int amount = (cardType == "+2") ? 2 : 4;
        
        cout << ">>> Challenge accepted! " << players[winnerIdx].name << " draws " << amount << "." << endl;
        players[winnerIdx].numberCards += drawFromNumberDeck(amount);
        players[challengerIdx].actionCards = max(0, players[challengerIdx].actionCards - 1);
    }
    
    void checkWinCondition() {
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i].numberCards == 0) {
                handleDrawChallenge(i);
                if (gameOver) return;
            }
        }
    }
    
    void manualAdjustment() {
        cout << "\n--- Manual Adjustment ---" << endl;
        int pIdx = getValidatedPlayerIndex("Select player to adjust:");
        
        cout << "1. Number Cards\n2. Action Cards\n3. Reset Wins" << endl;
        int choice = getValidatedInt("Choice: ", 1, 3);
        
        if (choice == 1) {
            players[pIdx].numberCards = getValidatedInt("New Count: ", 0, 100);
        } else if (choice == 2) {
            players[pIdx].actionCards = getValidatedInt("New Count: ", 0, 50);
        } else {
            players[pIdx].consecutiveWins = 0;
        }
    }

public:
    SplitUnoArbiter() {
        numberDeckRemaining = INITIAL_NUMBER_DECK;
        actionDeckRemaining = INITIAL_ACTION_DECK;
        gameOver = false;
    }
    
    void setupGame() {
        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════════╗\n";
        cout << "║          SPLIT UNO ARBITER - GAME TRACKER v3.0             ║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n";
        
        cout << ">>> STRICTLY 2 PLAYERS MODE <<<\n";
        int numPlayers = 2;
        for (int i = 1; i <= numPlayers; ++i) {
            string name;
            cout << "Enter name for Player " << i << ": ";
            cin >> name;
            players.emplace_back(name, INITIAL_CARDS);
        }
        clearInputBuffer(); // Clear newline after name inputs
    }
    
    void run() {
        setupGame();
        displayGameState();
        
        while (!gameOver) {
            cout << "\n--- NEW ROUND ---" << endl;
            cout << "1. Number Round\n2. Action Card\n3. Display State\n4. Adjust\n5. End Game" << endl;
            int choice = getValidatedInt("Choice: ", 1, 5);
            
            switch (choice) {
                case 1: handleNumberRound(); break;
                case 2: handleActionCard(); break;
                case 3: displayGameState(); break;
                case 4: manualAdjustment(); break;
                case 5: gameOver = true; break;
            }
            
            if (!gameOver && (choice == 1 || choice == 2)) {
                displayGameState();
            }
        }
        
        if (!winner.empty()) {
            cout << "\n🏆 WINNER: " << winner << " 🏆\n" << endl;
        }
    }
};

int main() {
    SplitUnoArbiter arbiter;
    arbiter.run();
    return 0;
}