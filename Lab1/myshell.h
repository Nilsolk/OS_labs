#ifndef MYSHELL_H
#define MYSHELL_H

/* Максимальная длина одной введённой строки */
#define MAX_LINE 1024

/* Максимальное количество слов в одной команде */
#define MAX_ARGS 100

/* Максимальный размер буфера для пути */
#define MAX_PATH_LEN 1024

/* Инициализация оболочки и переменных среды */
void init_shell(char *argv0);

/* Обработка одной введённой строки */
void process_line(char *line);

/* Разбиение строки на слова */
void split_words(char *line, char *args[], int *count);

/* Проверка и выполнение встроенной команды */
int run_builtin(char *args[], int count);

/* Запуск внешней программы */
void run_external(char *args[]);

#endif
