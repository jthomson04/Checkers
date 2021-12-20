//
// Created by johnw on 2021-12-18.
//

#ifndef CHECKERS_BOARD_H
#define CHECKERS_BOARD_H

#include <vector>

enum type {
    x, o, BLANK
};

struct piece {
    type t;
    bool kinged;
};

struct location {
    int x;
    int y;
};

struct move {
    location from;
    std::vector<location> to;
};

class Board {

private:

    static std::vector<location> get_diagonals(location loc, int dist);
    std::vector<move> possible_moves(type team, bool only_required, location *specific_loc);
    std::vector<location> team_pieces(type team);

public:
    int score_board();
    piece **pieces;
    std::vector<Board*> history;
    Board();
    ~Board();
    explicit Board(piece **pieces);
    std::vector<move> possible_moves(type team);
    void undo();
    void display_board();
    void make_move(const move& m);
    static void display_moves(const std::vector<move>& moves);
};

class Game {
public:
    void play();
private:
    move calc_move();
    int calc_move(int alpha, int beta, int depth, bool maximize, bool root);
    Board b;
};

#endif //CHECKERS_BOARD_H
