# C Basics

## 1. Data Types

| Data Type | Description |
|-----------|-------------|
| int | Used to store whole numbers without decimal values. |
| float | Used to store decimal numbers with single precision. |
| double | Used to store decimal numbers with double precision. |
| char | Used to store a single character. |
| bool | Used to store true or false values. |
| void | Indicates that a function does not return a value. |

## 2. Format Specifiers

| Format Specifier | Description |
|------------------|-------------|
| %d | Used to print or read a signed integer. |
| %u | Used to print or read an unsigned integer. |
| %o | Used to print an integer in octal form. |
| %x | Used to print an integer in hexadecimal form using lowercase letters. |
| %X | Used to print an integer in hexadecimal form using uppercase letters. |
| %f | Used to print a floating-point number. |
| %e | Used to print a floating-point number in scientific notation. |
| %c | Used to print or read a single character. |
| %s | Used to print or read a string. |
| %ld | Used to print or read a long integer. 



## 3. Input/Output Functions

### `scanf()`

The `scanf()` function is used to take input from the user.

Example:

```c
int age;
printf("Enter your age: ");
scanf("%d", &age);
```

### `printf()`

The `printf()` function is used to display output on the screen.

Example:

```c
printf("Hello World");
```

### `getchar()`

The `getchar()` function is used to read a single character from the keyboard.

Example:

```c
char ch;
printf("Enter a character: ");
ch = getchar();
```

### `putchar()`

The `putchar()` function is used to display a single character on the screen.

Example:

```c
char ch = 'A';
putchar(ch);
```

### `fgets()`

The `fgets()` function is used to read a line of text, including spaces.

Example:

```c
char name[50];
printf("Enter your name: ");
fgets(name, 50, stdin);
```

### `puts()`

The `puts()` function is used to display a string followed by a new line.

Example:

```c
puts("Hello World");
```

## 4. Escape Sequences

Escape sequences are special characters used inside strings and character constants in C.

| Escape Sequence | Meaning               | Example                   |
| --------------- | --------------------- | ------------------------- |
| `\n`            | New line              | `printf("Hello\nWorld");` |
| `\t`            | Horizontal tab        | `printf("Name\tAge");`    |
| `\\`            | Backslash             | `printf("\\");`           |
| `\"`            | Double quotation mark | `printf("\"Hello\"");`    |
| `\'`            | Single quotation mark | `printf("\'A\'");`        |
| `\b`            | Backspace             | `printf("ABC\b");`        |
| `\r`            | Carriage return       | `printf("Hello\r");`      |



## 5. Precision

Precision is used to control the number of digits displayed after the decimal point when printing floating-point numbers.

For example:

```c
printf("%.2f", 12.3456);
```

Output:

```text
12.35
```

Here, `.2` means that two digits will be displayed after the decimal point.

The general format is:

```text
%.nf
```

where `n` represents the number of digits required after the decimal point.

For example:

```c
printf("%.3f", 12.3456);
```

Output:

```text
12.346
```

Therefore, precision helps control the number of decimal places shown in floating-point output.



```c
int age;
scanf("%d", &age);
