#include "state.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  game_state_t *state = malloc(sizeof(game_state_t));
  state->num_snakes = 1;
  state->num_rows = 18;
  state->snakes = malloc(sizeof(snake_t)*state->num_snakes);
  state->snakes->head_col = 4;
  state->snakes->head_row = 2;
  state->snakes->tail_col = 2;
  state->snakes->tail_row = 2;
  state->snakes->live = true;
  state->board = malloc(18*sizeof(char*));
  for (int i =0;i<state->num_rows;i++) {
    *(state->board+i) = malloc(22*sizeof(char));
  }
  strcpy(state->board[0],  "####################\n");
  strcpy(state->board[1],  "#                  #\n");
  strcpy(state->board[2],  "# d>D    *         #\n");
  strcpy(state->board[3],  "#                  #\n");
  for (unsigned int i = 4; i < 17; i++) {
    strcpy(state->board[i], "#                  #\n");
  }
  strcpy(state->board[17], "####################\n");

  return state;
}

/* Task 2 */
void free_state(game_state_t *state) {
    if (state->board != NULL) { //free board
      for (int i = 0; i < state->num_rows; i++) {
        if (state->board[i] != NULL) {
          free(state->board[i]);
        }
      }
      free(state->board);
    }
    if (state->snakes != NULL) { //free snake
      free(state->snakes);
    }
    free(state);                 //free game
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  for (int i = 0;i<state->num_rows;i++) {
    fputs(state->board[i], fp);
  }
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  char tail[]="awds";
  for (int i =0;tail[i]!='\0';i++) {
    if (c == tail[i]) {
      return true;
    }
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return isupper(c);
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  char src[]="wasd^<v>WASDx";
  for (int i =0;i<12;i++) {
    if (c == src[i]) {
      return true;
    }
  }
  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
    case '^':
      return 'w';
    case 'v':
      return 's';
    case '<':
      return 'a';
    case '>':
      return 'd';

  }
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
    case 'W':
      return '^';
    case 'S':
      return 'v';
    case 'A':
      return '<';
    case 'D':
      return '>';
  }
  return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  switch (c) {
    case '^':
    case 'w':
    case 'W':
      return cur_row - 1;
    case 'v':
    case 's':
    case 'S':
      return cur_row + 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  switch (c) {
    case '>':
    case 'd':
    case 'D':
      return cur_col+1;
    case '<':
    case 'a':
    case 'A':
      return cur_col-1;

  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  snake_t cur_snake = state->snakes[snum];
  char head = get_board_at(state,cur_snake.head_row,cur_snake.head_col);
  unsigned int next_col = get_next_col(cur_snake.head_col,head);
  unsigned int next_row = get_next_row(cur_snake.head_row,head);
  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  snake_t* cur_snake = &state->snakes[snum];
  char head = get_board_at(state,cur_snake->head_row,cur_snake->head_col);
  unsigned int next_col = get_next_col(cur_snake->head_col,head);
  unsigned int next_row = get_next_row(cur_snake->head_row,head);
  state->board[cur_snake->head_row][cur_snake->head_col] = head_to_body(head);
  state->board[next_row][next_col] = head;
  cur_snake->head_col = next_col;
  cur_snake->head_row = next_row;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  snake_t* cur_snake = &state->snakes[snum];
  char tail = get_board_at(state,cur_snake->tail_row,cur_snake->tail_col);
  unsigned int next_col = get_next_col(cur_snake->tail_col,tail);
  unsigned int next_row = get_next_row(cur_snake->tail_row,tail);
  state->board[next_row][next_col] = body_to_tail(state->board[next_row][next_col]);
  state->board[cur_snake->tail_row][cur_snake->tail_col] = ' ';
  cur_snake->tail_col = next_col;
  cur_snake->tail_row = next_row;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  snake_t* snakes = state->snakes;
  for (unsigned int i=0;i<state->num_snakes;i++) {
    unsigned int cur_row = snakes[i].head_row;
    unsigned int cur_col = snakes[i].head_col;
    if (next_square(state,i) == '*') {
      update_head(state,i);
      add_food(state);
    }else if (next_square(state,i) == ' ') {
      update_head(state,i);
      update_tail(state,i);
    }else{
      set_board_at(state,cur_row,cur_col,'x');
      snakes[i].live = false;
    }

  }

  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  size_t buffer_size = 128;  // 初始缓冲区大小
  char *line = malloc(buffer_size);  // 为行分配内存
  if (line == NULL) {
    return NULL;  // 内存分配失败
  }

  // 使用 fgets 从文件流读取一行
  if (fgets(line, buffer_size, fp) == NULL) {
    free(line);  // 如果读取失败，释放内存并返回 NULL
    return NULL;
  }

  // 检查如果一行字符长度超过了缓冲区，进行扩展
  size_t len = strlen(line);  // 获取当前行的字符数
  char *temp = realloc(line,len);
  if (temp == NULL) {
    return NULL;
  }
  line = temp;
  return line;  // 返回读取到的行

}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  game_state_t *state = malloc(sizeof(game_state_t));
  state->snakes=NULL;
  state->num_snakes = 0;
  size_t row_count = 0;
  char **board = NULL;
  char *line;
  while ((line = read_line(fp)) != NULL) {
    row_count++;
    board = realloc(board, row_count * sizeof(char *));
    if (!board) { //分配失败
      free(line);
      for (size_t i = 0; i < row_count - 1; i++) {
        free(board[i]);
      }
      free(board);
      return NULL;
    }
    board[row_count - 1] = line;
  }
  state->board = board;
  state->num_rows = row_count;

  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  int row = state->snakes[snum].tail_row;
  int col = state->snakes[snum].tail_col;
  char direction = state->board[row][col];
  while (1) {
    if (direction == 's' || direction == 'v') { // 向下移动
      row++;
    } else if (direction == 'w' || direction == '^') { // 向上移动
      row--;
    } else if (direction == 'd' || direction == '>') { // 向右移动
      col++;
    } else if (direction == 'a' || direction == '<') { // 向左移动
      col--;
    }
    // 检查当前位置是否是蛇头
    if (is_head(state->board[row][col])) {
      // 填充蛇头位置
      state->snakes[snum].head_row = row;
      state->snakes[snum].head_col = col;
      return;
    }
    direction = state->board[row][col];
  }
  return;
}
static void find_tail(game_state_t *state, unsigned int snum) {
  int row = state->snakes[snum].head_row;
  int col = state->snakes[snum].head_col;
  char direction = state->board[row][col];
  while (1) {
    if (direction == 'S' || direction == 'v') { // 向下移动
      row--;
    } else if (direction == 'W' || direction == '^') { // 向上移动
      row++;
    } else if (direction == 'D' || direction == '>') { // 向右移动
      col--;
    } else if (direction == 'A' || direction == '<') { // 向左移动
      col++;
    }
    // 检查当前位置是否是蛇头
    if (is_tail(state->board[row][col])) {
      // 填充蛇头位置
      state->snakes[snum].tail_row = row;
      state->snakes[snum].tail_col = col;
      return;
    }
    direction = state->board[row][col];
  }
  return;
}
/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  state->num_snakes = 0;
  state->snakes = malloc(state->num_snakes*sizeof(snake_t));
  for (int i =0;i <state->num_rows;i++) {
    for (int j = 0;state->board[i][j]!='\0';j++) {
      if (is_tail(state->board[i][j])) {
        state->num_snakes++;
        state->snakes = realloc(state->snakes,state->num_snakes*sizeof(snake_t));
        if (state->snakes == NULL) { //分配失败
          free_state(state);
          return state;
        }
        state->snakes[state->num_snakes-1].tail_row = i;
        state->snakes[state->num_snakes-1].tail_col = j;
        state->snakes[state->num_snakes - 1].live = 1;
        find_head(state,state->num_snakes-1);

      }
    }
  }
  return state;
}
