# Alpha 7G

## Компилятор

Использем компилятор **gcc**, а точнее его порт под *Windows* **MinGW-w64**.
А так же **Gnu-Make** для сборки

## Сборка проекта

Перед сборкой проекта, надо прописать в `PATH` путь к `gcc.exe` и к `mingw32-make.exe`.
Для сборки проекта, требуется запустить файл `"+console.bat"`.
После чего ввести: `"+build.bat"` для сборки проекта или `"+clean.bat"` для отчистки.

## Стиль кода

### Отступы

Используем 4 символа пробела вместо табуляций для отступов.

### Фигурные скобки

Для всех случаев, открывающая фигурная скобка начинается с той же строки, а не с новой.

### Наименования

Названия функций на новой строке в отличие от определении типа возвращаемого значения.

| Префикс | Для чего и смысл                             |
|---------|----------------------------------------------|
| `P_`    | отметка GOTO                                 |
| `P7`    | описатель указателя на функцию               |
| `S7`    | структуры и типы                             |
| `A7`    | функции                                      |
| `_7`    | статичные функции                            |
| `E7`    | макроподстановки констант номеров ошибок     |
| `F7`    | макроподстановки макросов                    |
| `K7`    | перечесляемые значения                       |
| `D7`    | остальные макроподстановки                   |
| `g_`    | глобальная переменная                        |
| `k`     | константа                                    |
| `p`     | указатель                                    |
| `pv`    | void указатель                               |
| `s`     | строка                                       |
| `w`     | широкая строка                               |
| `n`     | целое число                                  |
| `i`     | какой-то перечесляемый тип, уникальное число |
| `b`     | булева переменная                            |
| `h`     | дексриптор                                   |
| `a`     | массив                                       |
| `r`     | указатель на функцию                         |

### Больше пространства

Разделяем всевозможные символы одним пробелом.

```c
int
main ( ) {
    return 0;
}
```
