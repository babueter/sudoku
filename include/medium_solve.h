/*
 * Program: SuDoku Killer
 *
 * Functions for medium difficulty solving methods of the SuDoku puzzles.
 *
 * guessOneCell
 * ------------
 * For one section, guess every possible value, then fill in the cross section
 * cells and see if a solution can be found.  If any number has only one cell
 * that yields a solution, return that address and value.
 *
 * fillBoard
 * ---------
 * Fill the entire board with a specified number.  If a solution is found, it then
 * tallys the location of each number by cell.  The goal is for the calling program
 * to then determine if any number could only exist in one cell for all solutions.
 *
 */

int cross_sections[5];

c_address guessOneCell (SuDoku*, int, int*);
c_address nextCrossSectionCell (SuDoku*, int, int, int);

void fillBoard (SuDoku*, c_address, int, SuDoku*, int*);
void fillSection (SuDoku*, c_address, int, int, int*);
void adjustIndex (int, int*);

c_address guessOneCell (SuDoku *board, int section, int *solutions) {
  SuDoku *newBoard;
  c_address nextmove, theMove;
  int values_available[9], values_count=0;
  int i,j, start_index, num_solutions;
  int cells_array[3];

  if (section%3 == section || section == 3) {
    cross_sections[0] = 1;
    cross_sections[1] = 2;
    cross_sections[2] = 3;
    cross_sections[3] = section+3;
    cross_sections[4] = section+6;

  } else if (section%6 == section || section == 6 ) {
    cross_sections[0] = section-3;
    cross_sections[1] = 4;
    cross_sections[2] = 5;
    cross_sections[3] = 6;
    cross_sections[4] = section+3;

  } else {
    cross_sections[0] = section-6;
    cross_sections[1] = section-3;
    cross_sections[2] = 7;
    cross_sections[3] = 8;
    cross_sections[4] = 9;
  }

  for (i=1; i<=9; i++) {
    if (valueAvailable(board, section, i) == 0) {
      values_available[values_count] = i;
      values_count++;
    }
  }

  newBoard = malloc(sizeof(SuDoku));
  for (i=0; i<values_count; i++) {
    num_solutions=0;
    for (j=1; j<=9; j++) {
      if (board->section[section].cell[j] == 0) {
        *solutions=0;

        memcpy(newBoard, board, sizeof(SuDoku));
        newBoard->section[section].cell[j] = values_available[i];
        nextmove = nextCrossSectionCell(newBoard, cross_sections[0], 1, values_available[i]);
        if (nextmove.section != 0) {
          adjustIndex(nextmove.section, &start_index);
          fillSection (newBoard, nextmove, values_available[i], start_index, solutions);

          if (*solutions > 0) {
            theMove.section = section;
            theMove.cell = j;
            theMove.value = values_available[i];

            num_solutions++;
          }
        }
      }
    }
    if (num_solutions == 1) {
      return(theMove);
    }
  }

  theMove.section = 0;
  theMove.cell = 0;
  theMove.value = 0;
  return(theMove);
}

void fillBoard (SuDoku *board, c_address curCell, int value, SuDoku *tally, int *solutions) {
  SuDoku *newBoard;
  c_address nextMove;
  int nextcellopen;
  int i,j;

  if (valueAvailable(board, curCell.section, value) != 0) {
    nextMove = nextCell(board, curCell.section+1, 1);
    if (nextMove.section == 0) {
      if (checkRules(board) == 0) {
        for (i=1; i<=9; i++) {
          for (j=1; j<=9; j++) {
            if (board->section[i].cell[j] == value) {
              (tally->section[i].cell[j])++;
              j=10;
            }
          }
        }
        (*solutions)++;
        return;
      }
    }

    fillBoard(board, nextMove, value, tally, solutions);

  } else {
    nextcellopen = anotherCellOpen(board, curCell);
    if (nextcellopen != 0) {
      nextMove = curCell;
      nextMove.cell = nextcellopen;

      newBoard = malloc(sizeof(SuDoku));
      memcpy(newBoard, board, sizeof(SuDoku));
      fillBoard(newBoard, nextMove, value, tally, solutions);
      free(newBoard);
    }

    board->section[curCell.section].cell[curCell.cell] = value;
    if (checkRules(board) != 0) {
      return;
    }
    nextMove = nextCell(board, curCell.section+1, 1);
    if ( nextMove.section == 0 ) {
      if (checkRules(board) == 0) {
        for (i=1; i<=9; i++) {
          for (j=1; j<=9; j++) {
            if (board->section[i].cell[j] == value) {
              (tally->section[i].cell[j])++;
              j=10;
            }
          }
        }
        (*solutions)++;
      }
      return;
    }

    fillBoard(board, nextMove, value, tally, solutions);
  }

  return;
}

c_address nextCrossSectionCell (SuDoku *board, int section, int cell, int value) {
  int i,j,k,l=cell;
  c_address theCell;

  i=0;
  while (cross_sections[i] != section) {
    i++;
  }
  for (j=i; j<5; j++) {
    if (valueAvailable(board, section, value) == 0) {
      if (j != i ) {l = 0;}
      for (k=l; k<=9; k++) {
        if (board->section[cross_sections[j]].cell[k] == 0) {
          theCell.section = cross_sections[j];
          theCell.cell = k;
          return(theCell);
        }
      }
    }
  }

  theCell.section = 0;
  theCell.cell = 0;
  return(theCell);
}

void fillSection (SuDoku *board, c_address curCell, int value, int index, int *solutions) {
  SuDoku *newBoard;
  c_address nextMove;
  int nextcellopen;

  nextcellopen = anotherCellOpen(board, curCell);
  if (nextcellopen != 0) {
    nextMove = curCell;
    nextMove.cell = nextcellopen;

    newBoard = malloc(sizeof(SuDoku));
    memcpy(newBoard, board, sizeof(SuDoku));
    fillSection(newBoard, nextMove, value, index, solutions);
    free(newBoard);
  }

  board->section[curCell.section].cell[curCell.cell] = value;
  if (checkRules(board) != 0) {
    return;
  }

  if ( index == 4 ) {
    if (checkRules(board) == 0) {
      (*solutions)++;
    }
    return;
  }
  nextMove = nextCrossSectionCell(board, cross_sections[index+1], 1, value);
  if ( nextMove.section == 0 ) {
    if (checkRules(board) == 0) {
      (*solutions)++;
      return;
    }
  }

  adjustIndex(nextMove.section, &index);
  fillSection(board, nextMove, value, index, solutions);

  return;
}

void adjustIndex (int section, int *index) {
  int i;

  for (i=0; i<5; i++) {
    if (cross_sections[i] == section) { *index = i; return; }
  }
}

