
#include "GameLogic.h"

#include <fstream>
#include <algorithm> 
#include <queue>     
#include <cmath>     
#include <iostream>  

GoGame::GoGame(int boardSize)
{
    reset(boardSize);
}

void GoGame::reset(int boardSize)
{
    if (boardSize != 9 && boardSize != 13 && boardSize != 19)
        boardSize = 9;

    m_boardSize        = boardSize;
    m_currentPlayer    = 0;  
    m_blackCaptured    = 0;
    m_whiteCaptured    = 0;
    m_gameOver         = false;
    m_consecutivePasses = 0;

    m_phase = Phase::Playing;
    int n = m_boardSize;
    m_boardCells.assign((std::size_t)(n * n), 0);

   
    m_history.clear();
    m_historyIndex = -1;

    

    saveState();

    ensureBoardArray();

    
    m_prevBoardCells = m_boardCells;
    m_hasPrevBoard   = true;

    m_deadMarks.assign(m_boardCells.size(), false);

}

int GoGame::getCell(int row, int col) const
{
    if (row < 0 || col < 0 || row >= m_boardSize || col >= m_boardSize)
        return 0;

    int idx = row * m_boardSize + col;
    if (idx < 0 || idx >= (int)m_boardCells.size())
        return 0;

    return m_boardCells[(std::size_t)idx];
}

void GoGame::ensureBoardArray()
{
    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    std::size_t expected = (std::size_t)(n * n);
    if (m_boardCells.size() != expected)
        m_boardCells.assign(expected, 0);
}


void GoGame::getGroupAndLiberties(
    int row, int col, int color,
    std::vector<int>& outGroup,
    int& outLiberties,
    const std::vector<int>& board
) const
{
    outGroup.clear();
    outLiberties = 0;

    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    if (row < 0 || row >= n || col < 0 || col >= n)
        return;

    int startIdx = row * n + col;
    if (startIdx < 0 || startIdx >= n * n)
        return;

    if (color != Black && color != White)
        return;

    if (board.empty() || board[(std::size_t)startIdx] != color)
        return;

    std::vector<bool> visited((std::size_t)(n * n), false);
    std::vector<bool> libertyMarked((std::size_t)(n * n), false);

    std::queue<int> q;
    visited[(std::size_t)startIdx] = true;
    q.push(startIdx);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    while (!q.empty())
    {
        int idx = q.front();
        q.pop();

        outGroup.push_back(idx);

        int r = idx / n;
        int c = idx % n;

        for (int k = 0; k < 4; ++k)
        {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr < 0 || nr >= n || nc < 0 || nc >= n)
                continue;

            int nIdx = nr * n + nc;
            int v    = board[(std::size_t)nIdx];

            if (v == Empty)
            {
                if (!libertyMarked[(std::size_t)nIdx])
                {
                    libertyMarked[(std::size_t)nIdx] = true;
                    ++outLiberties;
                }
            }
            else if (v == color && !visited[(std::size_t)nIdx])
            {
                visited[(std::size_t)nIdx] = true;
                q.push(nIdx);
            }
        }
    }
}


GoGame::MoveResult GoGame::playMove(int row, int col)
{   
    
    MoveResult result{false, "", 0};

    if (m_phase != Phase::Playing) {
    result.message = "Cannot play, scoring phase.";
    return result;
    }

    if (m_gameOver)
    {
        result.message = "Game is over";
        return result;
    }

    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    if (row < 0 || row >= n || col < 0 || col >= n)
    {
        result.message = "Out of board";
        return result;
    }

    ensureBoardArray();
    int idx = row * n + col;
    if (idx < 0 || idx >= (int)m_boardCells.size())
    {
        result.message = "Invalid index";
        return result;
    }

    if (m_boardCells[(std::size_t)idx] != Empty)
    {
        result.message = "Occupied";
        return result;
    }

    
    std::vector<int> oldBoard = m_boardCells;

    int color    = (m_currentPlayer == 0 ? Black : White);
    int opponent = (color == Black ? White : Black);

    m_boardCells[(std::size_t)idx] = color;

  
    int capturedThisMove = 0;
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int k = 0; k < 4; ++k)
    {
        int nr = row + dr[k];
        int nc = col + dc[k];
        if (nr < 0 || nr >= n || nc < 0 || nc >= n)
            continue;

        int nIdx = nr * n + nc;
        if (m_boardCells[(std::size_t)nIdx] != opponent)
            continue;

        std::vector<int> oppGroup;
        int oppLibs = 0;
        getGroupAndLiberties(nr, nc, opponent, oppGroup, oppLibs, m_boardCells);

        if (oppLibs == 0)
        {
            for (int gIdx : oppGroup)
            {
                if (gIdx >= 0 && gIdx < n * n)
                    m_boardCells[(std::size_t)gIdx] = Empty;
            }
            capturedThisMove += (int)oppGroup.size();
        }
    }

    
    {
        std::vector<int> myGroup;
        int myLibs = 0;
        getGroupAndLiberties(row, col, color, myGroup, myLibs, m_boardCells);

        if (myLibs == 0 && capturedThisMove == 0)
        {
            m_boardCells = oldBoard;
            result.message = "Illegal move (suicide)";
            return result;
        }
    }

 
    if (m_hasPrevBoard && m_prevBoardCells.size() == m_boardCells.size())
    {
        bool same = std::equal(
            m_boardCells.begin(), m_boardCells.end(),
            m_prevBoardCells.begin()
        );
        if (same)
        {
            m_boardCells = oldBoard;
            result.message = "Ko rule";
            return result;
        }
    }


    
    if (capturedThisMove > 0)
    {
        if (color == Black)
            m_blackCaptured += capturedThisMove;
        else
            m_whiteCaptured += capturedThisMove;
    }

  
    
    m_prevBoardCells    = oldBoard;
    m_hasPrevBoard      = true;
    m_consecutivePasses = 0;


    
    m_currentPlayer = 1 - m_currentPlayer;

    
    
    saveState();

    result.ok       = true;
    result.captured = capturedThisMove;
    result.message  = "OK";
    return result;
}



GoGame::MoveResult GoGame::pass()
{
    MoveResult result{true, "", 0};
    if (m_phase != Phase::Playing)
    {
        result.ok = false;
        result.message = "Not in playing phase";
        return result;
    }
    

    ++m_consecutivePasses;
    std::string who = (m_currentPlayer == 0 ? "Black" : "White");

    if (m_consecutivePasses >= 2)
    {
        
        
        m_consecutivePasses = 0;
        m_phase = Phase::MarkDead;

        m_deadMarks.assign(m_boardCells.size(), false);

        result.message =
            "Both players passed.\n"
            "Mark dead stones by clicking them.\n"
            "Press 'Finish and Score' to finish scoring.";
    }
        else
    {
        result.message  = who + " passed.";
        m_currentPlayer = 1 - m_currentPlayer;
    }

   
    
    saveState();

    return result;
}



bool GoGame::saveToFile(const std::string& path) const
{
    std::ofstream out(path);
    if (!out)
        return false;

    out << m_boardSize     << " "
        << m_currentPlayer << " "
        << m_blackCaptured << " "
        << m_whiteCaptured << "\n";

    int n = m_boardSize;
    for (int i = 0; i < n * n; ++i)
    {
        out << m_boardCells[(std::size_t)i];
        if (i + 1 < n * n)
            out << ' ';
    }
    out << "\n";

    return (bool)out;
}

bool GoGame::loadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        return false;

    int bSize = 0;
    int cur   = 0;
    int bCap  = 0;
    int wCap  = 0;

    in >> bSize >> cur >> bCap >> wCap;
    if (!in)
        return false;

    if (bSize != 9 && bSize != 13 && bSize != 19)
        return false;
    if (cur < 0 || cur > 1)
        return false;

    m_boardSize     = bSize;
    m_currentPlayer = cur;
    m_blackCaptured = bCap;
    m_whiteCaptured = wCap;
    m_gameOver      = false;
    m_consecutivePasses = 0;

    ensureBoardArray();

    int n = m_boardSize;
    for (int i = 0; i < n * n; ++i)
    {
        if (!(in >> m_boardCells[(std::size_t)i]))
            return false;
        int v = m_boardCells[(std::size_t)i];
        if (v < 0 || v > 2)
            m_boardCells[(std::size_t)i] = 0;
    }


    
    m_prevBoardCells    = m_boardCells;
    m_hasPrevBoard      = true;


    m_history.clear();
    m_historyIndex = -1;
    saveState();          

   
    
    

    return true;
}



std::vector<std::pair<int,int>> GoGame::getLegalMoves() const
{
    std::vector<std::pair<int,int>> moves;

    if (m_gameOver)
        return moves;

    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    
    
    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            int idx = r * n + c;
            if (m_boardCells[(std::size_t)idx] != Empty)
                continue;

            GoGame tmp = *this; // copy
            MoveResult res = tmp.playMove(r, c);
            if (res.ok)
            {
                moves.emplace_back(r, c);
            }
        }
    }

    return moves;
}


GoGame::JapaneseScore GoGame::computeJapaneseScoreImpl(
    const std::vector<int>& board,
    int extraBlackCap,
    int extraWhiteCap
) const
{
    JapaneseScore score;
    score.komi          = m_komi;
    score.blackCaptures = m_blackCaptured + extraBlackCap;
    score.whiteCaptures = m_whiteCaptured + extraWhiteCap;

    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    if (board.empty())
        return score;

    std::vector<bool> visited(board.size(), false);
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            int idx = r * n + c;
            if (visited[(std::size_t)idx])
                continue;

            int v = board[(std::size_t)idx];

            
            
            if (v != Empty)
            {
                visited[(std::size_t)idx] = true;
                continue;
            }

           
            
            bool adjBlack = false;
            bool adjWhite = false;
            std::vector<int> region;

            std::queue<int> q;
            visited[(std::size_t)idx] = true;
            q.push(idx);

            while (!q.empty())
            {
                int cur = q.front();
                q.pop();
                region.push_back(cur);

                int cr = cur / n;
                int cc = cur % n;

                for (int k = 0; k < 4; ++k)
                {
                    int nr = cr + dr[k];
                    int nc = cc + dc[k];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= n)
                        continue;

                    int nIdx = nr * n + nc;
                    int cell = board[(std::size_t)nIdx];

                    if (cell == Empty)
                    {
                        if (!visited[(std::size_t)nIdx])
                        {
                            visited[(std::size_t)nIdx] = true;
                            q.push(nIdx);
                        }
                    }
                    else if (cell == Black)
                    {
                        adjBlack = true;
                    }
                    else if (cell == White)
                    {
                        adjWhite = true;
                    }
                }
            }

            int regionSize = (int)region.size();

            if (adjBlack && !adjWhite)
            {
              
                
                score.blackTerritory += regionSize;
            }
            else if (adjWhite && !adjBlack)
            {
               
                
                score.whiteTerritory += regionSize;
            }
            else
            {
                
                
                score.neutral += regionSize;
            }
        }
    }
    
    
    score.blackTotal = score.blackTerritory + score.blackCaptures;
    score.whiteTotal = score.whiteTerritory + score.whiteCaptures + score.komi;

    return score;
}

GoGame::JapaneseScore GoGame::computeJapaneseScore() const
{
    return computeJapaneseScoreImpl(m_boardCells, 0, 0);
}

GoGame::JapaneseScore GoGame::computeJapaneseScoreWithDead() const
{
    if (m_deadMarks.empty())
        return computeJapaneseScore();

    std::vector<int> tmp = m_boardCells;
    int extraBlackCap = 0;
    int extraWhiteCap = 0;

    int nCells = (int)tmp.size();
    for (int idx = 0; idx < nCells; ++idx)
    {
        if (!m_deadMarks[(std::size_t)idx])
            continue;

        int v = tmp[(std::size_t)idx];
        if (v == Black)
        {
            ++extraWhiteCap;
            tmp[(std::size_t)idx] = Empty;
        }
        else if (v == White)
        {
            ++extraBlackCap;
            tmp[(std::size_t)idx] = Empty;
        }
    }

    return computeJapaneseScoreImpl(tmp, extraBlackCap, extraWhiteCap);
}

GoGame::JapaneseScore GoGame::finalizeScore()
{
    
    
    std::vector<int> tmp = m_boardCells;
    int extraBlackCap = 0;
    int extraWhiteCap = 0;

    int nCells = (int)tmp.size();
    for (int idx = 0; idx < nCells; ++idx)
    {
        if (!m_deadMarks.empty() && !m_deadMarks[(std::size_t)idx])
            continue;

        int v = tmp[(std::size_t)idx];
        if (v == Black)
        {
            ++extraWhiteCap;                  
            
            tmp[(std::size_t)idx] = Empty;    
            

        }
        else if (v == White)
        {
            ++extraBlackCap;                  
            tmp[(std::size_t)idx] = Empty;
        }
    }

  
    JapaneseScore s = computeJapaneseScoreImpl(tmp, extraBlackCap, extraWhiteCap);

    
    
    m_boardCells     = std::move(tmp);
    m_blackCaptured += extraBlackCap;
    m_whiteCaptured += extraWhiteCap;
    m_prevBoardCells = m_boardCells;
    m_hasPrevBoard   = true;

    m_phase    = Phase::Finished;
    m_gameOver = true;

 
    
    saveState();

    return s;
}



bool GoGame::toggleDeadStone(int row, int col)
{
    if (m_phase != Phase::MarkDead)
        return false;

    int n = m_boardSize;
    if (row < 0 || col < 0 || row >= n || col >= n)
        return false;

    int idx = row * n + col;
    if (idx < 0 || idx >= (int)m_boardCells.size())
        return false;

    if (m_boardCells[(std::size_t)idx] == Empty)
        return false;

    if (m_deadMarks.size() != m_boardCells.size())
        m_deadMarks.assign(m_boardCells.size(), false);

    m_deadMarks[(std::size_t)idx] = !m_deadMarks[(std::size_t)idx];
    return true;
}
int GoGame::getLibertiesAt(int row, int col) const
{
    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    if (row < 0 || row >= n || col < 0 || col >= n)
        return 0;

    int idx = row * n + col;
    if (idx < 0 || idx >= (int)m_boardCells.size())
        return 0;

    int color = m_boardCells[(std::size_t)idx];
    if (color != Black && color != White)
        return 0;

    std::vector<int> group;
    int libs = 0;
    getGroupAndLiberties(row, col, color, group, libs, m_boardCells);
    return libs;
}

int GoGame::countGroupLiberties(int row, int col) const
{
    int n = m_boardSize;
    if (row < 0 || col < 0 || row >= n || col >= n)
        return 0;

    int color = getCell(row, col);
    if (color != Black && color != White)
        return 0;

    std::vector<int> group;
    int libs = 0;
    getGroupAndLiberties(row, col, color, group, libs, m_boardCells);
    return libs;
}





GoGame::MarkDeadResult GoGame::markDeadGroup(int row, int col)
{
    MarkDeadResult res{false, "", 0};
    
    int n = m_boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

  
    if (m_phase != Phase::MarkDead)
    {
        res.message = "You can only mark dead stones in mark-dead phase.";
        return res;
    }

    if (row < 0 || row >= n || col < 0 || col >= n)
    {
        res.message = "Out of board.";
        return res;
    }

    int idx = row * n + col;
    if (idx < 0 || idx >= (int)m_boardCells.size())
    {
        res.message = "Invalid index.";
        return res;
    }

    int color = m_boardCells[(std::size_t)idx];
    if (color != Black && color != White)
    {
        res.message = "No stone at this point.";
        return res;
    }

    int opponent = (color == Black ? White : Black);

    
    std::vector<int> group;
    int libs = 0;
    getGroupAndLiberties(row, col, color, group, libs, m_boardCells);

    if (group.empty())
    {
        res.message = "Cannot find group.";
        return res;
    }

   
    for (int gIdx : group)
    {
        if (gIdx >= 0 && gIdx < n * n)
        {
            m_boardCells[(std::size_t)gIdx] = Empty;
        }
    }

    int removed = (int)group.size();
    res.removed = removed;

  
    if (color == Black)
        m_whiteCaptured += removed;
    else
        m_blackCaptured += removed; 

    
    m_prevBoardCells = m_boardCells;
    m_hasPrevBoard   = true;

    res.ok = true;
    res.message = (color == Black
        ? "Removed " + std::to_string(removed) + " black stone(s)."
        : "Removed " + std::to_string(removed) + " white stone(s).");

    

    saveState();
    return res;

    
    
}


bool GoGame::canUndo() const
{
    return (m_historyIndex > 0);
}



bool GoGame::canRedo() const
{
    return (m_historyIndex + 1 < (int)m_history.size());
}

bool GoGame::undo()
{
    if (!canUndo()) return false;
    restoreState(m_historyIndex - 1);
    return true;
}

bool GoGame::redo()
{
    if (!canRedo()) return false;
    restoreState(m_historyIndex + 1);
    return true;
}



void GoGame::saveState()
{
    GameState st;
    st.boardSize        = m_boardSize;
    st.currentPlayer    = m_currentPlayer;
    st.blackCaptured    = m_blackCaptured;
    st.whiteCaptured    = m_whiteCaptured;
    st.gameOver         = m_gameOver;
    st.consecutivePasses= m_consecutivePasses;
    st.phase            = m_phase;
    st.hasPrevBoard     = m_hasPrevBoard;
    st.boardCells       = m_boardCells;
    st.prevBoardCells   = m_prevBoardCells;
    st.deadMarks        = m_deadMarks;

    
    if (m_historyIndex + 1 < (int)m_history.size())
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());

    m_history.push_back(std::move(st));
    m_historyIndex = (int)m_history.size() - 1;
}


void GoGame::restoreState(int idx)
{
    if (idx < 0 || idx >= (int)m_history.size())
        return;

    const GameState& st = m_history[(std::size_t)idx];

    m_boardSize        = st.boardSize;
    m_currentPlayer    = st.currentPlayer;
    m_blackCaptured    = st.blackCaptured;
    m_whiteCaptured    = st.whiteCaptured;
    m_gameOver         = st.gameOver;
    m_consecutivePasses= st.consecutivePasses;
    m_phase            = st.phase;
    m_hasPrevBoard     = st.hasPrevBoard;
    m_boardCells       = st.boardCells;
    m_prevBoardCells   = st.prevBoardCells;
    m_deadMarks        = st.deadMarks;

    m_historyIndex = idx;
}