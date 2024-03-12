# TicTacToe

## Introduction 
<b>Simple TicTacToe Game in C++</b></p><br>
Hi, this is my Simple Multiplayer TicTacToe game. You can play single player mode (2 difficulty levels) or multiplayer.<br> Program was written in C++ for Windows (tested on windows 10 and 11).

## How to run program
1. Visual Studio (it was originally coded in VS 2022). You will need those libraries:<br>
  <i>-glfw3</i><br>
  <i>-glew</i><br>
  <i>-opengl</i><br>
  <i>-ImGui</i><br>
<i>You should open project with .sln file</i>

2. You can download **Game.zip** file. It contains .exe files (server and client). **Game.zip** also contains database with 3 example accounts.

## Game Mechanics

### Singleplayer
There are 2 difficulty levels:<br>
  <i>-easy</i> - AI just do random move,<br>
  <i>-hard</i> - AI checks if it can finish the game it current turn. If not then AI checks if it has to block. If none of that returns true then AI does the same thing as easy AI; just do random move.<br>
You can also turn on time control (it is turn off by deafult). It set turn's limit on 15 second.

### Multiplayer
  ***How to connect:***<br>
  If you want to play with other player you have to turn on server and connect your clients with it by inserting an server's IP. You have to also be in the same network or use a Hamachi. In case you use Hamachi, you have to insert Hamachi's IP.<br>

  <br>***Basic interactions with account:***<br>
  First, after connecting, you have to login to your account (you have 3 example accounts in your database) or register new account. For creating a new account you need to insert unique nick, password and repeat a password. You are able to change your     password. You are also able to permanently delete your account.You can also see players' stats.<br>

  <br>***Multiplayer Segment:***<br>
We need 2 players for multiplayer game, first you join to queue and wait for other player, who want to play (you can exit whenever you want). When 2 players are waiting the game starts. In multiplayer game there is always time limit (15 seconds per turn). When game ends the points are inserted into database. In case of draw, players don't get any points. If time limit ends - you lose.<br>

### Database
Server collects data and save them when player disconnects. Server has following data:<br>
 <i>-Nick</i><br>
 <i>-Password</i><br>
 <i>-number of wins</i><br>
 <i>-number of loses</i><br>
 Server has vector of structs for these data for easier access, but whenever player disconnects, server saves it to the <i>Data.txt</i> file in special order, so it is able to load them. Server read <i>Data.txt</i> when it starts. If <i>Data.txt</i> doesn't exist then server creates new, empty file.<br>
<br>***Data schema:***<br>
(Player1Nick)(spacebar)(Player1password)(spacebar)(Player1number of wins)(spacebar)(Player1number of loses)<br>
(Player2Nick)(spacebar)(Player2password)(spacebar)(Player2number of wins)(spacebar)(Player2number of loses)<br>
...<br>
That allows to easily import data from one server to another.

<sub>Last update data: 12.03.2024</sub>
