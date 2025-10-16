#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

enum class CardType {
    NUMBER,
    BLOCK,
    REVERSE,
    COLOR_CHANGE,
    DRAW_TWO,
    DRAW_FOUR,
    TRUTH,
    DARE
};

enum class Color {
    RED, YELLOW, GREEN, BLUE, WILD
};

struct Card {
    CardType type;
    Color color;
    int number; // -1 for action cards
};

class SplitUnoArbiter {
private:
    int playerACards;
    int playerBCards;
    int playerAActionCards;
    int playerBActionCards;
    int consecutiveWinsA;
    int consecutiveWinsB;
    string lastWinner;
    bool playerABlocked;
    bool playerBBlocked;
    int numberDeckRemaining;
    int actionDeckRemaining;
    bool gameOver;
    string winner;
    
    map<string, string> cardTypeNames = {
        {"0", "NUMBER"}, {"1", "NUMBER"}, {"2", "NUMBER"}, {"3", "NUMBER"},
        {"4", "NUMBER"}, {"5", "NUMBER"}, {"6", "NUMBER"}, {"7", "NUMBER"},
        {"8", "NUMBER"}, {"9", "NUMBER"},
        {"BLOCK", "BLOCK"}, {"SKIP", "BLOCK"},
        {"REVERSE", "REVERSE"},
        {"COLOR", "COLOR_CHANGE"}, {"WILD", "COLOR_CHANGE"},
        {"+2", "DRAW_TWO"},
        {"+4", "DRAW_FOUR"},
        {"TRUTH", "TRUTH"},
        {"DARE", "DARE"}
    };

public:
    SplitUnoArbiter() {
        playerACards = 20;
        playerBCards = 20;
        playerAActionCards = 0;
        playerBActionCards = 0;
        consecutiveWinsA = 0;
        consecutiveWinsB = 0;
        lastWinner = "";
        playerABlocked = false;
        playerBBlocked = false;
        numberDeckRemaining = 68; // 108 - 40 dealt
        actionDeckRemaining = 32; // Approximate action cards in standard UNO + specials
        gameOver = false;
        winner = "";
    }

    void displayGameState() {
        cout << "\n" << string(60, '=') << endl;
        cout << "           SPLIT UNO - GAME STATE" << endl;
        cout << string(60, '=') << endl;
        cout << "Player A: " << playerACards << " Number Cards | " 
             << playerAActionCards << " Action Cards";
        if (playerABlocked) cout << " [BLOCKED]";
        cout << endl;
        
        cout << "Player B: " << playerBCards << " Number Cards | " 
             << playerBActionCards << " Action Cards";
        if (playerBBlocked) cout << " [BLOCKED]";
        cout << endl;
        
        cout << "\nConsecutive Wins: A=" << consecutiveWinsA 
             << " | B=" << consecutiveWinsB << endl;
        cout << "Deck Remaining: Numbers=" << numberDeckRemaining 
             << " | Actions=" << actionDeckRemaining << endl;
        cout << string(60, '=') << "\n" << endl;
    }

    string toUpper(string s) {
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    void handleNumberRound() {
        if (playerABlocked) {
            cout << "\n>>> Player A is BLOCKED! Player B plays alone." << endl;
            handleBlockedRound('B');
            playerABlocked = false;
            return;
        }
        if (playerBBlocked) {
            cout << "\n>>> Player B is BLOCKED! Player A plays alone." << endl;
            handleBlockedRound('A');
            playerBBlocked = false;
            return;
        }

        string cardA, cardB;
        cout << "Enter Player A's card (0-9): ";
        cin >> cardA;
        cout << "Enter Player B's card (0-9): ";
        cin >> cardB;

        cardA = toUpper(cardA);
        cardB = toUpper(cardB);

        int numA = stoi(cardA);
        int numB = stoi(cardB);

        // Handle special effects first
        if (numA == 0) {
            cout << "\n>>> Player A played 0! A draws 1 random card from B's hand." << endl;
            playerACards++;
            playerBCards--;
        }
        if (numB == 0) {
            cout << "\n>>> Player B played 0! B draws 1 random card from A's hand." << endl;
            playerBCards++;
            playerACards--;
        }

        if (numA == 7) {
            cout << "\n>>> Player A played 7! B must draw 2 Number Cards and 1 Action Card." << endl;
            playerBCards += 2;
            playerBActionCards++;
            numberDeckRemaining -= 2;
            actionDeckRemaining--;
        }
        if (numB == 7) {
            cout << "\n>>> Player B played 7! A must draw 2 Number Cards and 1 Action Card." << endl;
            playerACards += 2;
            playerAActionCards++;
            numberDeckRemaining -= 2;
            actionDeckRemaining--;
        }

        // Resolve the bid
        if (numA > numB) {
            cout << "\n>>> Player A WINS the round! (" << numA << " > " << numB << ")" << endl;
            playerACards--; // Shed card
            playerBCards++; // Draw penalty
            numberDeckRemaining--;
            consecutiveWinsA++;
            consecutiveWinsB = 0;
            lastWinner = "A";
        } else if (numB > numA) {
            cout << "\n>>> Player B WINS the round! (" << numB << " > " << numA << ")" << endl;
            playerBCards--; // Shed card
            playerACards++; // Draw penalty
            numberDeckRemaining--;
            consecutiveWinsB++;
            consecutiveWinsA = 0;
            lastWinner = "B";
        } else {
            cout << "\n>>> TIE! Both players shed their cards and draw 1 card." << endl;
            playerACards--; // Shed
            playerBCards--; // Shed
            playerACards++; // Draw
            playerBCards++; // Draw
            consecutiveWinsA = 0;
            consecutiveWinsB = 0;
        }

        checkConsecutiveWins();
        checkWinCondition();
    }

    void handleBlockedRound(char activePlayer) {
        string card;
        if (activePlayer == 'A') {
            cout << "Enter Player A's card (0-9): ";
            cin >> card;
            cout << "\n>>> Player A sheds their card. Player B draws 1 card (penalty)." << endl;
            playerACards--;
            playerBCards++;
            numberDeckRemaining--;
            consecutiveWinsA++;
            consecutiveWinsB = 0;
            lastWinner = "A";
        } else {
            cout << "Enter Player B's card (0-9): ";
            cin >> card;
            cout << "\n>>> Player B sheds their card. Player A draws 1 card (penalty)." << endl;
            playerBCards--;
            playerACards++;
            numberDeckRemaining--;
            consecutiveWinsB++;
            consecutiveWinsA = 0;
            lastWinner = "B";
        }
        
        checkConsecutiveWins();
        checkWinCondition();
    }

    void handleActionCard() {
        string player, action;
        cout << "Which player is playing an action card (A/B)? ";
        cin >> player;
        player = toUpper(player);
        
        cout << "Enter action card type (BLOCK/REVERSE/COLOR/+2/+4/TRUTH/DARE): ";
        cin >> action;
        action = toUpper(action);

        if (action == "BLOCK" || action == "SKIP") {
            handleBlockCard(player);
        } else if (action == "REVERSE") {
            handleReverseCard(player);
        } else if (action == "COLOR" || action == "WILD") {
            handleColorChangeCard(player);
        } else if (action == "+2") {
            handleDrawTwo(player);
        } else if (action == "+4") {
            handleDrawFour(player);
        } else if (action == "TRUTH") {
            handleTruthCard(player);
        } else if (action == "DARE") {
            handleDareCard(player);
        }
    }

    void handleBlockCard(string player) {
        cout << "\n>>> " << player << " plays BLOCK card!" << endl;
        
        string opponent;
        cout << "Did the opponent also play BLOCK? (Y/N): ";
        cin >> opponent;
        opponent = toUpper(opponent);
        
        if (opponent == "Y") {
            cout << ">>> Both played BLOCK! Effects cancel. Both shed 1 Number Card." << endl;
            playerACards--;
            playerBCards--;
            playerAActionCards--;
            playerBActionCards--;
        } else {
            if (player == "A") {
                cout << ">>> Player B is BLOCKED for next round!" << endl;
                playerBBlocked = true;
                playerAActionCards--;
            } else {
                cout << ">>> Player A is BLOCKED for next round!" << endl;
                playerABlocked = true;
                playerBActionCards--;
            }
        }
    }

    void handleReverseCard(string player) {
        cout << "\n>>> " << player << " plays REVERSE card!" << endl;
        cout << ">>> Players exchange their entire hands!" << endl;
        
        swap(playerACards, playerBCards);
        swap(playerAActionCards, playerBActionCards);
        
        if (player == "A") playerAActionCards--;
        else playerBActionCards--;
    }

    void handleColorChangeCard(string player) {
        string color;
        cout << "\n>>> " << player << " plays COLOR CHANGE card!" << endl;
        cout << ">>> Both players shed 1 Number Card each." << endl;
        
        playerACards--;
        playerBCards--;
        
        cout << "Enter the color chosen for opponent's next play (R/Y/G/B): ";
        cin >> color;
        cout << ">>> Opponent must play " << color << " in the next round." << endl;
        
        if (player == "A") playerAActionCards--;
        else playerBActionCards--;
    }

    void handleDrawTwo(string player) {
        cout << "\n>>> " << player << " plays +2 card!" << endl;
        
        string opponent;
        cout << "Did opponent play an action card? (Y/N): ";
        cin >> opponent;
        opponent = toUpper(opponent);
        
        if (opponent == "Y") {
            string oppCard;
            cout << "Enter opponent's action card (+2/+4): ";
            cin >> oppCard;
            oppCard = toUpper(oppCard);
            
            if (oppCard == "+4") {
                cout << ">>> Both shed their action cards. " << player << " draws 3 cards (1 loss + 2 difference)." << endl;
                if (player == "A") {
                    playerAActionCards--;
                    playerBActionCards--;
                    playerACards += 3;
                } else {
                    playerBActionCards--;
                    playerAActionCards--;
                    playerBCards += 3;
                }
                numberDeckRemaining -= 3;
            } else if (oppCard == "+2") {
                cout << ">>> Both shed action cards and draw 1 Number Card each." << endl;
                playerAActionCards--;
                playerBActionCards--;
                playerACards++;
                playerBCards++;
                numberDeckRemaining -= 2;
            }
        } else {
            if (player == "A") {
                cout << ">>> Player B draws 2 cards!" << endl;
                playerAActionCards--;
                playerBCards += 2;
            } else {
                cout << ">>> Player A draws 2 cards!" << endl;
                playerBActionCards--;
                playerACards += 2;
            }
            numberDeckRemaining -= 2;
        }
    }

    void handleDrawFour(string player) {
        cout << "\n>>> " << player << " plays +4 card!" << endl;
        
        string opponent;
        cout << "Did opponent play an action card? (Y/N): ";
        cin >> opponent;
        opponent = toUpper(opponent);
        
        if (opponent == "Y") {
            string oppCard;
            cout << "Enter opponent's action card (+2/+4): ";
            cin >> oppCard;
            oppCard = toUpper(oppCard);
            
            if (oppCard == "+2") {
                cout << ">>> Both shed their action cards. Opponent draws 3 cards (1 loss + 2 difference)." << endl;
                if (player == "A") {
                    playerAActionCards--;
                    playerBActionCards--;
                    playerBCards += 3;
                } else {
                    playerBActionCards--;
                    playerAActionCards--;
                    playerACards += 3;
                }
                numberDeckRemaining -= 3;
            } else if (oppCard == "+4") {
                cout << ">>> Both shed action cards and draw 1 Number Card each." << endl;
                playerAActionCards--;
                playerBActionCards--;
                playerACards++;
                playerBCards++;
                numberDeckRemaining -= 2;
            }
        } else {
            if (player == "A") {
                cout << ">>> Player B draws 4 cards!" << endl;
                playerAActionCards--;
                playerBCards += 4;
            } else {
                cout << ">>> Player A draws 4 cards!" << endl;
                playerBActionCards--;
                playerACards += 4;
            }
            numberDeckRemaining -= 4;
        }
    }

    void handleTruthCard(string player) {
        string response;
        cout << "\n>>> " << player << " plays TRUTH card!" << endl;
        cout << "Did opponent answer the truth question? (Y/N): ";
        cin >> response;
        response = toUpper(response);
        
        if (response == "N") {
            string choice;
            cout << "Choose penalty - (1) Take 2 action cards, opponent draws 2 number cards OR (2) Opponent draws 5 number cards: ";
            cin >> choice;
            
            if (choice == "1") {
                if (player == "A") {
                    playerAActionCards += 2;
                    playerBCards += 2;
                } else {
                    playerBActionCards += 2;
                    playerACards += 2;
                }
                actionDeckRemaining -= 2;
                numberDeckRemaining -= 2;
            } else {
                if (player == "A") {
                    playerBCards += 5;
                } else {
                    playerACards += 5;
                }
                numberDeckRemaining -= 5;
            }
        }
        
        if (player == "A") {
            playerAActionCards--;
            playerACards--;
        } else {
            playerBActionCards--;
            playerBCards--;
        }
    }

    void handleDareCard(string player) {
        string response;
        cout << "\n>>> " << player << " plays DARE card!" << endl;
        cout << "Did opponent complete the dare? (Y/N): ";
        cin >> response;
        response = toUpper(response);
        
        if (response == "N") {
            cout << ">>> Opponent forfeits! " << player << " WINS THE GAME!" << endl;
            gameOver = true;
            winner = player;
        } else {
            if (player == "A") {
                playerAActionCards--;
                playerACards--;
            } else {
                playerBActionCards--;
                playerBCards--;
            }
        }
    }

    void checkConsecutiveWins() {
        if (consecutiveWinsA >= 2) {
            cout << "\n>>> Player A won 2 consecutive rounds!" << endl;
            cout << "Choose: (1) Draw 1 Action Card OR (2) Player B draws 2 Number Cards: ";
            int choice;
            cin >> choice;
            
            if (choice == 1) {
                playerAActionCards++;
                actionDeckRemaining--;
                cout << ">>> Player A draws 1 Action Card." << endl;
            } else {
                playerBCards += 2;
                numberDeckRemaining -= 2;
                cout << ">>> Player B draws 2 Number Cards." << endl;
            }
            consecutiveWinsA = 0;
        }
        
        if (consecutiveWinsB >= 2) {
            cout << "\n>>> Player B won 2 consecutive rounds!" << endl;
            cout << "Choose: (1) Draw 1 Action Card OR (2) Player A draws 2 Number Cards: ";
            int choice;
            cin >> choice;
            
            if (choice == 1) {
                playerBActionCards++;
                actionDeckRemaining--;
                cout << ">>> Player B draws 1 Action Card." << endl;
            } else {
                playerACards += 2;
                numberDeckRemaining -= 2;
                cout << ">>> Player A draws 2 Number Cards." << endl;
            }
            consecutiveWinsB = 0;
        }
    }

    void checkWinCondition() {
        if (playerACards == 0) {
            cout << "\n>>> Player A has 0 cards! Checking for +2/+4 challenge..." << endl;
            
            string challenge;
            cout << "Does Player B have a +2 or +4 to challenge? (Y/N): ";
            cin >> challenge;
            challenge = toUpper(challenge);
            
            if (challenge == "Y") {
                string isOnlyCard;
                cout << "Is this Player B's ONLY card? (Y/N): ";
                cin >> isOnlyCard;
                isOnlyCard = toUpper(isOnlyCard);
                
                if (isOnlyCard == "Y") {
                    cout << ">>> Player B cannot play their only +2/+4 card!" << endl;
                    cout << ">>> Player B draws 1 Number Card as penalty." << endl;
                    playerBCards++;
                    numberDeckRemaining--;
                } else {
                    string cardType;
                    cout << "Enter card type (+2/+4): ";
                    cin >> cardType;
                    cardType = toUpper(cardType);
                    
                    int drawAmount = (cardType == "+2") ? 2 : 4;
                    cout << ">>> Player B plays " << cardType << "! Player A draws " << drawAmount << " cards." << endl;
                    playerACards += drawAmount;
                    playerBActionCards--;
                    numberDeckRemaining -= drawAmount;
                }
            } else {
                cout << "\n" << string(60, '*') << endl;
                cout << "          PLAYER A WINS THE GAME!" << endl;
                cout << string(60, '*') << "\n" << endl;
                gameOver = true;
                winner = "A";
            }
        }
        
        if (playerBCards == 0) {
            cout << "\n>>> Player B has 0 cards! Checking for +2/+4 challenge..." << endl;
            
            string challenge;
            cout << "Does Player A have a +2 or +4 to challenge? (Y/N): ";
            cin >> challenge;
            challenge = toUpper(challenge);
            
            if (challenge == "Y") {
                string isOnlyCard;
                cout << "Is this Player A's ONLY card? (Y/N): ";
                cin >> isOnlyCard;
                isOnlyCard = toUpper(isOnlyCard);
                
                if (isOnlyCard == "Y") {
                    cout << ">>> Player A cannot play their only +2/+4 card!" << endl;
                    cout << ">>> Player A draws 1 Number Card as penalty." << endl;
                    playerACards++;
                    numberDeckRemaining--;
                } else {
                    string cardType;
                    cout << "Enter card type (+2/+4): ";
                    cin >> cardType;
                    cardType = toUpper(cardType);
                    
                    int drawAmount = (cardType == "+2") ? 2 : 4;
                    cout << ">>> Player A plays " << cardType << "! Player B draws " << drawAmount << " cards." << endl;
                    playerBCards += drawAmount;
                    playerAActionCards--;
                    numberDeckRemaining -= drawAmount;
                }
            } else {
                cout << "\n" << string(60, '*') << endl;
                cout << "          PLAYER B WINS THE GAME!" << endl;
                cout << string(60, '*') << "\n" << endl;
                gameOver = true;
                winner = "B";
            }
        }
    }

    void run() {
        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════════╗\n";
        cout << "║          SPLIT UNO ARBITER - GAME TRACKER                  ║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n";
        cout << "\nInitializing game with 20 cards each...\n";
        
        displayGameState();
        
        while (!gameOver) {
            cout << "\n--- NEW ROUND ---" << endl;
            cout << "Select action:" << endl;
            cout << "  1. Play Number Card Round" << endl;
            cout << "  2. Play Action Card" << endl;
            cout << "  3. Display Game State" << endl;
            cout << "  4. Manual Adjustment" << endl;
            cout << "  5. End Game" << endl;
            cout << "Choice: ";
            
            int choice;
            cin >> choice;
            
            switch (choice) {
                case 1:
                    handleNumberRound();
                    break;
                case 2:
                    handleActionCard();
                    break;
                case 3:
                    displayGameState();
                    break;
                case 4:
                    manualAdjustment();
                    break;
                case 5:
                    gameOver = true;
                    cout << "\n>>> Game ended by arbiter." << endl;
                    break;
                default:
                    cout << "Invalid choice. Try again." << endl;
            }
            
            if (!gameOver && (choice == 1 || choice == 2)) {
                displayGameState();
            }
        }
        
        cout << "\nFinal Game State:" << endl;
        displayGameState();
        
        if (!winner.empty()) {
            cout << "\n🏆 WINNER: Player " << winner << " 🏆\n" << endl;
        }
    }

    void manualAdjustment() {
        cout << "\n--- Manual Adjustment ---" << endl;
        cout << "1. Adjust Player A Number Cards" << endl;
        cout << "2. Adjust Player B Number Cards" << endl;
        cout << "3. Adjust Player A Action Cards" << endl;
        cout << "4. Adjust Player B Action Cards" << endl;
        cout << "5. Reset Consecutive Wins" << endl;
        cout << "Choice: ";
        
        int choice;
        cin >> choice;
        
        int newValue;
        switch (choice) {
            case 1:
                cout << "Enter new count for Player A Number Cards: ";
                cin >> newValue;
                playerACards = newValue;
                break;
            case 2:
                cout << "Enter new count for Player B Number Cards: ";
                cin >> newValue;
                playerBCards = newValue;
                break;
            case 3:
                cout << "Enter new count for Player A Action Cards: ";
                cin >> newValue;
                playerAActionCards = newValue;
                break;
            case 4:
                cout << "Enter new count for Player B Action Cards: ";
                cin >> newValue;
                playerBActionCards = newValue;
                break;
            case 5:
                consecutiveWinsA = 0;
                consecutiveWinsB = 0;
                cout << ">>> Consecutive wins reset." << endl;
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    }
};

int main() {
    SplitUnoArbiter arbiter;
    arbiter.run();
    return 0;
}