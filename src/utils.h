#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 256

void clear_input_buffer(void);
void remove_newline(char *str);
int get_valid_int(int min_val, int max_val, const char *prompt);

#endif /* UTILS_H */

