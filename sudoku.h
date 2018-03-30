/*
 * Program: SuDoku Killer
 *
 * Objects and structures for assisting in solving the SuDoku puzzles.
 *
 * resetBoard
 * ----------
 * Set all cell values to 0 on the original board.
 *
 * loadBoard
 * ---------
 * Load the board from a text file.
 *
 * printBoard
 * ----------
 * Print board to standard out.  This format is compatible for
 * loadBoard.
 *
 * checkRules
 * ----------
 * Checks board to make sure no rules are broken
 *
 * valueAvailable
 * --------------
 * Test the given cell and value and determine if the move is valid.
 *
 * valueCellAvailable
 * ------------------
 * More specialized test which takes into consideration the row and column
 * values intersecting the cell.
 *
 * determineRow
 * ------------
 * Figures out the indexes for each section and cell that intersect
 * the row that a specified cell is in.
 *
 * determineCol
 * ------------
 * Figures out the indexes for each section and cell that intersect
 * the column that a specified cell is in.
 *
 * oneMoveLeft
 * -----------
 * Check to see there is at least one more move left to make on the board.
 *
 * nextCell
 * --------
 * Finds the next empty cell on the board.
 *
 * nextAvailableCell
 * -----------------
 * Finds the next available cell on the board for a given value.
 *
 */

/* One 3x3 section of the 9x9 SuDoku board */
typedef struct section {
        int cell[10];
} Section;

/* An entire 9x9 board of 3x3 sections */
typedef struct sudoku {
        Section section[10];
} SuDoku;

/* The address of one cell including possible value */
typedef struct c_address {
	int section;
	int cell;
	int value;
} c_address;

/* Possible values a cell could contain, constraint for eliminating possibilities */
typedef struct cell_constraint {
	int value[10];
} cell_constraint;

/* A section of 9 cells and thier constraints */
typedef struct section_constraint {
	cell_constraint cell[10];
} section_constraint;

/* A board of 9 sections containing cell constraints */
typedef struct board_constraint {
	section_constraint section[10];
} board_constraint;

void determineRow(int*, int*, int, int);
void determineCol(int*, int*, int, int);
void printConstraint(SuDoku*, board_constraint*);
void printBoard(SuDoku*);
int valueCellAvailable(SuDoku*, c_address, int);
int checkRowCol(SuDoku*, int, int);

void resetBoard(SuDoku *board) {
/*
 * Set all cell values to 0.  The argument is a pointer to
 * the board and any existing value will be erased.
 *
 */

  int i,j;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      board->section[i].cell[j] = 0;
    }
  }
}

void resetConstraint( board_constraint *constraint) {
/*
 * Sets all values to true for a given constraint.
 *
 */

  int i,j,k;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      for (k=1; k<=9; k++) {
        constraint->section[i].cell[j].value[k] = 1;
      }
    }
  }
}

void updateConstraint(SuDoku *board, board_constraint *constraint) {
/*
 * Update the constraint values based on the current board.
 *
 */

  int i,j,k,l,m;
  int row_sections[3],row_cells[3];
  int col_sections[3],col_cells[3];
  int section, section_count, available, intersect;
  int subset_cells[9];
  c_address thisCell;

 /* Eliminate naked singles */
  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      if (board->section[i].cell[j] != 0) {
        for (k=1; k<=9; k++) {
          constraint->section[i].cell[j].value[k] = 0;
        }
      } else {
        for (k=1; k<=9; k++) {
          thisCell.section = i;
          thisCell.cell = j;
          if (valueCellAvailable(board, thisCell, k) != 0) {
            constraint->section[i].cell[j].value[k] = 0;
          }
        }
      }
    }
  }

 /* Reduce Line/Box Intersections */
  for (i=1; i<=9; i+=4)
    for (j=1; j<=9; j+=4) {
      determineRow(&row_sections[0], &row_cells[0], i, j);
      determineCol(&col_sections[0], &col_cells[0], i, j);

      for (k=1; k<=9; k++) {	/* each number */

       /* Find rows where number can only exist in one section.  Then update constraints for that section */
        section = 0;
        section_count = 0;
        for (l=0; l<3; l++) {		/* each row and column section */
          available = 0;
          for (m=0; m<3; m++) {			/* each row and column cell */
            if (constraint->section[ row_sections[l] ].cell[ row_cells[m] ].value[k] == 1) {
              available = 1;
              section = row_sections[l];
            }
          }
          if (available == 1) {section_count++;}
        }
        if (section_count == 1) {
          for (l=1;l<=9;l++) {
            intersect = 0;
            for (m=0;m<3;m++) {
              if (l == row_cells[m]) { intersect = 1; }
            }
            if (intersect != 1) { constraint->section[ section ].cell[l].value[k] = 0; }
          }
        }

       /* Find cols where number can only exist in one section.  Then update constraints for that section */
        section = 0;
        section_count = 0;
        for (l=0; l<3; l++) {		/* each row and column section */
          available = 0;
          for (m=0; m<3; m++) {			/* each row and column cell */
            if (constraint->section[ col_sections[l] ].cell[ col_cells[m] ].value[k] == 1) {
              available = 1;
              section = col_sections[l];
            }
          }
          if (available == 1) {section_count++;}
        }
        if (section_count == 1) {
          for (l=1;l<=9;l++) {
            intersect = 0;
            for (m=0;m<3;m++) {
              if (l == col_cells[m]) { intersect = 1; }
            }
            if (intersect != 1) { constraint->section[ section ].cell[l].value[k] = 0; }
          }
        }

      }
    }

 /* Hidden subsets or hidden pair */
  for (i=1; i<=9; i++) {	/* each section */
    for (j=1; j<9; j++) {		/* each first value */
      for (k=j+1; k<=9; k++) {			/* each paired value */
        section_count = 0;
        for (l=1; l<=9; l++) {				/* each cell */
          if (constraint->section[i].cell[l].value[j] == 1 && constraint->section[i].cell[l].value[k] == 1) {
            subset_cells[section_count] = l;
            section_count++;
          } else if (constraint->section[i].cell[l].value[j] == 1 || constraint->section[i].cell[l].value[k] == 1) {
            section_count = 3;
          }
        }
        if (section_count == 2) {
          for (l=1; l<=9; l++) {
            if (l != j && l != k) {
              constraint->section[i].cell[ subset_cells[0] ].value[l] = 0;
              constraint->section[i].cell[ subset_cells[1] ].value[l] = 0;
            }
          }
        }
      }
    }
  }

 /* More naked single reduction after line/box is evaluated */
  for (i=1; i<=9; i++) {	/* each section */
    for (j=1; j<=9; j++) {		/* each value */
      section_count = 0;
      for (k=1; k<=9; k++) {			/* each cell */
        if (constraint->section[i].cell[k].value[j] == 1) {
          section_count++;
          thisCell.section = i;
          thisCell.cell = k;
        }
      }
      if (section_count == 1) {
        for (k=1; k<=9; k++) {
          constraint->section[i].cell[ thisCell.cell ].value[k] = 0;
        }
        constraint->section[i].cell[ thisCell.cell ].value[j] = 1;
      }
    }
  }
}

void printConstraint(SuDoku *board, board_constraint *constraint) {
  int i,j,k,l,m;
  int col_spacing;

  for (i=1; i<=7; i+=3) {
    printf ("-------------------------------------------------------------------\n");
    for (j=1; j<=7; j+=3) {
      printf("|");
      for (k=i; k<i+3; k++) {
        for (l=j; l<j+3; l++) {
          if (board->section[k].cell[l] != 0) {
            printf ("  %d    ",board->section[k].cell[l]);
          } else {
            col_spacing=7;
            for (m=1; m<=9; m++) {
              if (constraint->section[k].cell[l].value[m] == 1) {
                printf("%d",m);
                col_spacing--;
              }
            }
            for (m=col_spacing; m>0; m--) {
              printf(" ");
            }
          }
        }
        printf("|");
      }
      printf("\n");
    }
  }
  printf ("-------------------------------------------------------------------\n");

}

void loadBoard(SuDoku *board, char *fname) {
/*
 * Assumes the pointer to the board has be reset.  This
 * function reads in values from "boards.txt" which assumes
 * the file has exactly the correct number of cells represented.
 * Empty cells are denoted by 0.  This will load the incomplete
 * board into the board passed as a pointer.
 *
 * This function tries to be somewhat fault tolerant, however,
 * if the board.txt file is messy, expect this function to fail
 * miserably.
 *
 */

  int i,j,k,l;

  FILE *fin;
  int value;

  if ( strncmp("-", fname, 1) == 0) {
    fin = fdopen(dup(STDIN_FILENO), "r");
  } else {
    fin = fopen(fname, "rb");
  }
  if ( fin == NULL ) {
    printf ("Error: could not open '%s'\n", fname);
    exit(1);
  }

  /*
   * I'm really sorry about this, but I had a problem reading in
   * via fscanf and so i gave up and read the values in as ascii
   * and converted them.
   *
   */
  for (i=1; i<=7; i+=3) {
    for (j=1; j<=7; j+=3) {
      for (k=i; k<i+3; k++) {
        for (l=j; l<j+3; l++) {
          value = fgetc(fin);
          while ( (value != '_' && value != '.') && (value < 48 || value > 57) && value != EOF) {
            value = fgetc(fin);
          }
          if ( value == EOF ) { printf ("Error: board.txt corrupt\n"); exit(1); }
          if ( value == '_' || value == '.' ) { value = 48; }

          board->section[k].cell[l] = value - 48;
        }
      }
    }
  }

  value = fgetc(fin);
  while (value != EOF ) {
    if ( value > 47 && value < 58 ) {
      printf ("Error: file '%s' corrupt\n", fname);
      exit(1);
    }
    value = fgetc(fin);
  }
  fclose(fin);
}

int loadBenchmark(SuDoku *board, FILE *fin) {
  int i,j,k,l;
  int value;

  for (i=1; i<=7; i+=3) {
    for (j=1; j<=7; j+=3) {
      for (k=i; k<i+3; k++) {
        for (l=j; l<j+3; l++) {
          value = fgetc(fin);
          while ( (value != '_' && value != '.') && (value < 48 || value > 57) && value != EOF) {
            value = fgetc(fin);
          }
          if ( value == EOF ) {
            if (i != 1) {
              printf ("Error: benchmark file corrupt\n");
              exit(1);
            } else { return(1); }
          }
          if ( value == '_' || value == '.' ) { value = 48; }

          board->section[k].cell[l] = value - 48;
        }
      }
    }
  }
  value = fgetc(fin);
  if (value == EOF) { return (1); }

  return(0);
}

void printBoard(SuDoku *board) {
/*
 * Quite simply, print the board, passed via the pointer.
 * No changes are made to the board itself.
 *
 */

  int i,j,k,l;

  for (i=1; i<=7; i+=3) {
    printf ("-------------------------------\n");
    for (j=1; j<=7; j+=3) {
      printf("|");
      for (k=i; k<i+3; k++) {
        for (l=j; l<j+3; l++) {
          if (board->section[k].cell[l] == 0) {
            printf(" _ ");
          } else {
            printf(" %d ",board->section[k].cell[l]);
          }
        }
        printf("|");
      }
      printf("\n");
    }
  }
  printf ("-------------------------------\n");
}

void printBoardBenchmark(SuDoku *board) {
  int i,j,k,l;

  for (i=1; i<=7; i+=3) {
    for (j=1; j<=7; j+=3) {
      for (k=i; k<i+3; k++) {
        for (l=j; l<j+3; l++) {
          if (board->section[k].cell[l] == 0) {
            printf(".");
          } else {
            printf("%d",board->section[k].cell[l]);
          }
        }
      }
    }
  }
  printf("\n");
}

int checkRules (SuDoku *board) {
/*
 * Validate that there are no broken rules on the board.  The board
 * does not have to be completed.  Empty cells are denoted with a
 * value of zero.  Nothing is changed on the board.
 *
 */

  int i,j,k;

 /* Check within each section to determine if a nubmer exists more then once */
  for (i=1; i<=9; i++) {
    for (j=1; j<9; j++) {
      for (k=j+1; k<=9; k++) {
        if (board->section[i].cell[j] == board->section[i].cell[k] && board->section[i].cell[j] != 0) {
          return(1);
        }
      }
    }
  }

 /* Check within each column and row using the checkRowCol function */
  for (i=1; i<=9; i+=4) {
    for (j=1; j<=9; j+=4) {
      if (checkRowCol(board, i, j) != 0) {
        return(1);
      }
    }
  }

  return(0);
}

int checkConstraintRules (SuDoku *board, board_constraint *constraint) {
  int i,j,k,l,m;
  int row_sections[3], row_cells[3];
  int col_sections[3], col_cells[3];
  int sum,available;

 /*
  * Find cells that are constrained to no options, and sections that are
  * missing numbers due to constraints
  */
  for (i=1; i<=9; i++) {
    for (j=1; j<9; j++) {
      sum = 0;
      available = 0;
      for (k=1; k<=9; k++) {
        if (board->section[i].cell[j] == 0) {
          if (constraint->section[i].cell[j].value[k] == 1) { sum++; }
        } else {
          sum = 1;
        }

        if (board->section[i].cell[k] == j) {
          available = 1;
        } else {
          if (constraint->section[i].cell[k].value[j] == 1) { available = 1; }
        }
      }
      if (sum == 0) {		/* Cell j had no values available */
        return(1);
      }
      if (available == 0) {	/* Value j was not available in section i */
        return(1);
      }
    }
  }

 /* Find rows and columns that are missing values as options due to constraints */
  for (i=1; i<=9; i+=4) {
    for (j=1; j<=9; j+=4) {
      determineRow(&row_sections[0], &row_cells[0], i, j);
      determineCol(&col_sections[0], &col_cells[0], i, j);

      for (k=1; k<=9; k++) {
        available=0;
        l=0;
        while (available == 0 && l<3) {
          m=0;
          while (available == 0 && m<3) {
            if (board->section[ row_sections[l] ].cell[ row_cells[m] ] == k || constraint->section[ row_sections[l] ].cell[ row_cells[m] ].value[k] == 1) {
              available = 1;
            }
            m++;
          }
          l++;
        }
      }
      if (available == 0) {
        return(1);
      }

      for (k=1; k<=9; k++) {
        available=0;
        l=0;
        while (available == 0 && l<3) {
          m=0;
          while (available == 0 && m<3) {
            if (board->section[ col_sections[l] ].cell[ col_cells[m] ] == k || constraint->section[ col_sections[l] ].cell[ col_cells[m] ].value[k] == 1) {
              available = 1;
            }
            m++;
          }
          l++;
        }
      }
      if (available == 0) {
        return(1);
      }
    }
  }
  return(0);
}

void determineRow(int *sections_array, int *cells_array, int section, int cell) {
/*
 * Set up two arrays, one with section values that exist in the same row as
 * the section,cell passed, the other with the cells.
 *
 */

if (section > 9 || section < 1 || cell > 9 || cell < 1) {
printf("called at (%d, %d)\n",section, cell);
}
  if (section%3 == section || section == 3) {
    *sections_array++ = 1;
    *sections_array++ = 2;
    *sections_array++ = 3;

  } else if (section%6 == section || section == 6) {
    *sections_array++ = 4;
    *sections_array++ = 5;
    *sections_array++ = 6;

  } else {
    *sections_array++ = 7;
    *sections_array++ = 8;
    *sections_array++ = 9;
  }

  if (cell%3 == cell || cell == 3) {
    *cells_array++ = 1;
    *cells_array++ = 2;
    *cells_array++ = 3;

  } else if (cell%6 == cell || cell == 6) {
    *cells_array++ = 4;
    *cells_array++ = 5;
    *cells_array++ = 6;

  } else {
    *cells_array++ = 7;
    *cells_array++ = 8;
    *cells_array++ = 9;
  }

}

void determineCol(int *sections_array, int *cells_array, int section, int cell) {
/*
 * Set up two arrays, one with section values that exist in the same column as
 * the section,cell passed, the other with the cells.
 *
 */

if (section > 9 || section < 1 || cell > 9 || cell < 1) {
printf("called at (%d, %d)\n",section, cell);
}
  if (section%3 == section || section == 3) {
    *sections_array++ = section;
    *sections_array++ = section+3;
    *sections_array++ = section+6;

  } else if (section%6 == section || section == 6) {
    *sections_array++ = section-3;
    *sections_array++ = section;
    *sections_array++ = section+3;

  } else {
    *sections_array++ = section-6;
    *sections_array++ = section-3;
    *sections_array++ = section;

  }

  if (cell%3 == cell || cell == 3) {
    *cells_array++ = cell;
    *cells_array++ = cell+3;
    *cells_array++ = cell+6;

  } else if (cell%6 == cell || cell == 6) {
    *cells_array++ = cell-3;
    *cells_array++ = cell;
    *cells_array++ = cell+3;

  } else {
    *cells_array++ = cell-6;
    *cells_array++ = cell-3;
    *cells_array++ = cell;
  }

}

int checkRowCol(SuDoku *board, int section, int cell) {
/*
 * This function check one cell to determine if the column and row it
 * exists on is valid for the solution.  This works on incomplete boards
 * as well.  Empty cells denoted by a zero value.  Nothing is changed on
 * the board.  integer 0 is returned for success, 1 for failure.
 *
 */

  c_address rows[9], cols[9], tmpcell;
  int i,j,k;
  int sections_row[3], cells_row[3];
  int sections_col[3], cells_col[3];

  determineRow(&sections_row[0], &cells_row[0], section, cell);
  determineCol(&sections_col[0], &cells_col[0], section, cell);

  k=0;
  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      tmpcell.section = sections_row[i];
      tmpcell.cell = cells_row[j];
      tmpcell.value = board->section[sections_row[i]].cell[cells_row[j]];
      rows[k] = tmpcell;

      tmpcell.section = sections_col[i];
      tmpcell.cell = cells_col[j];
      tmpcell.value = board->section[sections_col[i]].cell[cells_col[j]];
      cols[k] = tmpcell;

      k++;
    }
  }

  for (i=0; i<8; i++) {
    for (j=i+1; j<9; j++) {
      if (cols[i].value != 0 && cols[j].value != 0) {
        if (cols[i].value == cols[j].value) {
          return(1);
        }
      }

      if (rows[i].value != 0 && rows[j].value != 0) {
        if (rows[i].value == rows[j].value) {
          return(1);
        }
      }
    }
  }

  return(0);
}

int oneMoveLeft (SuDoku *board) {
/*
 * This function tests the board to see if at least one cell does
 * not have a value (indicated by a value of 0).  The argument is
 * a pointer to a SuDoku board.  No changes are made to the board.
 *
 */

  int i,j;

  for (i=1; i<=9; i++) {
    for (j=1; j<=9; j++) {
      if (board->section[i].cell[j] == 0) {
        return(0);
      }
    }
  }
  return(1);
}

c_address nextCell (SuDoku *board, int section, int cell) {
/*
 * Find the next empty cell and return the address.
 *
 */

  int i,j,k=cell;
  c_address theCell;

  for (i=section; i<=9; i++) {
    if (i != section) {
      k=1;
    }
    for (j=k; j<=9; j++) {
      if (board->section[i].cell[j] == 0) {
        theCell.section = i;
        theCell.cell = j;

        return(theCell);
      }
    }
  }

  theCell.section = 0;
  theCell.cell = 0;
  theCell.value = 0;
  return(theCell);
}

c_address nextAvailableCell (SuDoku *board, c_address curCell, int value, board_constraint *constraint) {
/*
 * Find the next available cell on the board for a given value.  This function also evaluates
 * the constraints of that number.
 *
 */

  c_address nextCell;
  int i,j,k;

  for (i=curCell.section; i<=9; i++) {
    if (i==curCell.section) {k = curCell.cell + 1;} else {k=1;}
    for (j=k; j<=9; j++) {
      nextCell.section = i;
      nextCell.cell = j;
      if (valueCellAvailable(board, nextCell, value) == 0) {
        if (constraint->section[nextCell.section].cell[nextCell.cell].value[value] == 1) {
          return(nextCell);
        }
      }
    }
  }

  nextCell.section=0;
  nextCell.cell=0;
  return(nextCell);
}

int valueAvailable(SuDoku *board, int section, int value) {
/*
 * Simply determine if the number 1-9 is already used within
 * the section.  This does not scan the row or column, it only
 * considers the section.  No changes are made to the board.
 *
 */

  int i;

  for (i=1; i<=9; i++) {
    if (board->section[section].cell[i] == value) {
      return(1);
    }
  }

  return(0);
}

int valueCellAvailable(SuDoku *board, c_address curCell, int value) {
/*
 * This specifically tests a value for a cell and returns true or false
 * depending on if the value breaks any rules or not.
 *
 */

  int row_sections[3], row_cells[3];
  int col_sections[3], col_cells[3];
  int i,j;

  if (board->section[curCell.section].cell[curCell.cell] != 0) {return(1);}

  if (valueAvailable(board, curCell.section, value) != 0 ) {return(1);}

  determineRow(&row_sections[0], &row_cells[0], curCell.section, curCell.cell);
  determineCol(&col_sections[0], &col_cells[0], curCell.section, curCell.cell);

  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      if (board->section[row_sections[i]].cell[row_cells[j]] == value) { return(1); }
      if (board->section[col_sections[i]].cell[col_cells[j]] == value) { return(1); }
    }
  }

  return(0);
}

int anotherCellOpen(SuDoku *board, c_address curCell) {
/*
 * This function returns the section number (int) of the next
 * open cell within the current section.  It does not consider the
 * current cell a candidate.  It does not evaluate other sections.
 * A zero is returned if no cells are open after the current cell.
 *
 */

  int i;

  for (i=curCell.cell+1; i<=9; i++) {
    if (board->section[curCell.section].cell[i] == 0) {return(i);}
  }

  return(0);
}

