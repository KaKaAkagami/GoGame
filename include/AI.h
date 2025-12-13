#pragma once
#include <utility>
#include "GameLogic.h"   
#include <algorithm>
enum class AIDifficulty {
    Easy = 1,
    Medium = 2,
    Hard = 3
};

class GoAI {
public:
    GoAI(AIDifficulty diff = AIDifficulty::Easy);

    void setDifficulty(AIDifficulty diff);
    AIDifficulty getDifficulty() const;

    // Trả về (row, col) AI muốn đánh
    // Nếu không có nước hợp lệ (pass), trả (-1, -1)
    std::pair<int,int> chooseMove(const GoGame& game, int aiPlayerColor);

private:
    AIDifficulty m_diff;

    // Các hàm hỗ trợ 
    double evaluatePosition(const GoGame& game, int aiColor) const;

    double minimax(GoGame state, int depth, bool maximizingPlayer,
                   int aiColor);

    double minimaxAlphaBeta(GoGame state, int depth, bool maximizingPlayer,
                            int aiColor, double alpha, double beta);
};
