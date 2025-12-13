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

    // =========================
    // EASY: 1-ply greedy, KHÔNG random
    // =========================
    if (m_diff == AIDifficulty::Easy) {
        double bestScore = -1e18;
        std::pair<int,int> bestMove = legalMoves[0];

        for (auto [r, c] : legalMoves) {
            GoGame child = game;
            auto res = child.playMove(r, c);
            if (!res.ok) continue;

            double s = evaluatePosition(child, aiColor);

            // BONUS: ăn càng nhiều quân càng ngon
            s += res.captured * 2.5;    // chỉnh 2–4 tuỳ cậu

            // PHẠT: nước chơi xong mà group còn 1 liberty và không ăn quân
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

    // =========================
    // MEDIUM & HARD: đánh giá + minimax
    // =========================
    struct Candidate {
        int r, c;
        double eval;
    };

    std::vector<Candidate> candidates;
    candidates.reserve(legalMoves.size());

    // 1) Đánh giá nhanh từng nước
    for (auto [r, c] : legalMoves) {
        GoGame child = game;
        auto res = child.playMove(r, c);
        if (!res.ok) continue;

        double e = evaluatePosition(child, aiColor);

        // cùng tweak như trên
        e += res.captured * 2.5;
        int libs = child.countGroupLiberties(r, c);
        if (libs == 1 && res.captured == 0)
            e -= 4.0;

        candidates.push_back({r, c, e});
    }


    if (candidates.empty())
        return {-1, -1};

    // 2) Sắp xếp theo eval giảm dần
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.eval > b.eval;
              });

    // 3) Chọn K và depth theo độ khó + kích thước bàn
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

    // Giới hạn K theo số ứng viên thực tế
    if ((int)candidates.size() > K)
        candidates.resize(K);
    else
        K = (int)candidates.size();  // đề phòng ít hơn K

    // 5) Minimax / Alpha-Beta trên các ứng viên
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

// ==== ĐÁNH GIÁ VỊ TRÍ (có bonus ô giữa cho mọi mode) ====

double GoAI::evaluatePosition(const GoGame& game, int aiColor) const
{
    // 1) Dùng JapaneseScore để lấy territory + captures
    GoGame::JapaneseScore js = game.computeJapaneseScore();

    // điểm cho mỗi bên theo Nhật luật (không cần phân biệt Black/White ở đây)
    double blackScore = js.blackTerritory + js.blackCaptures;
    double whiteScore = js.whiteTerritory + js.whiteCaptures + js.komi;

    double baseDiff;
    if (aiColor == GoGame::Black)
        baseDiff = blackScore - whiteScore;
    else
        baseDiff = whiteScore - blackScore;

    // scale nhẹ cho đỡ “quá nhạy”
    double score = baseDiff * 0.6;  // có thể chỉnh 0.5–1.0 tuỳ cảm giác

    // 2) Thêm ưu tiên vị trí: gần trung tâm thì tốt hơn
    int n = game.getBoardSize();
    int center = n / 2;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            int v = game.getCell(r, c);
            if (v == GoGame::Empty) continue;

            int manDist = std::abs(r - center) + std::abs(c - center);
            double centerWeight = std::max(0, n - manDist); // càng gần càng lớn
            double w = 0.08 * centerWeight;                 // hơi nhỏ hơn bản trước

            if (v == aiColor) score += w;
            else              score -= w;
        }
    }

    // 3) BONUS mạnh riêng cho đúng ô giữa (tất cả mode đều có)
    int cv = game.getCell(center, center);
    if (cv == aiColor)
        score += 1.0;
    else if (cv != GoGame::Empty && cv != aiColor)
        score -= 1.0;

    return score;
}


// ==== MINIMAX THƯỜNG ====

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

// ==== MINIMAX + ALPHA-BETA (Hard) ====

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
