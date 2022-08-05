//
// Created by johnw on 2021-12-18.
//
#include <iostream>
#include "board.h"
#include <cstring>
#define RECURSIVE_DEPTH 11
#define INT_MIN -2147483648
#define INT_MAX 2147483647

Board::Board() {
    // initialize the board with the starting configuration
    pieces = new piece *[8];
    for (int i = 0; i < 8; i++) {
        pieces[i] = new piece[8];
        for (int j = 0; j < 8; j++) {
            pieces[i][j] = {BLANK, false};
        }
    }
    // iterate over each column, and place pieces corresponding to each column
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) {
            pieces[1][i] = {o, false};
            pieces[7][i] = {x, false};
            pieces[5][i] = {x, false};
        } else {
            pieces[0][i] = {o, false};
            pieces[2][i] = {o, false};
            pieces[6][i] = {x, false};
        }
    }
}

Board::Board(piece **p) {
    // copy constructor
    pieces = new piece *[8];
    for (int i = 0; i < 8; i++) {
        pieces[i] = new piece[8];
        memcpy(pieces[i], p[i], 8 * sizeof(piece));
    }
}

void Board::display_board() {
    // output the board in a pretty format
    using namespace std;
    cout << endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // display row number if on first col
            if (j == 0) {
                cout << i + 1 << "  ";
            }
            std::string piece;

            // lowercase represents regular, capital represents kinged
            // use an empty space for blank slots
            switch (pieces[i][j].t) {
                case x:
                    piece = pieces[i][j].kinged ? "X" : "x";
                    break;
                case o:
                    piece = pieces[i][j].kinged ? "O" : "o";
                    break;
                case BLANK:
                    piece = " ";
                    break;
            }
            // use vertical bar to delineate between each row and col
            cout << "|" << piece;
        }
        cout << "|" << endl;
    }
    cout << "    ";
    for (int i = 0; i < 8; i++) {
        cout << i + 1 << " ";
    }
    cout << endl;
}

// get all the diagonals a given distance away, ensuring they are all valid locations on the board
// this doesn't check that a piece isn't already there or if it is a valid move; that comes later
std::vector<location> Board::get_diagonals(location loc, int dist) {
    std::vector<location> locs;
    if (loc.x >= dist && loc.y >= dist) {
        locs.push_back(location{loc.x - dist, loc.y - dist});
    }
    if (loc.x >= dist && loc.y <= 7 - dist) {
        locs.push_back(location{loc.x - dist, loc.y + dist});
    }
    if (loc.x <= 7 - dist && loc.y <= 7 - dist) {
        locs.push_back(location{loc.x + dist, loc.y + dist});
    }
    if (loc.x <= 7 - dist && loc.y >= dist) {
        locs.push_back(location{loc.x + dist, loc.y - dist});
    }
    return locs;
}

// get an array of locations for all pieces on the board
std::vector<location> Board::team_pieces(type team) {
    std::vector<location> p;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (pieces[i][j].t == team) {
                p.push_back(location{j, i});
            }
        }
    }
    return p;
}

std::vector<move> Board::possible_moves(type team) {
    return possible_moves(team, false, nullptr);
}

std::vector<move> Board::possible_moves(type team, bool only_required, location *specific_loc = nullptr) {
    /*
     * find all possible moves for the player
     * team: x or o
     * only_required: indicates whether the function should only return takeaways, otherwise return none
     * - this is the recursive part of the algorithm, since you are required to takeout if able to, with removals allowed to be chained
     * - when initially called, this is false, but when called recursively to find further possible takeouts, this is true
     * specific_loc: specify a single location to check for possible moves, instead of the entire team's pieces
     * - this is also used when recursively computing takeouts
     */
    std::vector<move> moves;
    std::vector<location> locs = specific_loc ? std::vector<location>{*specific_loc} : team_pieces(team);
    int i = 2;
    for (; i > 0; i--) {
        for (auto &loc: locs) {
            // when i=2 find all possible removals (since required to remove, don't need to check anything else if removals found)
            // when i=1 find all possible moves (non-takeout)
            piece p = pieces[loc.y][loc.x];
            std::vector<location> diagonals = get_diagonals(loc, i);
            for (auto &diag: diagonals) {
                // only check diagonals in a valid direction (if kinged, can go in any direction, otherwise just forward)
                if (p.kinged || (diag.y > loc.y && p.t == o) || (diag.y < loc.y && p.t == x)) {
                    // check that the end location isn't already occupied and if looking for takeouts, check if this is hopping over another piece
                    if ((pieces[diag.y][diag.x].t == BLANK) & (i == 1 || pieces[loc.y + (diag.y > loc.y ? 1 : -1)][loc.x + (diag.x > loc.x ? 1 : -1)].t == (1 - team))) {
                        moves.push_back(move{loc.x, loc.y, std::vector<location>{location{diag.x, diag.y}}});
                    }
                }
            }
        }
        // if at least one takeout has been found, no need to look any further
        if (!moves.empty() || only_required) {
            break;
        }
    }
    if (i == 2) {
        // if takeout found: recursively find further hops that can be done
        if (moves.empty()) {
            return moves;
        }
        std::vector<move> new_moves;
        for (auto &m: moves) {
            pieces[m.to.back().y][m.to.back().x] = pieces[m.from.y][m.from.x];
            location from = m.to.back();
            std::vector<move> chain = possible_moves(team, true, &from);
            pieces[m.to.back().y][m.to.back().x] = piece{BLANK, false};
            if (chain.empty()) {
                new_moves.push_back(m);
                continue;
            }

            for (auto &c: chain) {
                std::vector<location> chained_to = m.to;
                // append move chain to found move
                chained_to.insert(chained_to.end(), c.to.begin(), c.to.end());
                new_moves.push_back(move{m.from, chained_to});
            }
        }
        return new_moves;
    } else {
        // return found non-takeout moves
        return moves;
    }
}

// output possible moves for a player (taking output from Board::possiblemoves
void Board::display_moves(const std::vector<move> &moves) {
    int i=1;
    for (auto &m: moves) {
        // displayed in the following format
        // n = move number
        // from = from location
        // to = concatenated string of 'to' locs, each with format (x, y),
        // <n>. <from> to <to>
        std::cout << i << ". (" << m.from.x + 1 << ", " << m.from.y + 1 << ") ";
        for (auto &p: m.to) {
            std::cout << "to (" << p.x + 1 << ", " << p.y + 1 << ") ";
        }
        std::cout << std::endl;
        i++;
    }
}

// make a move on the board
// while doing this, a stack of move histories is kept
// this is used to be able to rewind a given move (used for move searching algorithm)
void Board::make_move(const move &move) {
    // store board in history stack
    auto *b1 = new Board(pieces);
    history.push_back(b1);

    piece p = pieces[move.from.y][move.from.x];
    // set from loc to nothing
    pieces[move.from.y][move.from.x] = piece{BLANK, false};

    // set to loc to piece
    pieces[move.to.back().y][move.to.back().x] = p;

    // if piece has reached the end, king the piece
    if ((move.to.back().y == 7 && p.t == o) || (move.to.back().y == 0 && p.t == x)) {
        pieces[move.to.back().y][move.to.back().x].kinged = true;
    }

    for (int i = 0; i < move.to.size(); i++) {
        location prev_loc = i == 0 ? move.from : move.to[i - 1];

        // if this is a takeout (jump of 2 on the diagonal, erase pieces jumped over
        // do this for each of the 'to' locs
        if (std::abs(prev_loc.y - move.to[i].y) > 1) {
            pieces[prev_loc.y + (move.to[i].y - prev_loc.y > 0 ? 1 : -1)][prev_loc.x + (move.to[i].x - prev_loc.x > 0 ? 1 : -1)] = piece{BLANK, false};
        }
    }
}

// scoring heuristic function, scored in terms of x
// the higher the score, the better the board state is for x
int Board::score_board() {
    // scoring in terms of x
    const int piece_score = 20; // score for having a regular piece
    const int king_score = 31; // score for having kinged piece
    const int back_row_score = 16; // score for having piece in back row (prevents opponent from kinging piece
    const int middle_rows_score = 2; // score for having pieces in the middle rows
    const int vulnerable_piece_score = -3; // score for having a piece under threat

    std::vector<location> piece_locs[2] = {team_pieces(x), team_pieces(o)};
    if (piece_locs[0].empty()) {
        return INT_MIN;
    } else if (piece_locs[1].empty()) {
        return INT_MAX;
    }
    int score = 0;
    for (int i = 0; i < 2; i++) {
        int mult = i == 0 ? 1 : -1;
        for (auto &loc: piece_locs[i]) {
            score += mult * (pieces[loc.y][loc.x].kinged ? king_score : piece_score); // piece and king scoring
            if ((loc.y == 7 && i == 0) || (loc.y == 0 && i == 1)) { // back row scoring
                score += mult * back_row_score;
            } else if (loc.y == 3 || loc.y == 4) { // middle row scoring
                score += mult * middle_rows_score;
            }
        }
        std::vector<move> moves = possible_moves(i == 0 ? o : x);
        for (auto &m: moves) { // vulnerable piece score
            if (std::abs(m.from.y - m.to.front().y) > 1) {
                score += mult * m.to.size() * vulnerable_piece_score;
            }
        }
    }
    return score;
}

// rewind board state
void Board::undo() {
    if (history.empty()) {
        return;
    }
    if (prev_undo != nullptr) {
        delete prev_undo;
    }
    for (int i=0; i<8; i++) {
        delete[] pieces[i];
    }
    delete[] pieces;
    pieces = history.back()->pieces;
    prev_undo = history.back();
    history.pop_back();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

// main game loop
void Game::play() {
    type team = x; // current team move
    for (int i = 0;; i++) {
        b.display_board();
        std::cout << (team == x ? "x" : "o") << "'s turn" << std::endl;
        if (team == x) {
            // find possible moves for the user
            auto poss_moves = b.possible_moves(team);

            // if there aren't any possible moves, they lose
            if (poss_moves.empty()) {
                std::cout << "You suck! The computer wins!" << std::endl;
                return;
            }
            Board::display_moves(poss_moves);
            int choice;
            std::cin >> choice;
            if (choice == 0) {
                std::cout << "Invalid Choice" << std::endl;
                std::cin.ignore();
                std::cin.clear();
                continue;
            }
            b.make_move(poss_moves[choice - 1]);
        } else {
            move m = calc_move();
            // indicates no possible move
            if (m.from.x == -1) {
                std::cout << "You win! Congrats!" << std::endl;
            }
            b.make_move(m);
        }




        team = team == x ? o : x;
    }
}

move Game::calc_move() {
    board_states_analyzed = 0;
    std::vector<move> moves = b.possible_moves(o);
    if (moves.empty()) {
        return move{location{-1, -1}, std::vector<location>{location{-1, -1}}};
    } else if (moves.size() == 1) {
        return moves[0];
    } else {
        int m = calc_move(INT_MIN, INT_MAX, RECURSIVE_DEPTH, false, true);
        std::cout << std::endl << "board states analyzed: " << board_states_analyzed << std::endl;
        return moves[m];
    }
}

int Game::calc_move(int alpha, int beta, int depth, bool maximize, bool root) {
    board_states_analyzed++;
    if (depth == 0) {
        // return board heuristic at leaf node
        return b.score_board();
    }

    // vanilla minimax algorithm with alpha-beta pruning
    if (maximize) {
        int max_index = -1;
        int val = INT_MIN;
        std::vector<move> moves = b.possible_moves(x);
        for (int i = 0; i < moves.size(); i++) {
            b.make_move(moves[i]);
            int move_score = calc_move(alpha, beta, depth - 1, false, false);
            b.undo();
            if (move_score > val) {
                val = move_score;
                max_index = i;
            }
            if (val >= beta) {
                break;
            }
            alpha = std::max(alpha, val);
        }
        return root ? max_index : val;
    } else {
        int min_index = -1;
        int val = INT_MAX;
        std::vector<move> moves = b.possible_moves(o);
        for (int i = 0; i < moves.size(); i++) {
            b.make_move(moves[i]);
            int move_score = calc_move(alpha, beta, depth - 1, true, false);
            b.undo();
            if (root) {
                std::cout << ".";
            }
            if (move_score < val) {
                val = move_score;
                min_index = i;
            }
            if (val <= alpha) {
                break;
            }
            beta = std::min(beta, val);
        }
        return root ? min_index : val;
    }
}

#pragma clang diagnostic pop