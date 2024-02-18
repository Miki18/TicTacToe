#pragma once

extern char GameBoardStatus[3][3];

int BoardCheck()    //return 0 means no winner, 1 - X wins, 2 - O wins
{
    for (int i = 0; i < 3; i++)    //check rows and columns
    {
        if (GameBoardStatus[i][0] == 'X' and GameBoardStatus[i][1] == 'X' and GameBoardStatus[i][2] == 'X')  //check rows
        {
            return 1;
        }

        if (GameBoardStatus[i][0] == 'O' and GameBoardStatus[i][1] == 'O' and GameBoardStatus[i][2] == 'O')
        {
            return 2;
        }

        if (GameBoardStatus[0][i] == 'X' and GameBoardStatus[1][i] == 'X' and GameBoardStatus[2][i] == 'X')  //check colums
        {
            return 1;
        }

        if (GameBoardStatus[0][i] == 'O' and GameBoardStatus[1][i] == 'O' and GameBoardStatus[2][i] == 'O')
        {
            return 2;
        }
    }

    if ((GameBoardStatus[0][0] == 'X' and GameBoardStatus[1][1] == 'X' and GameBoardStatus[2][2] == 'X') or (GameBoardStatus[0][2] == 'X' and GameBoardStatus[1][1] == 'X' and GameBoardStatus[2][0] == 'X'))  //check diagonal
    {
        return 1;
    }
    else if ((GameBoardStatus[0][0] == 'O' and GameBoardStatus[1][1] == 'O' and GameBoardStatus[2][2] == 'O') or (GameBoardStatus[0][2] == 'O' and GameBoardStatus[1][1] == 'O' and GameBoardStatus[2][0] == 'O'))
    {
        return 2;
    }
    else
    {
        return 0;   //there is no winner
    }
}

void ResetGameBoard()
{
    for (int i = 0; i < 3; i++)
    {
        for (int a = 0; a < 3; a++)
        {
            GameBoardStatus[i][a] = '?';
        }
    }
}

int HardAiAlgorithm(char Mark)   //returns 0 or 1. 1 means that AI did something and it doesn't do random move. 0 means it didn't do anything and we are going to do random move
{
    //check if it can win istant
    for (int i = 0; i < 3; i++)
    {
        if (GameBoardStatus[i][0] == Mark and GameBoardStatus[i][1] == Mark)
        {
            if (GameBoardStatus[i][2] == '?')   //second check if that place is empty
            {
                GameBoardStatus[i][2] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[i][1] == Mark and GameBoardStatus[i][2] == Mark)
        {
            if (GameBoardStatus[i][0] == '?')
            {
                GameBoardStatus[i][0] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[i][0] == Mark and GameBoardStatus[i][2] == Mark)
        {
            if (GameBoardStatus[i][1] == '?')
            {
                GameBoardStatus[i][1] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[0][i] == Mark and GameBoardStatus[1][i] == Mark)
        {
            if (GameBoardStatus[2][i] == '?')
            {
                GameBoardStatus[2][i] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[1][i] == Mark and GameBoardStatus[2][i] == Mark)
        {
            if (GameBoardStatus[0][i] == '?')
            {
                GameBoardStatus[0][i] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[0][i] == Mark and GameBoardStatus[2][i] == Mark)
        {
            if (GameBoardStatus[1][i] == '?')
            {
                GameBoardStatus[1][i] = Mark;
                return 1;
            }
        }
    }

    if (GameBoardStatus[0][0] == Mark and GameBoardStatus[1][1] == Mark)
    {
        if (GameBoardStatus[2][2] == '?')
        {
            GameBoardStatus[2][2] = Mark;
            return 1;
        }
    }
    
    if (GameBoardStatus[1][1] == Mark and GameBoardStatus[2][2] == Mark)
    {
        if (GameBoardStatus[0][0] = '?')
        {
            GameBoardStatus[0][0] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[0][0] == Mark and GameBoardStatus[2][2] == Mark)
    {
        if (GameBoardStatus[1][1] == '?')
        {
            GameBoardStatus[1][1] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[0][2] == Mark and GameBoardStatus[2][0] == Mark)
    {
        if (GameBoardStatus[1][1] == '?')
        {
            GameBoardStatus[1][1] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[1][1] == Mark and GameBoardStatus[2][0] == Mark)
    {
        if (GameBoardStatus[0][2] == '?')
        {
            GameBoardStatus[0][2] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[1][1] == Mark and GameBoardStatus[0][2] == Mark)
    {
        if (GameBoardStatus[2][0] == '?')
        {
            GameBoardStatus[2][0] = Mark;
            return 1;
        }
    }

    //check if AI can block something
    char enemyMark = 'X';      //calculate enemy mark
    if (Mark == 'X')
    {
        enemyMark = 'O';
    }
    else
    {
        enemyMark = 'X';
    }

    for (int i = 0; i < 3; i++)
    {
        if (GameBoardStatus[i][0] == enemyMark and GameBoardStatus[i][1] == enemyMark)
        {
            if (GameBoardStatus[i][2] == '?')
            {
                GameBoardStatus[i][2] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[i][1] == enemyMark and GameBoardStatus[i][2] == enemyMark)
        {
            if (GameBoardStatus[i][0] == '?')
            {
                GameBoardStatus[i][0] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[i][0] == enemyMark and GameBoardStatus[i][2] == enemyMark)
        {
            if (GameBoardStatus[i][1] == '?')
            {
                GameBoardStatus[i][1] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[0][i] == enemyMark and GameBoardStatus[1][i] == enemyMark)
        {
            if (GameBoardStatus[2][i] == '?')
            {
                GameBoardStatus[2][i] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[1][i] == enemyMark and GameBoardStatus[2][i] == enemyMark)
        {
            if (GameBoardStatus[0][i] == '?')
            {
                GameBoardStatus[0][i] = Mark;
                return 1;
            }
        }

        if (GameBoardStatus[0][i] == enemyMark and GameBoardStatus[2][i] == enemyMark)
        {
            if (GameBoardStatus[1][i] == '?')
            {
                GameBoardStatus[1][i] = Mark;
                return 1;
            }
        }
    }

    if (GameBoardStatus[0][0] == enemyMark and GameBoardStatus[1][1] == enemyMark)
    {
        if (GameBoardStatus[2][2] == '?')
        {
            GameBoardStatus[2][2] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[1][1] == enemyMark and GameBoardStatus[2][2] == enemyMark)
    {
        if (GameBoardStatus[0][0] == '?')
        {
            GameBoardStatus[0][0] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[0][0] == enemyMark and GameBoardStatus[2][2] == enemyMark)
    {
        if (GameBoardStatus[1][1] == '?')
        {
            GameBoardStatus[1][1] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[0][2] == enemyMark and GameBoardStatus[2][0] == enemyMark)
    {
        if (GameBoardStatus[1][1] == '?')
        {
            GameBoardStatus[1][1] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[1][1] == enemyMark and GameBoardStatus[2][0] == enemyMark)
    {
        if (GameBoardStatus[0][2] == '?')
        {
            GameBoardStatus[0][2] = Mark;
            return 1;
        }
    }

    if (GameBoardStatus[1][1] == enemyMark and GameBoardStatus[0][2] == enemyMark)
    {
        if (GameBoardStatus[2][0] == '?')
        {
            GameBoardStatus[2][0] = Mark;
            return 1;
        }
    }

    return 0; //if AI did nothing
}

void AiRandomMove(char Mark)
{
    int filled = 0; //we need to control if we already put mark on the board or not    0 - not   1 - yes
    int a = 0;
    int b = 0;

    for (int i = 0; i < 30; i++)    //we do random 30 times
    {
        b = rand() % 3;
        a = rand() % 3;
        if (GameBoardStatus[b][a] == '?')
        {
            GameBoardStatus[b][a] = Mark;
            filled = 1;
            break;
        }
    }
    
    //If random doesn't work we have to do it manually
    if(filled == 0)                      //check in a and b
    {
        for (int i = 0; i < 3; i++)
        {
            if (GameBoardStatus[i][a] == '?')
            {
                GameBoardStatus[i][a] = Mark;
                filled = 1;
                break;
            }
        }
    }

    if (filled == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            if (GameBoardStatus[b][i] == '?')
            {
                GameBoardStatus[b][i] = Mark;
                filled = 1;
                break;
            }
        }
    }

    //if nothing works then we have to check the whole board and find empty place
    if (filled == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int z = 0; z < 2; z++)
            {
                if (GameBoardStatus[i][z] == '?')
                {
                    GameBoardStatus[i][z] = Mark;
                    filled = 1;
                    break;
                }
            }

            if (filled == 1)
            {
                break;
            }
        }
    }
}