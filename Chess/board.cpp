#include <iostream>
#include "Chess/board.h"
using namespace std;


Board::Board()
{
    board = {};
    enPassant = POS(-1,-1);
    //MoveUpdate = {};
    CreateBoard();
    //PrintBoard();
}

Board::Board(const Board &b)
{
    //cout << "Board copied" << endl;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (b.board[i][j] != nullptr)
            {
                board[i][j] = b.board[i][j]->Clone();
            }
            else {board[i][j] = nullptr;}
        }
    }
    whiteKing = b.whiteKing;
    blackKing = b.blackKing;
    enPassant = b.enPassant;

}

Board::~Board()
{
    for (ROW row : board)
    {
        for (Piece* p : row)
        {
            if (p != nullptr)
            {
                delete p;
            }
        }
    }
    //cout << "Board destroyed" << endl;
}

QJsonObject Board::Move(POS start, POS end)
{

    Piece* p = board[start.first][start.second];
    cout << "Moving " << p->GetName() << " from " << start.first << ", " << start.second << " to " << end.first << ", " << end.second << endl;

    QJsonObject Move;
    Move["fromX"] = start.first;
    Move["fromY"] = start.second;
    Move["toX"] = end.first;
    Move["toY"] = end.second;

    if (p->GetName() == "King" ) //King Moves
    {
        if (abs(start.second - end.second) > 1) //casting check
        {
            Move["type"] = "castling";
            if (start.second > end.second)//long castling
            {
                board[start.first][start.second - 2] = board[start.first][start.second];
                board[start.first][start.second] = nullptr;
                UpdateKingPOS(POS(start.first, start.second - 2));
                
                board[start.first][start.second - 1] = board[end.first][end.second];
                board[end.first][end.second] = nullptr;
            }
            else//short castling
            {
                board[start.first][start.second + 2] = board[start.first][start.second];
                board[start.first][start.second] = nullptr;
                UpdateKingPOS(POS(start.first, start.second + 2));
                
                board[start.first][start.second + 1] = board[end.first][end.second];
                board[end.first][end.second] = nullptr;
            }
        }
        else //normal king moves
        {
            Move["type"] = "normal";

            NukeTile(end);
            board[end.first][end.second] = p;
            board[start.first][start.second] = nullptr;
            UpdateKingPOS(end);
        }
        p->FirstMove();
        enPassant = POS(-1,-1);

    }
    else if (p->GetName() == "Pawn") //Pawn special actions
    {
        if (abs(start.first - end.first) == 2) //if pawn moved by 2 squares forward, set en passant position
        {
            Move["type"] = "normal";
            enPassant = end;
            p->FirstMove();
            board[end.first][end.second] = p;
            board[start.first][start.second] = nullptr;
        }
        else //all other pawn moves
        {   
            if (POS(end.first-1, end.second) == GetEnPassant()) //en passant check for blac
            {
                NukeTile(POS(end.first - 1, end.second));
                Move["type"] = "enpassant";
            }
            else if (POS(end.first+1, end.second) == GetEnPassant()) //en passant check for white
            {
                NukeTile(POS(end.first + 1, end.second));
                Move["type"] = "enpassant";
            }
            else
            {
                NukeTile(end);
                Move["type"] = "normal";
            }
            board[end.first][end.second] = p;
            board[start.first][start.second] = nullptr;
            p->FirstMove();
            enPassant = POS(-1,-1);
        }
    }
    else //all other moves
    {
        Move["type"] = "normal";
        NukeTile(end);
        string name = p->GetName();
        if (name == "Rook")
        {
            p->FirstMove();   
        }
        board[start.first][start.second] = nullptr; //Remove the piece from the start
        board[end.first][end.second] = p; //Place the piece at the end
        enPassant = POS(-1,-1);        
    }
    cout << "Move finished" << endl;
    return Move;
}


void Board::Promote(POS pos, string piece)
{
    Color color = board[pos.first][pos.second]->GetColor();
    NukeTile(pos);
    if (piece == "Queen")
    {
        board[pos.first][pos.second] = new Queen(color);
    }
    else if (piece == "Rook")
    {
        board[pos.first][pos.second] = new Rook(color);
    }
    else if (piece == "Bishop")
    {
        board[pos.first][pos.second] = new Bishop(color);
    }
    else if (piece == "Knight")
    {
        board[pos.first][pos.second] = new Knight(color);
    }
}

bool Board::IsSquareChecked(POS pos, Color color) const
{
    //Queen and Rook check
    MOVES directions = {POS(1, 0), POS(-1, 0), POS(0, 1), POS(0, -1)};
    for (POS dir : directions)
    {
        POS temp = pos;
        while (true)
        {
            temp.first += dir.first;
            temp.second += dir.second;
            SquareState state = CheckSquare(temp, color);
            if (state == SquareState::INVALID)
            {
                break;
            }
            else if (state == SquareState::EMPTY)
            {
                continue;
            }
            else if (state == SquareState::TAKEN_BY_FRIENDLY)
            {
                break;
            }
            else if (state == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(temp);
                if (p->GetName() == "Queen" || p->GetName() == "Rook")
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
        }
    }
    //Queen and Bishop check
    directions = {POS(1, 1), POS(-1, -1), POS(1, -1), POS(-1, 1)};
    for (POS dir : directions)
    {
        POS temp = pos;
        while (true)
        {
            temp.first += dir.first;
            temp.second += dir.second;
            SquareState state = CheckSquare(temp, color);
            if (state == SquareState::INVALID)
            {
                break;
            }
            else if (state == SquareState::EMPTY)
            {
                continue;
            }
            else if (state == SquareState::TAKEN_BY_FRIENDLY)
            {
                break;
            }
            else if (state == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(temp);
                if (p->GetName() == "Queen" || p->GetName() == "Bishop")
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
        }
    }
    //Knight check
    directions = {POS(2, 1), POS(2, -1), POS(-2, 1), POS(-2, -1), POS(1, 2), POS(1, -2), POS(-1, 2), POS(-1, -2)};
    for (POS dir : directions)
    {
        SquareState state = CheckSquare(dir, color);
        if (state == SquareState::TAKEN_BY_ENEMY)
        {
            Piece* p = GetPiece(dir);
            if (p->GetName() == "Knight")
            {
                return true;
            }
        }
    }
    //Pawn check
    switch (color)
    {
        case Color::WHITE:
            if (CheckSquare(POS(pos.first + 1, pos.second + 1), color) == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(POS(pos.first + 1, pos.second + 1));
                if (p->GetName() == "Pawn")
                {
                    return true;
                }
            }
            if (CheckSquare(POS(pos.first + 1, pos.second - 1), color) == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(POS(pos.first + 1, pos.second - 1));
                if (p->GetName() == "Pawn")
                {
                    return true;
                }
            }
            break;
        case Color::BLACK:
            if (CheckSquare(POS(pos.first - 1, pos.second + 1), color) == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(POS(pos.first - 1, pos.second + 1));
                if (p->GetName() == "Pawn")
                {
                    return true;
                }
            }
            if (CheckSquare(POS(pos.first - 1, pos.second - 1), color) == SquareState::TAKEN_BY_ENEMY)
            {
                Piece* p = GetPiece(POS(pos.first - 1, pos.second - 1));
                if (p->GetName() == "Pawn")
                {
                    return true;
                }
            }
            break;
    }
    //King Check
    directions = {POS(1, 1), POS(1, -1), POS(-1, 1), POS(-1, -1), POS(1, 0), POS(-1, 0), POS(0, 1), POS(0, -1)};
    for (POS dir : directions)
    {
        if (CheckSquare(POS(pos.first + dir.first, pos.second + dir.second), color) == SquareState::TAKEN_BY_ENEMY)
        {
            Piece* p = GetPiece(POS(pos.first + dir.first, pos.second + dir.second));
            if (p->GetName() == "King")
            {
                return true;
            }
        }
    }
    return false;
}

bool Board::IsMoveValid(POS start, POS end, Color color_in) const
{
    Board new_board = Board(*this);
    new_board.Move(start, end);
    POS KingPOS = new_board.GetKingPOS(color_in);
    return !new_board.IsSquareChecked(KingPOS, color_in);
}


POS Board::GetKingPOS(Color color) const
{
    if (color == Color::WHITE)
    {
        return whiteKing;
    }
    else
    {
        return blackKing;
    }
}


void Board::UpdateKingPOS(POS end)
{
    if (board[end.first][end.second]->GetColor() == Color::WHITE)
    {
        whiteKing = end;
    }
    else
    {
        blackKing = end;
    }
}


bool Board::NukeTile(POS pos)
{
    if (board[pos.first][pos.second] != nullptr)
     {
        delete board[pos.first][pos.second];
        board[pos.first][pos.second] = nullptr;
        return true;
    }
    return false;
}

SquareState Board::CheckSquare(POS pos, Color color) const
{
    if (pos.first < 0 || pos.first > 7 || pos.second < 0 || pos.second > 7)
    {
        return SquareState::INVALID;
    }
    else if (board[pos.first][pos.second] == nullptr)
    {
        return SquareState::EMPTY;
    }
    else if (board[pos.first][pos.second]->GetColor() != color)
    {
        return SquareState::TAKEN_BY_ENEMY;
    }
    else if (board[pos.first][pos.second]->GetColor() == color)
    {
        return SquareState::TAKEN_BY_FRIENDLY;
    }
    else
    {
        //Should never reach here, compiler is crying
        return SquareState::INVALID;
    }
}

POS Board::GetEnPassant() const
{
    return enPassant;
}

void Board::CreateBoard()
{
    for (int i = 0; i < 8; i++)
    {   
        board[1][i] = new Pawn(Color::BLACK);
        board[6][i] = new Pawn(Color::WHITE);
    }
    board[0][0] = new Rook(Color::BLACK);
    board[0][7] = new Rook(Color::BLACK);
    board[0][1] = new Knight(Color::BLACK);
    board[0][6] = new Knight(Color::BLACK);
    board[0][2] = new Bishop(Color::BLACK);
    board[0][5] = new Bishop(Color::BLACK);
    board[0][3] = new Queen(Color::BLACK);
    board[0][4] = new King( Color::BLACK);
    blackKing = POS(0,4);

    board[7][0] = new Rook(Color::WHITE);
    board[7][7] = new Rook(Color::WHITE);
    board[7][1] = new Knight(Color::WHITE);
    board[7][6] = new Knight(Color::WHITE);
    board[7][2] = new Bishop(Color::WHITE);
    board[7][5] = new Bishop(Color::WHITE);
    board[7][3] = new Queen(Color::WHITE);
    board[7][4] = new King(Color::WHITE);
    whiteKing = POS(7,4);
}

void Board::PrintBoard() const
{   
    for (ROW row : board)
    {
        for (Piece* p : row)
        {
            if (p == nullptr)
            {
                cout << "0 ";
            }
            else
            {
                cout << p->Info();
            }
        }
        cout << endl;
    }
}

Piece* Board::GetPiece(POS pos) const
{
    return board[pos.first][pos.second];
}