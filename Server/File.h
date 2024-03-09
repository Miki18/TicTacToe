#pragma once

//here we will do everything what references to file

struct Players   //declare all necessary variables for player
{
public:
	int Index;  //when we read data from file, we give everyone Index = line in file -> first line will Index = 0, second have Index = 1 etc
	std::string nick;
	std::string password;
	int win_number;
	int lose_number;
	bool IsLogged;   //player can't log in 2 times at the same time
	int WantPlay;   //-2 when player doesn't want to play multiplayer game or enemy's index when he wants or -1 when he is waiting in queue
};

void ReadFromFile(std::vector<Players>& PlayersData)
{
	std::fstream data;
	data.open("Data.txt", std::ios::in | std::ios::out | std::ios::app);   //we open file. If file doesn't exist we create a new file.

	int line_number = 0;
	std::string line;
	while (!data.eof())    //data schema is: nick(string)[spacebar]password(string)[spacebar]win_number(integer)[spacebar]lose_number(integer)[end line]
	{
		if (std::getline(data, line).fail())     //eof detects the end of file (data ends). But when file is empty (for example we just created the file) eof will not work.
		{										//That's why we need to check if that first getline was correct (if we aren't trying to read from empty file).
			break;							   //.fail() return true when something bad happen and then we use break to stop the whole read file procedure
		}

		int line_position = 0;
		std::string nick;
		while (line[line_position] != char(32))   //read nick - we read till we got spacebar
		{
			nick += line[line_position];
			line_position++;                     //we move one positon forward
		}

		line_position++;                //we move to the next position (we don't want to read space)
		std::string password;
		while (line[line_position] != char(32))  //read password - we read till we got spacebar
		{
			password += line[line_position];
			line_position++;
		}

		line_position++;

		std::string number;
		while (line[line_position] != char(32))    //read number of wins - we read till we got spacebar
		{
			number += line[line_position];
			line_position++;
		}
		int win_number = stoi(number);

		line_position++;
		number.clear();
		while (line_position <= line.length())    //read number of losses - because this is the last data in out line, we read till we reach the end of line
		{
			number += line[line_position];
			line_position++;
		}
		int lose_number = stoi(number);

		PlayersData.push_back({line_number, nick, password, win_number, lose_number, false, -2});   //push_back to the struct
		line_number++;                        //we increase the line number
	}
	data.close();    //we close data, because we dont' need it anymore
}

void SaveFile(std::vector<Players> PlayersData)   //we save data to file
{
	std::fstream data;
	data.open("Data.txt", std::ios::out);         //we open new file titled Data.txt

	std::string WritingLine;
	for (int i = 0; i < PlayersData.size(); i++)   //for every player in vector
	{
		WritingLine.clear();    //clear string
		//we collect all data and write them in specific order
		WritingLine = WritingLine + PlayersData[i].nick + ' ' + PlayersData[i].password + ' ' + std::to_string(PlayersData[i].win_number) + ' ' + std::to_string(PlayersData[i].lose_number);
		data << WritingLine << std::endl;
	}
}