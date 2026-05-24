#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myshell.h"

/*
    Главная функция оболочки.
    Здесь выбирается режим работы:
    1) обычный режим с вводом с клавиатуры
    2) пакетный режим, если передан файл с командами
*/
int main(int argc, char *argv[])
{
    FILE *input;          /* откуда читаем команды */
    char line[MAX_LINE];  /* буфер для одной введённой строки */
    int interactive;      /* 1 - ввод с клавиатуры, 0 - ввод из файла */

    /*
        Инициализация оболочки:
        сохраняем путь к myshell,
        выставляем переменные среды shell и PWD.
    */
    init_shell(argv[0]);

    /*
        Разрешаем только два варианта запуска:
        myshell
        myshell batchfile
    */
    if (argc > 2)
    {
        printf("Использование: %s [batchfile]\n", argv[0]);
        return 1;
    }

    /*
        Если указан файл, читаем команды из него.
        Если файл не открылся, завершаем программу с ошибкой.
    */
    if (argc == 2)
    {
        input = fopen(argv[1], "r");
        if (input == NULL)
        {
            perror("batchfile");
            return 1;
        }
        interactive = 0;
    }
    else
    {
        /* Обычный режим: команды вводит пользователь */
        input = stdin;
        interactive = 1;
    }

    /*
        Основной цикл оболочки.
        Пока есть строки для чтения, обрабатываем их по одной.
    */
    while (1)
    {
        /* В интерактивном режиме печатаем myshell*/
        if (interactive)
        {
            printf("myshell> ");
            fflush(stdout);
        }

        /*
            Читаем строку.
            Если дошли до конца файла или произошла ошибка чтения,
            выходим из цикла.
        */
        if (fgets(line, sizeof(line), input) == NULL)
        {
            break;
        }

        /*
            Если строка пустая и содержит только Enter,
            просто читаем следующую команду.
        */
        if (strcmp(line, "\n") == 0)
        {
            continue;
        }

        /* Передаём строку на разбор и выполнение */
        process_line(line);
    }

    /* Закрываем файл, если работали в пакетном режиме */
    if (input != stdin)
    {
        fclose(input);
    }

    return 0;
}
