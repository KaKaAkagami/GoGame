
#include "AI.h"
#include <random>
#include <algorithm>
#include <cmath>

GoAI::GoAI(AIDifficulty diff)
    : m_diff(diff)
{}

void GoAI::setDifficulty(AIDifficulty diff) {
    m_diff = diff;
}

AIDifficulty GoAI::getDifficulty() const {
    return m_diff;
}

std::pair<int,int> GoAI::chooseMove(const GoGame& game, int aiColor)
{
    auto legalMoves = game.getLegalMoves();
    if (legalMoves.empty())
        return {-1, -1};   // pass

    
    // EASY: 1-ply greedy
    
    if (m_diff == AIDifficulty::Easy) {
        double bestScore = -1e18;
        std::pair<int,int> bestMove = legalMoves[0];

        for (auto [r, c] : legalMoves) {
            GoGame child = game;
            auto res = child.playMove(r, c);
            if (!res.ok) continue;

            double s = evaluatePosition(child, aiColor);

            // Băn càng nhiều quân càng ngon
            s += res.captured * 2.5;

            // PHẠT nước chơi xong mà group còn 1 liberty và không ăn quân
            int libs = child.countGroupLiberties(r, c);
            if (libs == 1 && res.captured == 0)
                s -= 4.0;

            if (s > bestScore) {
                bestScore = s;
                bestMove  = {r, c};
            }
        }

        return bestMove;
    }

    
    // MEDIUM & HARD: đánh giá + minimax
    
    struct Candidate {
        int r, c;
        double eval;
    };

    std::vector<Candidate> candidates;
    candidates.reserve(legalMoves.size());

    // Đánh giá nhanh từng nước 1-ply bằng evaluatePosition
    for (auto [r, c] : legalMoves) {
        GoGame child = game;
        auto res = child.playMove(r, c);
        if (!res.ok) continue;

        double e = evaluatePosition(child, aiColor);

        // Thưởng nước ăn quân
        e += res.captured * 2.5;

        // Phạt nước còn 1 liberty và không ăn gì
        int libs = child.countGroupLiberties(r, c);
        if (libs == 1 && res.captured == 0)
            e -= 4.0;


         // Phạt nước dễ bị đối thủ ăn ngay ở lượt sau
        int maxOppCapture = 0;
        {
            GoGame afterMyMove = child;
            auto oppMoves = afterMyMove.getLegalMoves();
            for (auto [orow, ocol] : oppMoves) {
                GoGame tmp = afterMyMove;
                auto oppRes = tmp.playMove(orow, ocol);
                if (!oppRes.ok) continue;
                if (oppRes.captured > maxOppCapture)
                    maxOppCapture = oppRes.captured;
            }
        }
        // phạt 2
        e -= maxOppCapture * 2.0;

        candidates.push_back({r, c, e});
    }

    if (candidates.empty())
        return {-1, -1};

    // Sắp xếp theo giảm dần
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.eval > b.eval;
              });

    // Chọn K và depth theo độ khó + kích thước bàn
    int bs = game.getBoardSize();
    int K;
    int depth;

    if (m_diff == AIDifficulty::Medium) {
        if (bs <= 9) {          // 9x9
            K = 7;
            depth = 2;
        } else if (bs <= 13) {  // 13x13
            K = 6;
            depth = 2;
        } else {                // 19x19
            K = 5;
            depth = 2;
        }
    } else { // Hard
        if (bs <= 9) {          // 9x9
            K = 7;
            depth = 3;
        } else if (bs <= 13) {  // 13x13
            K = 5;
            depth = 3;
        } else {                // 19x19
            K = 3;
            depth = 3;
        }
    }

    // Giới hạn K theo số TH thực tế
    if ((int)candidates.size() > K)
        candidates.resize(K);
    else
        K = (int)candidates.size();  // đề phòng ít hơn K

    // Minimax / Alpha-Beta trên các ứng viên
    double bestScore = -1e18;
    std::pair<int,int> bestMove = {candidates[0].r, candidates[0].c};

    for (const Candidate& cand : candidates) {
        GoGame child = game;
        auto res = child.playMove(cand.r, cand.c);
        if (!res.ok) continue;

        double score = 0.0;
        if (depth <= 0) {
            score = evaluatePosition(child, aiColor);
        } else if (m_diff == AIDifficulty::Medium) {
            score = minimax(child, depth - 1, false, aiColor);
        } else {
            score = minimaxAlphaBeta(child, depth - 1, false,
                                     aiColor, -1e18, 1e18);
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove  = {cand.r, cand.c};
        }
    }

    return bestMove;
}



//  - Territory + captures (JapaneseScore)
//  - Ưu tiên trung tâm
//  - Bonus ô giữa
//  - Neighbor 


double GoAI::evaluatePosition(const GoGame& game, int aiColor) const
{
    // Territory + captures theo luật
    GoGame::JapaneseScore js = game.computeJapaneseScore();

    double blackScore = js.blackTerritory + js.blackCaptures;
    double whiteScore = js.whiteTerritory + js.whiteCaptures + js.komi;

    double baseDiff;
    if (aiColor == GoGame::Black)
        baseDiff = blackScore - whiteScore;
    else
        baseDiff = whiteScore - blackScore;

    
    double score = baseDiff * 0.6;

    //  ưu tiên vị trí gần trung tâm + kết nối chuỗi
    int n = game.getBoardSize();
    int center = n / 2;

    // neighbor: chỉ xét 2 hướng (phải, xuống) để tránh đếm đôi
    static const int dr2[2] = {0, 1};
    static const int dc2[2] = {1, 0};

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            int v = game.getCell(r, c);
            if (v == GoGame::Empty) continue;

            int sign = (v == aiColor ? +1 : -1);

            // ƯU TIÊN GẦN TRUNG TÂM
            int manDist = std::abs(r - center) + std::abs(c - center);
            double centerWeight = std::max(0, n - manDist); // càng gần càng lớn
            double wCenter = 0.08 * centerWeight;

            score += sign * wCenter;

            // thưởng khi 2 quân cùng màu đứng cạnh nhau
            for (int k = 0; k < 2; ++k) { // phải + xuống
                int nr = r + dr2[k];
                int nc = c + dc2[k];
                if (nr < 0 || nr >= n || nc < 0 || nc >= n) continue;

                int v2 = game.getCell(nr, nc);
                if (v2 == v) {
                    // cùng màu, tạo “chuỗi”
                    double wChain = 0.3;  // nhỏ 
                    score += sign * wChain;
                }
            }
        }
    }

    // 3) BONUS riêng cho đúng ô giữa 
    int cv = game.getCell(center, center);
    if (cv == aiColor)
        score += 1.0;
    else if (cv != GoGame::Empty && cv != aiColor)
        score -= 1.0;

    return score;
}


//  MINIMAX THƯỜNG


double GoAI::minimax(GoGame state, int depth, bool maximizingPlayer,
                     int aiColor)
{
    if (depth == 0 || state.isGameOver()) {
        return evaluatePosition(state, aiColor);
    }

    auto moves = state.getLegalMoves();
    if (moves.empty()) {
        return evaluatePosition(state, aiColor);
    }

    double bestVal = maximizingPlayer ? -1e18 : 1e18;

    for (auto [r, c] : moves) {
        GoGame child = state;
        auto res = child.playMove(r, c);
        if (!res.ok) continue;

        double val = minimax(child, depth - 1, !maximizingPlayer, aiColor);

        if (maximizingPlayer)
            bestVal = std::max(bestVal, val);
        else
            bestVal = std::min(bestVal, val);
    }

    return bestVal;
}


//  MINIMAX + ALPHA-BETA (Hard)


double GoAI::minimaxAlphaBeta(GoGame state, int depth, bool maximizingPlayer,
                              int aiColor, double alpha, double beta)
{
    if (depth == 0 || state.isGameOver()) {
        return evaluatePosition(state, aiColor);
    }

    auto moves = state.getLegalMoves();
    if (moves.empty()) {
        return evaluatePosition(state, aiColor);
    }

    if (maximizingPlayer) {
        double bestVal = -1e18;
        for (auto [r, c] : moves) {
            GoGame child = state;
            auto res = child.playMove(r, c);
            if (!res.ok) continue;

            double val = minimaxAlphaBeta(child, depth - 1, false,
                                          aiColor, alpha, beta);
            bestVal = std::max(bestVal, val);
            alpha   = std::max(alpha, bestVal);
            if (beta <= alpha) break;
        }
        return bestVal;
    } else {
        double bestVal = 1e18;
        for (auto [r, c] : moves) {
            GoGame child = state;
            auto res = child.playMove(r, c);
            if (!res.ok) continue;

            double val = minimaxAlphaBeta(child, depth - 1, true,
                                          aiColor, alpha, beta);
            bestVal = std::min(bestVal, val);
            beta    = std::min(beta, bestVal);
            if (beta <= alpha) break;
        }
        return bestVal;
    }
}
