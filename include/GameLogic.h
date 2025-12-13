#pragma once

#include <vector>
#include <string>
#include <utility> // std::pair


// Thuần logic cờ, ko SFML
class GoGame
{
public:
    // 0 = empty, 1 = black, 2 = white
    enum Color
    {
        Empty = 0,
        Black = 1,
        White = 2
    };

    // ======= phase của ván =======
    enum class Phase {
        Playing,    // đang chơi bình thường
        MarkDead,   // đang đánh dấu quân chết
        Finished    // đã chấm điểm xong
    };

    // ===== Nhật luật (Japanese scoring) =====
    struct JapaneseScore
    {
        int    blackTerritory = 0;
        int    whiteTerritory = 0;
        int    neutral        = 0;    // dame
        int    blackCaptures  = 0;
        int    whiteCaptures  = 0;
        double komi           = 0.0;  // điểm cộng cho trắng
        double blackTotal = 0;
        double whiteTotal = 0;
        
    };

    // Kết quả khi mark dead
    struct MarkDeadResult
    {   
        bool        ok;       // có gỡ được group nào không
        std::string message;  // thông báo
        int         removed;  // số quân bị gỡ
    };
    
    struct MoveResult
    {
        bool        ok;       // nước đi hợp lệ hay không
        std::string message;  // "OK", "Illegal move", "Ko rule", ...
        int         captured; // số quân bị bắt trong nước này
    };

    
    
        // Komi (điểm cộng cho trắng), ví dụ 6.5 hoặc 7.5
    void   setKomi(double k) { m_komi = k; }
    double getKomi() const   { return m_komi; }

    // Tính điểm theo Nhật luật trên bàn hiện tại
    JapaneseScore computeJapaneseScore() const;
    JapaneseScore computeJapaneseScoreWithDead() const;      // có dead mark
    JapaneseScore finalizeScore();                           // chốt kết quả, sang Finished
    
    const std::vector<bool>& getDeadMarks() const { return m_deadMarks; }

    bool toggleDeadStone(int row, int col);                  // click 1 quân để mark/unmark
    int countGroupLiberties(int row, int col) const;  // xai cho AI

public:
    explicit GoGame(int boardSize = 9);

    // Reset ván mới với kích thước 9/13/19
    void reset(int boardSize);

    // ====== Getter cơ bản (dùng cho UI / GameScreen) ======
    int getBoardSize() const { return m_boardSize; }
    int getCurrentPlayer() const { return m_currentPlayer; } // 0 = Black, 1 = White
    void setCurrentPlayer(int p)   //de dung trong App.cpp
        {
            if (p == 0 || p == 1)
                m_currentPlayer = p;
        }

    // trả về 0/1/2
    int getCell(int row, int col) const;

    int getBlackCaptured() const { return m_blackCaptured; }
    int getWhiteCaptured() const { return m_whiteCaptured; }
    bool isGameOver() const { return m_gameOver; }
    bool isMarkingDead() const { return m_phase == Phase::MarkDead; }
    bool isPlaying()    const { return m_phase == Phase::Playing; }

    // ====== Hành động chính ======
    // Đặt quân tại (row, col) theo lượt hiện tại, áp dụng bắt quân, ko, tự sát
    MoveResult playMove(int row, int col);

    // Pass lượt (có xử lý kết thúc ván nếu 2 lần pass liên tiếp)
    MoveResult pass();

    // Save / load (format giống GameScreen cũ)
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);
    int getLibertiesAt(int row, int col) const;

    // Danh sách các nước đi hợp lệ cho người chơi hiện tại (dùng cho AI)
    // Mỗi phần tử là (row, col)
    std::vector<std::pair<int,int>> getLegalMoves() const;
    // Mark dead stones: gỡ group tại (row, col), cộng vào captured của đối thủ
    MarkDeadResult markDeadGroup(int row, int col);


     // UNDO / REDO 
    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();
private:
    int m_boardSize;       // 9, 13, 19
    int m_currentPlayer;   // 0 = Black, 1 = White
    int m_blackCaptured;
    int m_whiteCaptured;
    

    double m_komi = 6.5;
    bool m_gameOver;
    int m_consecutivePasses; // số lần pass liên tiếp

    // Bàn cờ hiện tại: size = boardSize * boardSize, lưu 0/1/2
    std::vector<int> m_boardCells;

    // Bàn cờ trước đó (để check luật ko)
    std::vector<int> m_prevBoardCells;
    bool             m_hasPrevBoard;

    Phase             m_phase = Phase::Playing;
    std::vector<bool> m_deadMarks;   // true = quân đang được mark chết

    // ===== Helper nội bộ =====
    void ensureBoardArray(); // đảm bảo m_boardCells có size = n*n

    // BFS tìm group & liberties trên một "board" cho trước (dùng được cho mô phỏng)
    void getGroupAndLiberties(
        int row, int col, int color,
        std::vector<int>& outGroup,
        int& outLiberties,
        const std::vector<int>& board
    ) const;
    JapaneseScore computeJapaneseScoreImpl(
        const std::vector<int>& board,
        int extraBlackCap,
        int extraWhiteCap
    ) const;

    struct GameState {
        int boardSize;
        int currentPlayer;
        int blackCaptured;
        int whiteCaptured;
        bool gameOver;
        int consecutivePasses;
        Phase phase;
        bool hasPrevBoard;
        std::vector<int> boardCells;
        std::vector<int> prevBoardCells;
        std::vector<bool> deadMarks;
    };

    std::vector<GameState> m_history;
    int m_historyIndex = -1;

    void saveState();           // lưu state hiện tại vào history
    void restoreState(int idx); // load lại từ history[idx]
};

