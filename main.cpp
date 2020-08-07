/*
 * This file is part of Connect4 Game Solver <http://connect4.gamesolver.org>
 * Copyright (C) 2017-2019 Pascal Pons <contact@gamesolver.org>
 *
 * Connect4 Game Solver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Connect4 Game Solver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Connect4 Game Solver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Solver.hpp"
#include <iostream>
#include <sys/time.h>

using namespace GameSolver::Connect4;

/**
 * Get micro-second precision timestamp
 * uses unix gettimeofday function
 */
unsigned long long getTimeMicrosec() {
  timeval NOW;
  gettimeofday(&NOW, NULL);
  return NOW.tv_sec * 1000000LL + NOW.tv_usec;
}

/**
 * Main function.
 * Reads Connect 4 positions, line by line, from standard input
 * and writes one line per position to standard output containing:
 *  - score of the position
 *  - number of nodes explored
 *  - time spent in microsecond to solve the position.
 *
 *  Any invalid position (invalid sequence of move, or already won game)
 *  will generate an error message to standard error and an empty line to standard output.
 */
int main(int argc, char** argv) {
  time_t t;
  srand((unsigned) time(&t));

  Solver solver;

  bool weak = false;
  std::string opening_book = "7x6.book";
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == 'w') weak = true;
      else if(argv[i][1] == 'b') {
        if(++i < argc) opening_book = std::string(argv[i]);
      }
    }
  }
//   solver.loadBook(opening_book);

  std::string l;
  std::string line;
  std::string newLine;
  std::string play;

  for(int ply = 1; std::getline(std::cin, l); ply++) {
    Position P;

    newLine = line + l;

    std::cout << "User Playing " << newLine << std::endl;
    unsigned int i = P.play(newLine);
    if(i != newLine.size()) {
      if (P.isWinningMove(newLine[i]-'1')) {
          std::cout << "Winning move!" << std::endl;
          return 0;

      } else {
          std::cout << "User Invalid move " << (P.nbMoves() + 1) << " \"" << newLine << "\"" << std::endl;
      }
    } else {
      line = newLine;
      Position::position_t possible = P.possibleNonLosingMoves();
      int maxScore = Position::MIN_SCORE;
      int bestCols[Position::WIDTH + 1] = {0};
      for(int i = Position::WIDTH; i--;) {
        solver.reset();
        Position P2(P);
        if (P2.isWinningMove(i)) {
          std::cout << line << "." << i + 1 << ", s: Winning" << std::endl;
          bestCols[0] = 1;
          bestCols[1] = i + 1;
          break;
        }
        if(possible & Position::column_mask(i)) {
          P2.playCol(i);
          unsigned long long start_time = getTimeMicrosec();
          int score = -solver.solve(P2, weak);
          unsigned long long end_time = getTimeMicrosec();
          std::cout << line << "." << i + 1 << ", s: " << score << ", t: " << (end_time - start_time) << std::endl;
          if (score > maxScore) {
            maxScore = score;
            bestCols[0] = 1;
            bestCols[1] = i + 1;
          } else if (score == maxScore) {
            bestCols[++bestCols[0]] = i + 1;
          }
        } else {
          std::cout << line << "." << i + 1 << ", s: not possible"  << std::endl;
        }
      }

      // No bestCols found
      if (bestCols[0] == 0) {
        std::cout << "User wins!" << std::endl;
        return 0;
      }

      int r = (rand() % bestCols[0]) + 1;
      play = bestCols[r] + '0';
      line = line + play;
      std::cout << "Computer Playing " << play << std::endl;

      Position P2;
      i = P2.play(line);
      if(i != line.size()) {
        if (P2.isWinningMove(line[i]-'1')) {
            std::cout << "Winning move!" << std::endl;
            return 0;
        } else {
            std::cout << "Computer Invalid move " << (P2.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
            if (bestCols[0] == 0) std::cout << "User wins?" << std::endl;
        }
      }
    }
    char *board = P.printBoard();
    std::cout << board;

  }
}
