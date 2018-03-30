/*
 * Program: SuDoku Killer
 *
 * Functions for hard and very hard difficulty solving methods of the SuDoku puzzles.
 *
 *
 */

typedef void (*handler_func)(SuDoku*);

c_address findLeastConstrained (SuDoku*, board_constraint*);
c_address nextConstraint (board_constraint*, c_address);


void constraintPropagation (SuDoku *board, board_constraint *constraint, c_address curCell, handler_func solution) {
  SuDoku *newBoard;
  board_constraint *newConstraint;
  c_address nextMove;

  nextMove = nextConstraint(constraint, curCell);
  if (nextMove.section == curCell.section && nextMove.cell == curCell.cell) {
    newBoard = malloc(sizeof(SuDoku));
    memcpy(newBoard, board, sizeof(SuDoku));

    newConstraint = malloc(sizeof(board_constraint));
    memcpy(newConstraint, constraint, sizeof(board_constraint));

    constraintPropagation(newBoard, newConstraint, nextMove, solution);
    free(newBoard);
    free(newConstraint);
  }

  board->section[curCell.section].cell[curCell.cell] = curCell.value;
  if (scanBoard(board, constraint) == 0) {
    solution(board);
    exit(0);
  }
  updateConstraint(board, constraint);
  if (checkConstraintRules(board, constraint) == 1) {
    return;
  }
  nextMove = findLeastConstrained(board, constraint);

  if (nextMove.section != 0 && nextMove.value != 0) {
    constraintPropagation(board, constraint, nextMove, solution);
  }

  return;
}

c_address findLeastConstrained (SuDoku *board, board_constraint *constraint) {
  int i,j,k;
  int sum, first_value, least_sum=9;
  c_address bestMove;

  bestMove.section = 0;
  bestMove.cell = 0;
  bestMove.value = 0;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      sum = 0;
      first_value = 0;
      if (board->section[i].cell[j] == 0) {
        for (k=1; k<=9; k++) {
          if (constraint->section[i].cell[j].value[k] == 1) {
            if (first_value == 0) { first_value = k; }
            sum++;
          }
        }
        if (sum < least_sum) {
          bestMove.section = i;
          bestMove.cell = j;
          bestMove.value = first_value;

          least_sum = sum;
        }
      }
    }
  }

  return(bestMove);
}

c_address nextConstraint (board_constraint *constraint, c_address curCell) {
  int i,j,k;
  c_address theMove;

  i=curCell.section;
  j=curCell.cell;
  k=curCell.value+1;

  while (i<=9) {
    while (j<=9) {
      while (k<=9) {
        if (constraint->section[i].cell[j].value[k] == 1) {
          theMove.section = i;
          theMove.cell = j;
          theMove.value = k;

          return(theMove);
        }
        k++;
      }
      k=1;
      j++;
    }
    j=1;
    i++;
  }

  theMove.section = 0;
  theMove.cell = 0;
  theMove.value = 0;

  return(theMove);
}

c_address findMostConstrained (SuDoku *board, board_constraint *constraint) {
  int i,j,k;
  int sum, first_value, least_sum=0;
  c_address bestMove;

  bestMove.section = 0;
  bestMove.cell = 0;
  bestMove.value = 0;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      sum = 0;
      first_value = 0;
      if (board->section[i].cell[j] == 0) {
        for (k=1; k<=9; k++) {
          if (constraint->section[i].cell[j].value[k] == 1) {
            if (first_value == 0) { first_value = k; }
            sum++;
          }
        }
        if (sum > least_sum) {
          bestMove.section = i;
          bestMove.cell = j;
          bestMove.value = first_value;

          least_sum = sum;
        }
      }
    }
  }

  return(bestMove);
}

