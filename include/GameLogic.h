
#pragma once

#include <vector>
#include <string>
#include <utility> 



class GoGame
{
public:
  
    enum Color
    {
        Empty = 0,
        Black = 1,
        White = 2
    };

  
    enum class Phase {
        Playing,    
        MarkDead,   
        Finished    
    };

   
    struct JapaneseScore
    {
        int    blackTerritory = 0;
        int    whiteTerritory = 0;
        int    neutral        = 0;    
        int    blackCaptures  = 0;
        int    whiteCaptures  = 0;
        double komi           = 0.0;  
        double blackTotal = 0;
        double whiteTotal = 0;
        
    };

    struct MarkDeadResult
    {   
        bool        ok;       
        std::string message; 
        int         removed;  
    };
    
    struct MoveResult
    {
        bool        ok;       
        std::string message; 
        int         captured; 
    };

    
    
        
    void   setKomi(double k) { m_komi = k; }
    double getKomi() const   { return m_komi; }

    
    JapaneseScore computeJapaneseScore() const;
    JapaneseScore computeJapaneseScoreWithDead() const;      
    JapaneseScore finalizeScore();                          
    
    const std::vector<bool>& getDeadMarks() const { return m_deadMarks; }

    bool toggleDeadStone(int row, int col);                
    int countGroupLiberties(int row, int col) const;  

public:
    explicit GoGame(int boardSize = 9);

    
    void reset(int boardSize);


    int getBoardSize() const { return m_boardSize; }
    int getCurrentPlayer() const { return m_currentPlayer; } 
    void setCurrentPlayer(int p)   
        {
            if (p == 0 || p == 1)
                m_currentPlayer = p;
        }

  
    int getCell(int row, int col) const;

    int getBlackCaptured() const { return m_blackCaptured; }
    int getWhiteCaptured() const { return m_whiteCaptured; }
    bool isGameOver() const { return m_gameOver; }
    bool isMarkingDead() const { return m_phase == Phase::MarkDead; }
    bool isPlaying()    const { return m_phase == Phase::Playing; }

  
    MoveResult playMove(int row, int col);

    
    MoveResult pass();

    
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);
    int getLibertiesAt(int row, int col) const;

   
    
    std::vector<std::pair<int,int>> getLegalMoves() const;
   
    MarkDeadResult markDeadGroup(int row, int col);


    
    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();
private:
    int m_boardSize;       
    int m_currentPlayer;  
    int m_blackCaptured;
    int m_whiteCaptured;
    

    double m_komi = 6.5;
    bool m_gameOver;
    int m_consecutivePasses; 

  
    std::vector<int> m_boardCells;

    
    std::vector<int> m_prevBoardCells;
    bool             m_hasPrevBoard;

    Phase             m_phase = Phase::Playing;
    std::vector<bool> m_deadMarks;   

 
    void ensureBoardArray(); 

    
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

    

    void saveState();           
    void restoreState(int idx); 
};

