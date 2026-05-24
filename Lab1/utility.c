#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "myshell.h"

/*
    environ - стандартный массив переменных среды.
    Через него команда environ выводит все переменные окружения.
*/
extern char **environ;

/*
    Здесь будет храниться полный путь к исполняемому файлу myshell.
    Этот путь потом используется для переменных shell и parent.
*/
char shell_path[MAX_PATH_LEN];

/*
    Подготовка оболочки к работе.
    Функция пытается определить полный путь к myshell,
    записывает его в shell_path и сохраняет в переменную среды shell.
    Также обновляется переменная PWD текущим каталогом.
*/
void init_shell(char *argv0)
{
    char cwd[MAX_PATH_LEN];
    ssize_t len;

    /*
        Сначала пытаемся получить полный путь через /proc/self/exe.
        Это работает на Linux.
    */
    len = readlink("/proc/self/exe", shell_path, sizeof(shell_path) - 1);
    if (len != -1)
    {
        shell_path[len] = '\0';
    }
    else
    {
        /*
            Если readlink не сработал, пробуем realpath.
            Если и это не получилось, оставляем argv0 как есть.
        */
        if (realpath(argv0, shell_path) == NULL)
        {
            strcpy(shell_path, argv0);
        }
    }

    /* Сохраняем путь к оболочке в переменную среды shell */
    setenv("shell", shell_path, 1);

    /* Сохраняем текущий каталог в PWD */
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        setenv("PWD", cwd, 1);
    }
}

/*
    Разбиение введённой строки на отдельные слова.
    В конце массива ставится NULL,
    потому что это нужно для execvp.
*/
void split_words(char *line, char *args[], int *count)
{
    char *token;

    *count = 0;

    /* strtok режет строку по пробелам, табам и переводу строки */
    token = strtok(line, " \t\n");
    while (token != NULL && *count < MAX_ARGS - 1)
    {
        args[*count] = token;
        (*count)++;
        token = strtok(NULL, " \t\n");
    }

    /* Последний элемент должен быть NULL */
    args[*count] = NULL;
}

/*
    Встроенная команда cd.
    cd <путь>  - перейти в указанный каталог
    cd         - показать текущий каталог
*/
static void do_cd(char *args[], int count)
{
    char cwd[MAX_PATH_LEN];

    /* Если аргумента нет, просто печатаем текущий путь */
    if (count == 1)
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        return;
    }

    /* Пытаемся перейти в новый каталог */
    if (chdir(args[1]) != 0)
    {
        perror("cd");
        return;
    }

    /* После успешного перехода обновляем PWD */
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        setenv("PWD", cwd, 1);
    }
}

/*
    Встроенная команда clr.
    Очищает экран ANSI-последовательностью.
*/
static void do_clr(void)
{
    printf("\033[H\033[J");
}

/*
    Встроенная команда dir.
    Показывает содержимое каталога.
    Если каталог не указан, показывается текущий каталог.
*/
static void do_dir(char *args[], int count)
{
    DIR *d;
    struct dirent *entry;
    char *name;

    /* Если путь не задан, используем текущий каталог */
    if (count == 1)
    {
        name = ".";
    }
    else
    {
        name = args[1];
    }

    d = opendir(name);
    if (d == NULL)
    {
        perror("dir");
        return;
    }

    /* Читаем все записи каталога по одной */
    while ((entry = readdir(d)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        } else {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(d);
}

/*
    Встроенная команда environ.
    Печатает все переменные среды процесса.
*/
static void do_environ(void)
{
    int i;

    i = 0;
    while (environ[i] != NULL)
    {
        printf("%s\n", environ[i]);
        i++;
    }
}

/*
    Встроенная команда echo.
    Печатает все аргументы после имени команды через пробел.
*/
static void do_echo(char *args[], int count)
{
    int i;

    for (i = 1; i < count; i++)
    {
        printf("%s", args[i]);
        if (i != count - 1)
        {
            printf(" ");
        }
    }
    printf("\n");
}

/*
    Встроенная команда help.
    Просто открывает файл readme и печатает его содержимое.
*/
static void do_help(void)
{
    FILE *f;
    char text[MAX_LINE];

    f = fopen("readme", "r");
    if (f == NULL)
    {
        printf("Не удалось открыть readme\n");
        return;
    }

    while (fgets(text, sizeof(text), f) != NULL)
    {
        printf("%s", text);
    }

    fclose(f);
}

/*
    Встроенная команда pause.
    Останавливает оболочку, пока пользователь не нажмёт Enter.
*/
static void do_pause(void)
{
    char temp[MAX_LINE];

    printf("Нажмите Enter...\n");
    fgets(temp, sizeof(temp), stdin);
}

/*
    Проверка, является ли команда встроенной.
    Если да, выполняем её здесь же и возвращаем 1.
    Если нет, возвращаем 0, чтобы потом пробовать запуск как внешнюю программу.
*/
int run_builtin(char *args[], int count)
{
    /*если команда пустая, ничего делать не надо */
    if (count == 0)
    {
        return 1;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        do_cd(args, count);
        return 1;
    }

    if (strcmp(args[0], "clr") == 0)
    {
        do_clr();
        return 1;
    }

    if (strcmp(args[0], "dir") == 0)
    {
        do_dir(args, count);
        return 1;
    }

    if (strcmp(args[0], "environ") == 0)
    {
        do_environ();
        return 1;
    }

    if (strcmp(args[0], "echo") == 0)
    {
        do_echo(args, count);
        return 1;
    }

    if (strcmp(args[0], "help") == 0)
    {
        do_help();
        return 1;
    }

    if (strcmp(args[0], "pause") == 0)
    {
        do_pause();
        return 1;
    }

    if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }

    /* Если ни одно имя не подошло, это не встроенная команда */
    return 0;
}

/*
    Запуск внешней программы.
    Делается через fork:
    - родительский процесс ждёт завершения ребёнка
    - дочерний процесс вызывает execvp

    Перед execvp для дочернего процесса записывается переменная parent.
*/
void run_external(char *args[])
{
    pid_t pid;

    pid = fork();

    /* Ошибка создания дочернего процесса */
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    /* Дочерний процесс */
    if (pid == 0)
    {
        /* В дочернем процессе сохраняем путь к оболочке в parent */
        setenv("parent", shell_path, 1);

        /*
            Пытаемся заменить текущий процесс новой программой.
            Если execvp сработает, код ниже уже не выполнится.
        */
        execvp(args[0], args);

        /* Если дошли сюда, значит execvp завершился ошибкой */
        perror("exec");
        exit(1);
    }
    else
    {
        /* Родитель ждёт завершения дочернего процесса */
        waitpid(pid, NULL, 0);
    }
}

/**
    Обработка одной командной строки.
    Шаги
    1) разбить строку на слова
    2) если команда встроенная, выполнить её
    3) иначе запустить как внешнюю программу
*/
void process_line(char *line)
{
    char *args[MAX_ARGS];
    int count;

    /* Разбиваем строку на массив аргументов */
    split_words(line, args, &count);

    /* Если после разбора ничего нет, просто выходим */
    if (count == 0)
    {
        return;
    }

    /*
        Если это не встроенная команда,
        пробуем выполнить её как обычную внешнюю программу.
    */
    if (run_builtin(args, count) == 0)
    {
        run_external(args);
    }
}
