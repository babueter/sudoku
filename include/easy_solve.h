/*
 * Program: SuDoku Killer
 *
 * Functions for easy solving methods of the SuDoku puzzles.
 *
 * scanBoard
 * ---------
 * Scan entire board for cells that can have only one
 * solution.  Modify the original board and return success if entire
 * board is solved.
 *
 * all other functions are private and should not be called externally.
 *
 */

int scanBoard (SuDoku*, board_constraint*);
c_address findConstrainedMove (board_constraint*);

int scanBoard (SuDoku *board, board_constraint *constraint) {
/*
 * This function scans the entire board for cells that can
 * only have one solution.  The board itself is modified with
 * those solutions.  If the board is solved, a zero is returned,
 * a one is returned if it is not.
 *
 */

  c_address move;

  while (oneMoveLeft(board) == 0) {
    updateConstraint(board, constraint);
    move = findConstrainedMove(constraint);

    if (move.value == 0) { return(1); }

    board->section[ move.section ].cell[ move.cell ] = move.value;
  }

  return(0);
}

c_address findConstrainedMove (board_constraint *constraint) {
  int i,j,k;
  int section_count;
  c_address theMove;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      section_count = 0;
      for (k=1; k<=9; k++) {
        if (constraint->section[i].cell[j].value[k] == 1) {
          theMove.section = i;
          theMove.cell = j;
          theMove.value = k;

          section_count++;
        }
      }
      if (section_count == 1) { return(theMove); }
    }
  }

  theMove.section = 0;
  theMove.cell = 0;
  theMove.value = 0;
  return(theMove);
}
