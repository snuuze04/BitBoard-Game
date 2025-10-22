#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Phase 1*/
unsigned int SetBit(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value | (1u << position);
}
unsigned int ClearBit(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value & ~(1u << position);
}
unsigned int ToggleBit(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value ^ (1u << position);
}
int GetBit(unsigned int value, int position) {
    if (position < 0 || position >= 32) return 0;
    return (value >> position) & 1u;
}

int CountBits(unsigned int value) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if ((value >> i) & 1u) {
            count++;
        }
    }
    return count;
}

unsigned int ShiftLeft(unsigned int value, int positions) {
    if (positions < 0 || positions >= 32) return 0u;
    return value << positions;
}
unsigned int ShiftRight(unsigned int value, int positions) {
    if (positions < 0 || positions >= 32) return 0u;
    return value >> positions;
}

void PrintBinary(unsigned int value) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}
void PrintHex(unsigned int value) {
    printf("0x%08X\n", value);
}

/* Phase 2 */
uint64_t SetBit64(uint64_t value, int position) {
    if (position < 0 || position >= 64) return value;
    return value | (1ULL << position);
}
uint64_t ClearBit64(uint64_t value, int position) {
    if (position < 0 || position >= 64) return value;
    return value & ~(1ULL << position);
}
uint64_t ToggleBit64(uint64_t value, int position) {
    if (position < 0 || position >= 64) return value;
    return value ^ (1ULL << position);
}
int GetBit64(uint64_t value, int position) {
    if (position < 0 || position >= 64) return 0;
    return (value >> position) & 1ULL;
}

int CountBits64(uint64_t value) {
    int count = 0;
    for (int i = 0; i < 64; i++) {
        if ((value >> i) & 1ULL) {
            count++;
        }
    }
    return count;
}

uint64_t ShiftLeft64(uint64_t value, int positions) {
    if (positions < 0 || positions >= 64) return 0ULL;
    return value << positions;
}
uint64_t ShiftRight64(uint64_t value, int positions) {
    if (positions < 0 || positions >= 64) return 0ULL;
    return value >> positions;
}

void PrintBinary64(uint64_t value) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (value >> i) & 1ULL);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}
void PrintHex64(uint64_t value) {
    printf("0x%016llX\n", (unsigned long long)value);
}

typedef struct {
    uint64_t red;
    uint64_t red_kings;
    uint64_t black;
    uint64_t black_kings;
    int current_turn; // red = 0, black = 1
} GameState;

// diagonal piece movement directions
const int NE_DIRECTION = -7;
const int NW_DIRECTION = -9;
const int SE_DIRECTION = 9;
const int SW_DIRECTION = 7;

// checks if the position is valid
int OnBoard(int position) {
    return position >= 0 && position < 64;
}

// get the row and column
int GetRow(int position) {
    return position / 8;
}
int GetCol(int position) {
    return position % 8;
}

// checks to see if the square is a black square
int IsPlayableSquare(int position) {
    if (!OnBoard(position)) return 0;
    return ((GetRow(position) + GetCol(position)) & 1) == 1;
}

// checks to see if square is occupied by any pieces
uint64_t Occupancy(GameState* g) {
    return g->red | g->red_kings | g->black | g->black_kings;
}

void InitGame(GameState* g) {
    g->red = g->red_kings = g->black = g->black_kings = 0ULL;
    g->current_turn = 1; // black moves first

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int position = row * 8 + col;
            if (!IsPlayableSquare(position)) continue;
            if (row <= 2) {
                g->black = SetBit64(g->black, position);
            } else if (row >= 5) {
                g->red = SetBit64(g->red, position);
            }
        }
    }
}

void PrintRefBoard() {
    printf("This board is reference for choosing squares to move to (0 - 63): \n\n");
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int position = (row * 8) + col;
            printf("%2d ", position);
        }
        printf("\n");
    }
    printf("\n");
}

void PrintPlayBoard(GameState* g) {
    // board legend
    printf("\nBoard Legend:\n");
    printf("# - unoccupied light square\n");
    printf(". - unoccupied dark square\n");
    printf("r - red piece\n");
    printf("R - red king\n");
    printf("b - black piece\n");
    printf("B - black king\n\n");

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int pos = (row * 8) + col;
            char ch = '.';
            if (!IsPlayableSquare(pos)) ch = '#';
            else if ((g->red >> pos) & 1ULL) ch = 'r';
            else if ((g->red_kings >> pos) & 1ULL) ch = 'R';
            else if ((g->black >> pos) & 1ULL) ch = 'b';
            else if ((g->black_kings >> pos) & 1ULL) ch = 'B';
            printf(" %c ", ch);
        }
        printf("  \n");
    }
}

// checks pieces
int IsRed(GameState* g, int position) {
    return ((g->red | g->red_kings) >> position) & 1ULL;
}
int IsBlack(GameState* g, int position) {
    return ((g->black | g->black_kings) >> position) & 1ULL;
}
int IsKing(GameState* g, int position) {
    return ((g->red_kings | g->black_kings) >> position) & 1ULL;
}

// moving a piece, captures, and regular to king
void MovePiece(GameState* g, int src, int dest, int jumpedPosition) {
    uint64_t *player_pieces, *player_kings;
    uint64_t *opp_pieces, *opp_kings;
    if (g->current_turn == 0) {
        player_pieces = &g->red;
        player_kings = &g->red_kings;
        opp_pieces = &g->black;
        opp_kings = &g->black_kings;
    }
    else {
        player_pieces = &g->black;
        player_kings = &g->black_kings;
        opp_pieces = &g->red;
        opp_kings = &g->red_kings;
    }

    // checks if piece is a king
    int is_king = GetBit64(*player_kings, src);

    // empties the jump off square
    *player_pieces = ClearBit64(*player_pieces, src);
    *player_kings  = ClearBit64(*player_kings, src);

    //TODO make sure jumped position is understood
    // clears jumped opponent
    if (jumpedPosition != -1) {
        *opp_pieces = ClearBit64(*opp_pieces, jumpedPosition);
        *opp_kings  = ClearBit64(*opp_kings, jumpedPosition);
    }

    // places piece at destination and upgrades if applicable
    int make_king = 0;
    int dest_row = GetRow(dest);
    if (is_king) {
        *player_kings = SetBit64(*player_kings, dest);
    } else {
        if (g->current_turn == 0 && dest_row == 0) make_king = 1; // red reaches top
        if (g->current_turn == 1 && dest_row == 7) make_king = 1; // black reaches bottom
        if (make_king) *player_kings = SetBit64(*player_kings, dest);
        else *player_pieces = SetBit64(*player_pieces, dest);
    }
}

// checks if a simple move is valid
int IsValidMove(GameState* g, int src, int dest) {
    if (!OnBoard(src) || !OnBoard(dest)) return 0; // checks that both positions are on the board
    if (!IsPlayableSquare(src) || !IsPlayableSquare(dest)) return 0; // checks that both positions are playable squares
    if (GetBit64(Occupancy(g), dest)) return 0; // checks that destination is empty
    int row_change = GetRow(dest) - GetRow(src);
    int col_change = GetCol(dest) - GetCol(src);
    if (abs(row_change) != 1 || abs(col_change) != 1) return 0; // must move diagonally one

    int is_king_piece = IsKing(g, src);
    if (g->current_turn == 0) {
        if (!is_king_piece && row_change != -1) return 0; // red can only move forward
    } else {
        if (!is_king_piece && row_change != 1) return 0; // black can only move forward
    }
    return 1;
}

// returns jump position if the jump is valid
int JumpedPosition(GameState* g, int src, int dest) {
    if (!OnBoard(src) || !OnBoard(dest)) return -1;
    if (!IsPlayableSquare(src) || !IsPlayableSquare(dest)) return -1;
    if (GetBit64(Occupancy(g), dest)) return -1;

    int row_change = GetRow(dest) - GetRow(src);
    int col_change = GetCol(dest) - GetCol(src);
    if (abs(row_change) != 2 || abs(col_change) != 2) return -1;
    int jumped_row = (GetRow(src) + GetRow(dest)) / 2;
    int jumped_col = (GetCol(src) + GetCol(dest)) / 2;
    int jumped_square = jumped_row * 8 + jumped_col;

    // checks that jumped square contains opponent piece
    if (g->current_turn == 0) {
        if (!IsBlack(g, jumped_square)) return -1;
    } else {
        if (!IsRed(g, jumped_square)) return -1;
    }

    // direction check
    int is_king_piece = IsKing(g, src);
    if (!is_king_piece) {
        if (g->current_turn == 0 && row_change != -2) return -1;
        if (g->current_turn == 1 && row_change != 2) return -1;
    }
    return jumped_square;
}

// finds available captures
int CaptureAvailable(GameState* g) {
    uint64_t current_pieces;
    if (g->current_turn == 0) {
        current_pieces = g->red | g->red_kings;
    }
    else {
        current_pieces = g->black | g->black_kings;
    }

    for (int position = 0; position < 64; ++position) {
        if (!GetBit64(current_pieces, position)) continue;
        int directions[4] = {NW_DIRECTION*2, NE_DIRECTION*2, SW_DIRECTION*2, SE_DIRECTION*2};
        for (int direction = 0; direction < 4; ++direction) {
            int landing = position + directions[direction];
            int jumped = JumpedPosition(g, position, landing);
            if (jumped != -1) return 1;
        }
    }
    return 0;
}

// player wins if opponent has no pieces or no valid moves
int CheckWin(GameState* g) {
    uint64_t all_opp_pieces;
    if (g->current_turn == 0) {
        all_opp_pieces = g->black | g->black_kings;
    } else {
        all_opp_pieces = g->red | g->red_kings;
    }

    if (all_opp_pieces == 0ULL) return 1; // opponent has no pieces

    // check if opponent has any legal moves by temporarily switching turn and scanning
    GameState copy = *g;
    copy.current_turn = 1 - g->current_turn;

    uint64_t movable_opp_pieces;
    if (copy.current_turn == 0) {
        movable_opp_pieces = copy.red | copy.red_kings;
    } else {
        movable_opp_pieces = copy.black | copy.black_kings;
    }

    for (int position = 0; position < 64; ++position) {
        if (!GetBit64(movable_opp_pieces, position)) continue;
        // simple moves
        int possible_row_moves[4] = {-1,-1,1,1};
        int possible_col_moves[4] = {-1,1,-1,1};
        for (int i = 0; i < 4; ++i) {
            int target_row = GetRow(position) + possible_row_moves[i];
            int target_col = GetCol(position) + possible_col_moves[i];
            int target_square = target_row*8 + target_col;
            if (!OnBoard(target_square)) continue;
            if (!IsPlayableSquare(target_square)) continue;
            if (!GetBit64(Occupancy(&copy), target_square) && IsValidMove(&copy, position, target_square)) return 0; // opponent has move
        }

        // captures
        int directions[4] = {NW_DIRECTION*2, NE_DIRECTION*2, SW_DIRECTION*2, SE_DIRECTION*2};
        for (int direction = 0; direction < 4; ++direction) {
            int landing = position + directions[direction];
            if (JumpedPosition(&copy, position, landing) != -1) return 0;
        }
    }
    // opponent has no legal moves
    return 1;
}

// possible jump options
int GetJumps(GameState* g, int src, int possible_landings[], int jumped_squares[]) {
    int count = 0;
    int directions[4] = {NW_DIRECTION*2, NE_DIRECTION*2, SW_DIRECTION*2, SE_DIRECTION*2};
    for (int direction = 0; direction < 4; ++direction) {
        int landing_square = src + directions[direction];
        int jumped_piece = JumpedPosition(g, src, landing_square);
        if (jumped_piece != -1) {
            possible_landings[count] = landing_square;
            jumped_squares[count] = jumped_piece;
            ++count;
        }
    }
    return count;
}

// executes moves using other functions & returns 0 to end the game
int PlayerTurn(GameState* g) {
    PrintPlayBoard(g);
    if (g->current_turn == 0) {
        printf("\nIt's Red's turn.\n\n");
    }
    else {
        printf("\nIt's Black's turn.\n\n");
    }
    printf("Enter two numbers, first the square you want to move FROM,\n");
    printf("then the square you want to move TO.\n\n");
    int src, dest;
    // player must capture if possible
    int must_capture = CaptureAvailable(g);
    if (must_capture) printf("A capture is available — you must capture.\n");


    while (1) {
        printf("Enter your move (two numbers 0-63 separated by a space): ");
        if (scanf("%d %d", &src, &dest) != 2) {
            printf("Invalid input. Please enter two integers 0-63 seperated by a space.\n");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }

        if (!OnBoard(src) || !OnBoard(dest)) {
            printf("\nNumber must between 0 and 63.\n"); continue;
        }
        if (!IsPlayableSquare(src) || !IsPlayableSquare(dest)) {
            printf("\nMoves must be to dark squares only.\n");
            continue;
        }
        // must move your own piece
        if (g->current_turn == 0 && !IsRed(g, src)) {
            printf("\nSquare does not contain your piece (Red).\n");
            continue;
        }
        if (g->current_turn == 1 && !IsBlack(g, src)) {
            printf("\nFrom-square does not contain your piece (Black).\n");
            continue;
        }

        // check for available jumps
        int jumped = JumpedPosition(g, src, dest);
        if (jumped != -1) {
            MovePiece(g, src, dest, jumped);
            // multi-jumps
            int landings[4], jumped_pos[4];
            while (1) {
                int count = GetJumps(g, dest, landings, jumped_pos);

                if (count == 0) break;

                PrintPlayBoard(g);
                printf("Multi-jump available from %d. Enter next landing (or -1 to stop if none): ", dest);

                int next;
                if (scanf("%d", &next) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF); {
                        break;
                    }
                }

                // check if in landings
                    // TODO check on this
                int found = -1;
                for (int i = 0; i < count; ++i) if (landings[i] == next) found = i;
                if (found == -1) { printf("Not a valid landing for additional jump. Stopping multi-jump.\n"); break; }
                MovePiece(g, dest, next, jumped_pos[found]);
                dest = next;
            }
            // check win
            if (CheckWin(g)) { PrintPlayBoard(g); printf("%s wins!\n", g->current_turn == 0 ? "Red" : "Black"); return 0; }
            // switch turn
            g->current_turn = 1 - g->current_turn;
            return 1;
        }

        // reject non capture if a capture is available
        if (must_capture) {
            printf("You must capture.\n");
            continue;
        }
        // reject invalid moves
        if (!IsValidMove(g, src, dest)) {
            printf("Invalid move.\n");
            continue;
        }
        // execute move
        MovePiece(g, src, dest, -1);
        if (CheckWin(g)) { PrintPlayBoard(g); printf("%s wins!\n", g->current_turn == 0 ? "Red" : "Black"); return 0; }
        g->current_turn = 1 - g->current_turn;
        return 1;
    }
}

int main(void) {
    GameState game;
    InitGame(&game);
    printf("Welcome to BitBoard Checkers!\n\n");
    PrintRefBoard();

    // Run game loop
    while (1) {
        int cont = PlayerTurn(&game);
        if (!cont) break;
    }
    printf("Game over...");
    return 0;
}
