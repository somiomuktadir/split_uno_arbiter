/*******************************************************************************
 * SPLIT UNO - ARBITER APPLICATION
 * 
 * A game arbiter/tracker for Split UNO, a custom variant of the classic UNO
 * card game that separates number cards and action cards into distinct decks.
 * 
 * Author: Muktadir Somio
 * Version: 2.0 (Improved)
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

using namespace std;

/*******************************************************************************
 * ENUMERATIONS
 ******************************************************************************/

// Card types in Split UNO
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

// Card colors
enum class Color {
    RED, YELLOW, GREEN, BLUE, WILD
};

/*******************************************************************************
 * MAIN ARBITER CLASS
 ******************************************************************************/

class SplitUnoArbiter {
private:
    // Game Constants
    static constexpr int INITIAL_CARDS = 20;              // Starting number cards per player
    static constexpr int INITIAL_NUMBER_DECK = 68;        // Remaining number cards (108 - 40)
    static constexpr int INITIAL_ACTION_DECK = 32;        // Action cards available
    static constexpr int CONSECUTIVE_WINS_THRESHOLD = 2;  // Wins needed for bonus
    static constexpr int MAX_CARD_NUMBER = 9;             // Highest number card
    static constexpr int MIN_CARD_NUMBER = 0;             // Lowest number card
    static constexpr int CARD_0_DRAW = 1;                 // Cards stolen by playing 0
    static constexpr int CARD_7_NUMBER_DRAW = 2;          // Number cards from card 7
    static constexpr int CARD_7_ACTION_DRAW = 1;          // Action cards from card 7
    
    // Player State
    int playerACards;              // Player A's number card count
    int playerBCards;              // Player B's number card count
    int playerAActionCards;        // Player A's action card count
    int playerBActionCards;        // Player B's action card count
    int consecutiveWinsA;          // Player A's consecutive wins
    int consecutiveWinsB;          // Player B's consecutive wins
    string lastWinner;             // Last round winner ("A" or "B")
    bool playerABlocked;           // Is Player A blocked next round?
    bool playerBBlocked;           // Is Player B blocked next round?
    
    // Deck State
    int numberDeckRemaining;       // Remaining number cards in deck
    int actionDeckRemaining;       // Remaining action cards in deck
    
    // Game State
    bool gameOver;                 // Has the game ended?
    string winner;                 // Game winner ("A" or "B")

    /***************************************************************************
     * INPUT VALIDATION HELPERS
     ***************************************************************************/
    
    /**
     * Clears the input buffer after a failed cin operation
     */
    void clearInputBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    
    /**
     * Gets a validated integer input within a specified range
     * 
     * @param prompt The message to display to the user
     * @param min Minimum acceptable value (inclusive)
     * @param max Maximum acceptable value (inclusive)
     * @return A valid integer within [min, max]
     */
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
    
    /**
     * Gets a validated string input from a set of valid options
     * 
     * @param prompt The message to display to the user
     * @param validOptions Vector of acceptable responses (case-insensitive)
     * @return A valid option in uppercase
     */
    string getValidatedString(const string& prompt, const vector<string>& validOptions) {
        string input;
        while (true) {
            cout << prompt;
            if (cin >> input) {
                input = toUpper(input);
                
                // Check if input matches any valid option
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
    
    /**
     * Converts a string to uppercase
     * 
     * @param s The string to convert
     * @return The uppercase version of the string
     */
    string toUpper(string s) const {
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }
    
    /**
     * Safely draws cards from the number deck with exhaustion check
     * 
     * @param amount Number of cards to draw
     * @return Actual number of cards drawn (may be less if deck exhausted)
     */
    int drawFromNumberDeck(int amount) {
        if (numberDeckRemaining <= 0) {
            cout << ">>> WARNING: Number deck is exhausted! No cards drawn.\n";
            return 0;
        }
        
        int actualDraw = min(amount, numberDeckRemaining);
        if (actualDraw < amount) {
            cout << ">>> WARNING: Only " << actualDraw << " cards available in number deck.\n";
        }
        
        numberDeckRemaining -= actualDraw;
        return actualDraw;
    }
    
    /**
     * Safely draws cards from the action deck with exhaustion check
     * 
     * @param amount Number of cards to draw
     * @return Actual number of cards drawn (may be less if deck exhausted)
     */
    int drawFromActionDeck(int amount) {
        if (actionDeckRemaining <= 0) {
            cout << ">>> WARNING: Action deck is exhausted! No cards drawn.\n";
            return 0;
        }
        
        int actualDraw = min(amount, actionDeckRemaining);
        if (actualDraw < amount) {
            cout << ">>> WARNING: Only " << actualDraw << " cards available in action deck.\n";
        }
        
        actionDeckRemaining -= actualDraw;
        return actualDraw;
    }

    /***************************************************************************
     * GAME STATE DISPLAY
     ***************************************************************************/
    
    /**
     * Displays the current game state in a formatted table
     */
    void displayGameState() const {
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

    /***************************************************************************
     * NUMBER CARD ROUND HANDLERS
     ***************************************************************************/
    
    /**
     * Processes special effects of number cards 0 and 7
     * 
     * Card 0: The player who plays it draws 1 random card from opponent
     * Card 7: Opponent draws 2 number cards and 1 action card
     * 
     * @param numA Player A's card number
     * @param numB Player B's card number
     */
    void processNumberCardEffects(int numA, int numB) {
        // Handle card 0 effect (steal from opponent)
        if (numA == 0 && playerBCards > 0) {
            cout << "\n>>> Player A played 0! A draws 1 random card from B's hand." << endl;
            playerACards++;
            playerBCards--;
        }
        if (numB == 0 && playerACards > 0) {
            cout << "\n>>> Player B played 0! B draws 1 random card from A's hand." << endl;
            playerBCards++;
            playerACards--;
        }

        // Handle card 7 effect (opponent draws penalty cards)
        if (numA == 7) {
            cout << "\n>>> Player A played 7! B must draw 2 Number Cards and 1 Action Card." << endl;
            int numberDrawn = drawFromNumberDeck(CARD_7_NUMBER_DRAW);
            int actionDrawn = drawFromActionDeck(CARD_7_ACTION_DRAW);
            playerBCards += numberDrawn;
            playerBActionCards += actionDrawn;
        }
        if (numB == 7) {
            cout << "\n>>> Player B played 7! A must draw 2 Number Cards and 1 Action Card." << endl;
            int numberDrawn = drawFromNumberDeck(CARD_7_NUMBER_DRAW);
            int actionDrawn = drawFromActionDeck(CARD_7_ACTION_DRAW);
            playerACards += numberDrawn;
            playerAActionCards += actionDrawn;
        }
    }
    
    /**
     * Resolves the number card bid and updates player states
     * 
     * @param numA Player A's card number
     * @param numB Player B's card number
     */
    void resolveNumberBid(int numA, int numB) {
        if (numA > numB) {
            // Player A wins
            cout << "\n>>> Player A WINS the round! (" << numA << " > " << numB << ")" << endl;
            playerACards = max(0, playerACards - 1);  // Shed card (prevent negative)
            int drawn = drawFromNumberDeck(1);
            playerBCards += drawn;  // Draw penalty
            consecutiveWinsA++;
            consecutiveWinsB = 0;
            lastWinner = "A";
        } else if (numB > numA) {
            // Player B wins
            cout << "\n>>> Player B WINS the round! (" << numB << " > " << numA << ")" << endl;
            playerBCards = max(0, playerBCards - 1);  // Shed card (prevent negative)
            int drawn = drawFromNumberDeck(1);
            playerACards += drawn;  // Draw penalty
            consecutiveWinsB++;
            consecutiveWinsA = 0;
            lastWinner = "B";
        } else {
            // Tie - both shed and draw
            cout << "\n>>> TIE! Both players shed their cards and draw 1 card." << endl;
            playerACards = max(0, playerACards - 1);
            playerBCards = max(0, playerBCards - 1);
            int drawnA = drawFromNumberDeck(1);
            int drawnB = drawFromNumberDeck(1);
            playerACards += drawnA;
            playerBCards += drawnB;
            consecutiveWinsA = 0;
            consecutiveWinsB = 0;
        }
    }
    
    /**
     * Handles a standard number card round where both players play
     */
    void handleNumberRound() {
        // Check for blocked players
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

        // Get card inputs with validation
        int numA = getValidatedInt("Enter Player A's card (0-9): ", MIN_CARD_NUMBER, MAX_CARD_NUMBER);
        int numB = getValidatedInt("Enter Player B's card (0-9): ", MIN_CARD_NUMBER, MAX_CARD_NUMBER);

        // Process special card effects first
        processNumberCardEffects(numA, numB);

        // Resolve the bid
        resolveNumberBid(numA, numB);

        // Check for consecutive win bonuses and win conditions
        checkConsecutiveWins();
        checkWinCondition();
    }
    
    /**
     * Handles a round where one player is blocked
     * 
     * @param activePlayer The player who is NOT blocked ('A' or 'B')
     */
    void handleBlockedRound(char activePlayer) {
        [[maybe_unused]] int card = getValidatedInt(
            string("Enter Player ") + activePlayer + "'s card (0-9): ",
            MIN_CARD_NUMBER, MAX_CARD_NUMBER
        );
        
        if (activePlayer == 'A') {
            cout << "\n>>> Player A sheds their card. Player B draws 1 card (penalty)." << endl;
            playerACards = max(0, playerACards - 1);
            int drawn = drawFromNumberDeck(1);
            playerBCards += drawn;
            consecutiveWinsA++;
            consecutiveWinsB = 0;
            lastWinner = "A";
        } else {
            cout << "\n>>> Player B sheds their card. Player A draws 1 card (penalty)." << endl;
            playerBCards = max(0, playerBCards - 1);
            int drawn = drawFromNumberDeck(1);
            playerACards += drawn;
            consecutiveWinsB++;
            consecutiveWinsA = 0;
            lastWinner = "B";
        }
        
        checkConsecutiveWins();
        checkWinCondition();
    }

    /***************************************************************************
     * ACTION CARD HANDLERS
     ***************************************************************************/
    
    /**
     * Routes action card handling to the appropriate handler function
     */
    void handleActionCard() {
        string player = getValidatedString(
            "Which player is playing an action card (A/B)? ",
            {"A", "B"}
        );
        
        string action = getValidatedString(
            "Enter action card type (BLOCK/REVERSE/COLOR/+2/+4/TRUTH/DARE): ",
            {"BLOCK", "SKIP", "REVERSE", "COLOR", "WILD", "+2", "+4", "TRUTH", "DARE"}
        );

        // Route to appropriate handler
        if (action == "BLOCK" || action == "SKIP") {
            handleBlockCard(player);
        } else if (action == "REVERSE") {
            handleReverseCard(player);
        } else if (action == "COLOR" || action == "WILD") {
            handleColorChangeCard(player);
        } else if (action == "+2") {
            handleDrawCard(player, 2);
        } else if (action == "+4") {
            handleDrawCard(player, 4);
        } else if (action == "TRUTH") {
            handleTruthCard(player);
        } else if (action == "DARE") {
            handleDareCard(player);
        }
    }
    
    /**
     * Handles BLOCK/SKIP card effect
     * Blocks opponent from next round unless they also play BLOCK
     * 
     * @param player The player who played the BLOCK card ("A" or "B")
     */
    void handleBlockCard(const string& player) {
        cout << "\n>>> " << player << " plays BLOCK card!" << endl;
        
        string opponent = getValidatedString(
            "Did the opponent also play BLOCK? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );
        
        if (opponent == "Y" || opponent == "YES") {
            // Both played BLOCK - effects cancel
            cout << ">>> Both played BLOCK! Effects cancel. Both shed 1 Number Card." << endl;
            playerACards = max(0, playerACards - 1);
            playerBCards = max(0, playerBCards - 1);
            playerAActionCards = max(0, playerAActionCards - 1);
            playerBActionCards = max(0, playerBActionCards - 1);
        } else {
            // Only one player played BLOCK - opponent is blocked
            if (player == "A") {
                cout << ">>> Player B is BLOCKED for next round!" << endl;
                playerBBlocked = true;
                playerAActionCards = max(0, playerAActionCards - 1);
            } else {
                cout << ">>> Player A is BLOCKED for next round!" << endl;
                playerABlocked = true;
                playerBActionCards = max(0, playerBActionCards - 1);
            }
        }
    }
    
    /**
     * Handles REVERSE card effect
     * Players exchange their entire hands (number and action cards)
     * 
     * @param player The player who played the REVERSE card ("A" or "B")
     */
    void handleReverseCard(const string& player) {
        cout << "\n>>> " << player << " plays REVERSE card!" << endl;
        cout << ">>> Players exchange their entire hands!" << endl;
        
        swap(playerACards, playerBCards);
        swap(playerAActionCards, playerBActionCards);
        
        // Remove the REVERSE card from the player who played it
        if (player == "A") {
            playerAActionCards = max(0, playerAActionCards - 1);
        } else {
            playerBActionCards = max(0, playerBActionCards - 1);
        }
    }
    
    /**
     * Handles COLOR CHANGE/WILD card effect
     * Both players shed 1 number card, player chooses color for opponent
     * 
     * @param player The player who played the COLOR CHANGE card ("A" or "B")
     */
    void handleColorChangeCard(const string& player) {
        cout << "\n>>> " << player << " plays COLOR CHANGE card!" << endl;
        cout << ">>> Both players shed 1 Number Card each." << endl;
        
        playerACards = max(0, playerACards - 1);
        playerBCards = max(0, playerBCards - 1);
        
        string color = getValidatedString(
            "Enter the color chosen for opponent's next play (R/Y/G/B): ",
            {"R", "Y", "G", "B", "RED", "YELLOW", "GREEN", "BLUE"}
        );
        cout << ">>> Opponent must play " << color << " in the next round." << endl;
        
        if (player == "A") {
            playerAActionCards = max(0, playerAActionCards - 1);
        } else {
            playerBActionCards = max(0, playerBActionCards - 1);
        }
    }
    
    /**
     * Handles +2 or +4 draw cards (unified implementation)
     * Opponent can counter with their own +2/+4, otherwise draws penalty
     * 
     * @param player The player who played the draw card ("A" or "B")
     * @param drawAmount The number of cards to draw (2 or 4)
     */
    void handleDrawCard(const string& player, int drawAmount) {
        cout << "\n>>> " << player << " plays +" << drawAmount << " card!" << endl;
        
        string hasCounter = getValidatedString(
            "Did opponent play an action card? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );
        
        if (hasCounter == "Y" || hasCounter == "YES") {
            // Opponent countered with their own draw card
            string oppCard = getValidatedString(
                "Enter opponent's action card (+2/+4): ",
                {"+2", "+4"}
            );
            
            int oppDraw = (oppCard == "+2") ? 2 : 4;
            
            if (drawAmount != oppDraw) {
                // Different values - higher wins, lower draws difference + 1
                int difference = abs(drawAmount - oppDraw);
                int loserDraw = 1 + difference;  // 1 for losing + difference
                
                if (drawAmount > oppDraw) {
                    // Player wins the counter
                    cout << ">>> Both shed their action cards. Opponent draws " 
                         << loserDraw << " cards (1 loss + " << difference << " difference)." << endl;
                    int drawn = drawFromNumberDeck(loserDraw);
                    
                    if (player == "A") {
                        playerAActionCards = max(0, playerAActionCards - 1);
                        playerBActionCards = max(0, playerBActionCards - 1);
                        playerBCards += drawn;
                    } else {
                        playerBActionCards = max(0, playerBActionCards - 1);
                        playerAActionCards = max(0, playerAActionCards - 1);
                        playerACards += drawn;
                    }
                } else {
                    // Opponent wins the counter
                    cout << ">>> Both shed their action cards. " << player 
                         << " draws " << loserDraw << " cards (1 loss + " << difference << " difference)." << endl;
                    int drawn = drawFromNumberDeck(loserDraw);
                    
                    if (player == "A") {
                        playerAActionCards = max(0, playerAActionCards - 1);
                        playerBActionCards = max(0, playerBActionCards - 1);
                        playerACards += drawn;
                    } else {
                        playerBActionCards = max(0, playerBActionCards - 1);
                        playerAActionCards = max(0, playerAActionCards - 1);
                        playerBCards += drawn;
                    }
                }
            } else {
                // Same values - both shed and draw 1
                cout << ">>> Both shed action cards and draw 1 Number Card each." << endl;
                playerAActionCards = max(0, playerAActionCards - 1);
                playerBActionCards = max(0, playerBActionCards - 1);
                int drawnA = drawFromNumberDeck(1);
                int drawnB = drawFromNumberDeck(1);
                playerACards += drawnA;
                playerBCards += drawnB;
            }
        } else {
            // No counter - opponent draws full penalty
            int drawn = drawFromNumberDeck(drawAmount);
            
            if (player == "A") {
                cout << ">>> Player B draws " << drawAmount << " cards!" << endl;
                playerAActionCards = max(0, playerAActionCards - 1);
                playerBCards += drawn;
            } else {
                cout << ">>> Player A draws " << drawAmount << " cards!" << endl;
                playerBActionCards = max(0, playerBActionCards - 1);
                playerACards += drawn;
            }
        }
    }
    
    /**
     * Handles TRUTH card effect
     * Opponent must answer truth question or face penalty
     * Player who played TRUTH sheds 1 number card regardless
     * 
     * @param player The player who played the TRUTH card ("A" or "B")
     */
    void handleTruthCard(const string& player) {
        cout << "\n>>> " << player << " plays TRUTH card!" << endl;
        
        string response = getValidatedString(
            "Did opponent answer the truth question? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );
        
        if (response == "N" || response == "NO") {
            // Opponent refused - apply penalty
            int choice = getValidatedInt(
                "Choose penalty:\n"
                "  (1) Take 2 action cards, opponent draws 2 number cards\n"
                "  (2) Opponent draws 5 number cards\n"
                "Choice: ", 
                1, 2
            );
            
            if (choice == 1) {
                // Option 1: Player gains 2 action cards, opponent draws 2 number cards
                int actionDrawn = drawFromActionDeck(2);
                int numberDrawn = drawFromNumberDeck(2);
                
                if (player == "A") {
                    playerAActionCards += actionDrawn;
                    playerBCards += numberDrawn;
                } else {
                    playerBActionCards += actionDrawn;
                    playerACards += numberDrawn;
                }
            } else {
                // Option 2: Opponent draws 5 number cards
                int drawn = drawFromNumberDeck(5);
                
                if (player == "A") {
                    playerBCards += drawn;
                } else {
                    playerACards += drawn;
                }
            }
        }
        
        // Player who played TRUTH sheds 1 action card and 1 number card
        if (player == "A") {
            playerAActionCards = max(0, playerAActionCards - 1);
            playerACards = max(0, playerACards - 1);
        } else {
            playerBActionCards = max(0, playerBActionCards - 1);
            playerBCards = max(0, playerBCards - 1);
        }
    }
    
    /**
     * Handles DARE card effect
     * Opponent must complete dare or forfeit the entire game
     * 
     * @param player The player who played the DARE card ("A" or "B")
     */
    void handleDareCard(const string& player) {
        cout << "\n>>> " << player << " plays DARE card!" << endl;
        
        string response = getValidatedString(
            "Did opponent complete the dare? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );
        
        if (response == "N" || response == "NO") {
            // Opponent forfeits - game over
            cout << ">>> Opponent forfeits! " << player << " WINS THE GAME!" << endl;
            gameOver = true;
            winner = player;
        } else {
            // Dare completed - player sheds cards
            if (player == "A") {
                playerAActionCards = max(0, playerAActionCards - 1);
                playerACards = max(0, playerACards - 1);
            } else {
                playerBActionCards = max(0, playerBActionCards - 1);
                playerBCards = max(0, playerBCards - 1);
            }
        }
    }

    /***************************************************************************
     * GAME FLOW LOGIC
     ***************************************************************************/
    
    /**
     * Checks and handles consecutive win bonuses
     * After 2 consecutive wins, player chooses:
     *   (1) Draw 1 action card, or
     *   (2) Opponent draws 2 number cards
     */
    void checkConsecutiveWins() {
        if (consecutiveWinsA >= CONSECUTIVE_WINS_THRESHOLD) {
            cout << "\n>>> Player A won " << CONSECUTIVE_WINS_THRESHOLD << " consecutive rounds!" << endl;
            
            int choice = getValidatedInt(
                "Choose: (1) Draw 1 Action Card OR (2) Player B draws 2 Number Cards: ",
                1, 2
            );
            
            if (choice == 1) {
                int drawn = drawFromActionDeck(1);
                playerAActionCards += drawn;
                cout << ">>> Player A draws 1 Action Card." << endl;
            } else {
                int drawn = drawFromNumberDeck(2);
                playerBCards += drawn;
                cout << ">>> Player B draws 2 Number Cards." << endl;
            }
            consecutiveWinsA = 0;
        }
        
        if (consecutiveWinsB >= CONSECUTIVE_WINS_THRESHOLD) {
            cout << "\n>>> Player B won " << CONSECUTIVE_WINS_THRESHOLD << " consecutive rounds!" << endl;
            
            int choice = getValidatedInt(
                "Choose: (1) Draw 1 Action Card OR (2) Player A draws 2 Number Cards: ",
                1, 2
            );
            
            if (choice == 1) {
                int drawn = drawFromActionDeck(1);
                playerBActionCards += drawn;
                cout << ">>> Player B draws 1 Action Card." << endl;
            } else {
                int drawn = drawFromNumberDeck(2);
                playerACards += drawn;
                cout << ">>> Player A draws 2 Number Cards." << endl;
            }
            consecutiveWinsB = 0;
        }
    }
    
    /**
     * Handles the +2/+4 challenge when a player reaches 0 cards
     * 
     * @param winningPlayer The player who reached 0 cards ('A' or 'B')
     * @param challengingPlayer The opponent who might challenge ('A' or 'B')
     */
    void handleDrawChallenge(char winningPlayer, char challengingPlayer) {
        string challenge = getValidatedString(
            string("Does Player ") + challengingPlayer + " have a +2 or +4 to challenge? (Y/N): ",
            {"Y", "N", "YES", "NO"}
        );
        
        if (challenge == "Y" || challenge == "YES") {
            string isOnlyCard = getValidatedString(
                string("Is this Player ") + challengingPlayer + "'s ONLY card? (Y/N): ",
                {"Y", "N", "YES", "NO"}
            );
            
            if (isOnlyCard == "Y" || isOnlyCard == "YES") {
                // Cannot play their only card
                cout << ">>> Player " << challengingPlayer << " cannot play their only +2/+4 card!" << endl;
                cout << ">>> Player " << challengingPlayer << " draws 1 Number Card as penalty." << endl;
                
                int drawn = drawFromNumberDeck(1);
                if (challengingPlayer == 'A') {
                    playerACards += drawn;
                } else {
                    playerBCards += drawn;
                }
            } else {
                // Valid challenge
                string cardType = getValidatedString(
                    "Enter card type (+2/+4): ",
                    {"+2", "+4"}
                );
                
                int drawAmount = (cardType == "+2") ? 2 : 4;
                int drawn = drawFromNumberDeck(drawAmount);
                
                cout << ">>> Player " << challengingPlayer << " plays " << cardType 
                     << "! Player " << winningPlayer << " draws " << drawAmount << " cards." << endl;
                
                if (winningPlayer == 'A') {
                    playerACards += drawn;
                } else {
                    playerBCards += drawn;
                }
                
                if (challengingPlayer == 'A') {
                    playerAActionCards = max(0, playerAActionCards - 1);
                } else {
                    playerBActionCards = max(0, playerBActionCards - 1);
                }
            }
        } else {
            // No challenge - player wins
            cout << "\n" << string(60, '*') << endl;
            cout << "          PLAYER " << winningPlayer << " WINS THE GAME!" << endl;
            cout << string(60, '*') << "\n" << endl;
            gameOver = true;
            winner = string(1, winningPlayer);
        }
    }
    
    /**
     * Checks if either player has won the game (reached 0 number cards)
     * Handles the +2/+4 challenge mechanic for last-turn defense
     */
    void checkWinCondition() {
        if (playerACards == 0) {
            cout << "\n>>> Player A has 0 cards! Checking for +2/+4 challenge..." << endl;
            handleDrawChallenge('A', 'B');
        }
        
        if (playerBCards == 0) {
            cout << "\n>>> Player B has 0 cards! Checking for +2/+4 challenge..." << endl;
            handleDrawChallenge('B', 'A');
        }
    }
    
    /**
     * Allows manual adjustment of game state for arbiter corrections
     */
    void manualAdjustment() {
        cout << "\n--- Manual Adjustment ---" << endl;
        cout << "1. Adjust Player A Number Cards" << endl;
        cout << "2. Adjust Player B Number Cards" << endl;
        cout << "3. Adjust Player A Action Cards" << endl;
        cout << "4. Adjust Player B Action Cards" << endl;
        cout << "5. Reset Consecutive Wins" << endl;
        
        int choice = getValidatedInt("Choice: ", 1, 5);
        
        int newValue;
        switch (choice) {
            case 1:
                newValue = getValidatedInt("Enter new count for Player A Number Cards: ", 0, 100);
                playerACards = newValue;
                break;
            case 2:
                newValue = getValidatedInt("Enter new count for Player B Number Cards: ", 0, 100);
                playerBCards = newValue;
                break;
            case 3:
                newValue = getValidatedInt("Enter new count for Player A Action Cards: ", 0, 50);
                playerAActionCards = newValue;
                break;
            case 4:
                newValue = getValidatedInt("Enter new count for Player B Action Cards: ", 0, 50);
                playerBActionCards = newValue;
                break;
            case 5:
                consecutiveWinsA = 0;
                consecutiveWinsB = 0;
                cout << ">>> Consecutive wins reset." << endl;
                break;
        }
    }

public:
    /**
     * Constructor - Initializes game state
     */
    SplitUnoArbiter() {
        playerACards = INITIAL_CARDS;
        playerBCards = INITIAL_CARDS;
        playerAActionCards = 0;
        playerBActionCards = 0;
        consecutiveWinsA = 0;
        consecutiveWinsB = 0;
        lastWinner = "";
        playerABlocked = false;
        playerBBlocked = false;
        numberDeckRemaining = INITIAL_NUMBER_DECK;
        actionDeckRemaining = INITIAL_ACTION_DECK;
        gameOver = false;
        winner = "";
    }
    
    /**
     * Main game loop - Handles user input and game flow
     */
    void run() {
        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════════╗\n";
        cout << "║          SPLIT UNO ARBITER - GAME TRACKER                  ║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n";
        cout << "\nInitializing game with " << INITIAL_CARDS << " cards each...\n";
        
        displayGameState();
        
        while (!gameOver) {
            cout << "\n--- NEW ROUND ---" << endl;
            cout << "Select action:" << endl;
            cout << "  1. Play Number Card Round" << endl;
            cout << "  2. Play Action Card" << endl;
            cout << "  3. Display Game State" << endl;
            cout << "  4. Manual Adjustment" << endl;
            cout << "  5. End Game" << endl;
            
            int choice = getValidatedInt("Choice: ", 1, 5);
            
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
            }
            
            // Display state after gameplay actions
            if (!gameOver && (choice == 1 || choice == 2)) {
                displayGameState();
            }
        }
        
        // Final state display
        cout << "\nFinal Game State:" << endl;
        displayGameState();
        
        if (!winner.empty()) {
            cout << "\n🏆 WINNER: Player " << winner << " 🏆\n" << endl;
        }
    }
};

/*******************************************************************************
 * MAIN ENTRY POINT
 ******************************************************************************/

int main() {
    SplitUnoArbiter arbiter;
    arbiter.run();
    return 0;
}