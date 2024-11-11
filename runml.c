//  CITS2002 Project 1 2024
//  Student1:   23845246   Yimian Wang
//  Platform:   Linux
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

//  Store declared variables
bool is_variable_declared(const char *identifier, char *declared_vars[], int declared_count) {
//  Repeat through each declared variable to see if it is same as the one declared before
    for (int i = 0; i < declared_count; i++) {
        if (strcmp(declared_vars[i], identifier) == 0 ) {
            return true;
        }
    }
    return false;
}

//  Validate whether the text file contains .ml
bool ml_extension_validation(const char *filename) {
    int len = strlen(filename);
//  Check if the last 3 characters are .ml
    return len > 3 && strcmp(filename + len - 3, ".ml") == 0;
}

//  Verify if an identifier consists of 1..12 lowercase alphabetic characters
bool identifier_validation(const char *str) {
    int length = strlen(str);

    if (length < 1 || length > 12) {
        return false;
    }

    for (int i = 0; i < length; i++) {
        if (!islower(str[i])) {
            return false;
        }
    }

    return true;
}

//  Compile C code
bool compile_c_code(const char *c_file_pointer, const char *executable_file_pointer) {
    char command[BUFSIZ];
    snprintf(command, sizeof(command), "cc -std=c11 -Wall -Werror -o %s %s", executable_file_pointer, c_file_pointer);
    return system(command) == 0;
}

//  Remove comments beginning with #
void remove_comments(char *str)
{
//  Detect comments by iterating through the string until the end or '#'
     for (int i = 0; str[i] != '\0'; i++) {
          if (str[i] == '#') {
              str[i] = '\0';
              break;
          }
     }
}

//  Convert the assignment symbol into =
void assignment_statement_conversion(char *str)
{
     for (int i = 0; str[i] !='\0'; i++) {
//  If str[i] is '<' and the character after it is '-', it is an assignment symbol '<-', then the '<' will be replaced by '='
         if  (str[i] == '<' && str[i+1] == '-') {
              str[i] = '=';
//  Shift the string to the left so that '-' can be removed
              for (int j = i + 1; str[j] != '\0'; j++) {
                   str[j] = str[j + 1];
               }
               break;
          }
     }

}

//  Get identifier for assessment
void get_identifier(const char *str, char *identifier) {
    int i = 0, j = 0;

//  Get characters until there is an assignment symbol or end of string
    while (str[i] != '\0' && str[i] != '=' && str[i] != '<' && str[i] != '>') {
        if (isalpha(str[i])) {
            identifier[j++] = str[i];
        }
//  Increase the index so that the next valid character can be stored in the next position
        i++;
     }

//  The null byte terminates the identifier string
     identifier[j] = '\0';
}

//  Add a semicolon to the end of each statement if needed
void add_semicolon(char *str) {
    int stringlength = strlen(str);

//  Add a semicolon if the line does not end with one
    if (stringlength > 0 && str[stringlength - 1] != ';' && str[stringlength - 1] != '\n') {
        str[stringlength] = ';';
        str[stringlength + 1] = '\0';
    }
}

//  Remove whitespace and newline
void whitespace_newline_removal (char *str) {
    int length = strlen(str);
//  Repeat the loop to make sure the new line is removed
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0';
        length--;
    }
//  Remove whitespaces untill there isn't any
    while (length > 0 && str[length - 1] == ' ') {
         str[--length] = '\0';
    }
}

//  Convert mini language into C, write to a C file
void mini_language_to_c_conversion(FILE *mini_file_pointer, FILE *c_file_pointer) {

//  Add headers and essential functions to the generated C file
    fprintf(c_file_pointer, "#include <stdio.h>\n");
    fprintf(c_file_pointer, "#include <stdlib.h>\n");
    fprintf(c_file_pointer, "#include <stdbool.h>\n");
    fprintf(c_file_pointer, "#include <math.h>\n");
    fprintf(c_file_pointer, "#include <string.h>\n");
    fprintf(c_file_pointer, "#include <ctype.h>\n");
    fprintf(c_file_pointer, "#include <unistd.h>\n");
    fprintf(c_file_pointer, "\n");


    fprintf(c_file_pointer, "bool is_integer(double number) {\n");
    fprintf(c_file_pointer, "    return number == (int)number;\n");
    fprintf(c_file_pointer, "}\n");
    fprintf(c_file_pointer, "\n");

    fprintf(c_file_pointer, "void print_number(double number) {\n");
    fprintf(c_file_pointer, "    if (is_integer(number)) {\n");
    fprintf(c_file_pointer, "        printf(\"%%d\\n\", (int)number);\n");
    fprintf(c_file_pointer, "    } else {\n");
    fprintf(c_file_pointer, "        printf(\"%%.6f\\n\", number);\n");
    fprintf(c_file_pointer, "    }\n");
    fprintf(c_file_pointer, "}\n");
    fprintf(c_file_pointer, "\n");

    char buffer[BUFSIZ];
//  Ensure there will be at most 50 unique identifiers appearing in any program
    char identifier[50];
//  Track the declared variables
    char *declared_variable_list[BUFSIZ];
    char *declared_function_list[BUFSIZ];

    char parameter1[BUFSIZ] = "";
    char parameter2[BUFSIZ] = "";
    char function_type[BUFSIZ] = "";
    char function_name[BUFSIZ] = "";

//  Initialize
    int declared_count = 0;
    int declared_function_count = 0;

    bool main_function_added = false;

        while (fgets(buffer, sizeof(buffer), mini_file_pointer)) {
        remove_comments(buffer);
//  Debug
        printf("@%s", buffer);
//  Skip empty lines
        if (strlen(buffer) == 0 || buffer[0] == '\n') {
            continue;
        }
//  Handle print statement before any assignment conversion
    if (strncmp(buffer, "print", 5) == 0) {
//  Add main function if there isn't as for "print" there is no need to worry about void functions
        if (!main_function_added) {
            fprintf(c_file_pointer, "int main() {\n");
            main_function_added = true;
        }
//  Skip the "print" word and the space
        char *variable_to_be_printed = buffer + 5;

        while (*variable_to_be_printed == ' ') {
            variable_to_be_printed++;
        }
        whitespace_newline_removal(variable_to_be_printed);
//  Write to the generated C file
        fprintf(c_file_pointer, "    print_number(%s);\n", variable_to_be_printed);
        continue;
    }

//  Handle function declarations
    if (strncmp(buffer, "function", 8) == 0) {
        whitespace_newline_removal(buffer);
//  Determine the function type
        if (sscanf(buffer + 8, "%s", function_type) == 1 && (strcmp(function_type, "bool") == 0 || strcmp(function_type, "int") == 0 || strcmp(function_type, "double") == 0)) { sscanf(buffer + 8 + strlen(function_type), "%s %s %s", function_name, parameter1, parameter2);
        } else {
            strcpy(function_type, "void");
            sscanf(buffer + 8, "%s %s %s", function_name, parameter1, parameter2);
        }
        fprintf(c_file_pointer, "%s %s(double %s, double %s) {\n", function_type, function_name, parameter1, parameter2);
        while (fgets(buffer, sizeof(buffer), mini_file_pointer)) {
            whitespace_newline_removal(buffer);
//  Look for "print" and skip the word if there is
             if (strncmp(buffer, "print", 5) == 0) {
                 char *expression = buffer + 5;
                 while (*expression == ' ') {
                     expression++;
                 }
// Handle arithmetic calculations
                 if (strstr(expression, "+") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s + %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "-") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s - %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "*") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s * %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "/") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s / %s);\n", parameter1, parameter2);
                 }
                 continue;
             }

//  Look for "return" and skip the word if there is
             if (strncmp(buffer, "return", 6) == 0) {
                 char *expression = buffer + 6;
                 while (*expression == ' ') {
                     expression++;
                 }
                 if (strstr(expression, "+") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s + %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "-") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s - %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "*") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s * %s);\n", parameter1, parameter2);
                 } else if (strstr(expression, "/") != NULL) {
                     fprintf(c_file_pointer, "    print_number(%s / %s);\n", parameter1, parameter2);
                 }
                 continue;
            }
        }

//  Allocate the memory and copy the function name
        declared_function_list[declared_function_count] = malloc(strlen(function_name) + 1);
        if (declared_function_list[declared_function_count] == NULL) {
            fprintf(stderr, "!Error, memory allocation failure.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(declared_function_list[declared_function_count], function_name);
        declared_function_count ++;
        continue;
    }

//  Don't remove whitespace and newline for function lines
    if (strncmp(buffer, "function", 8) == 0) {
        fprintf(c_file_pointer, "%s\n", buffer);
    } else {
        whitespace_newline_removal(buffer);
//  Convert assignment symbol for assignment lines
        assignment_statement_conversion(buffer);
//  Get the identifier
        get_identifier(buffer, identifier);
//  Validate identifier
        if (!identifier_validation(identifier)) {
            fprintf(stderr, "!Error, invalid identifier: %s\n", identifier);
            exit(EXIT_FAILURE);
        }
    }

//  If the ml file has functions, only add the main function after all void functions are written
    if (!main_function_added) {
        fprintf(c_file_pointer, "\nint main() {\n");
        main_function_added = true;
    }
//  Extract parameters from the .ml file
    if (sscanf(buffer, "%s (%s , %s)", function_name, parameter1, parameter2) == 3) {
//  Call the declared functions to the main
        for (int i = 0; i < declared_function_count; i++) {
            if (strcmp(function_name, declared_function_list[i]) == 0) {
                fprintf(c_file_pointer, "    %s(%s, %s);\n", function_name, parameter1, parameter2);
            }
        }
    }
//  Handle assignment
    if (strlen(identifier) > 0 && strcmp(identifier, "print") != 0 && !is_variable_declared(identifier, declared_variable_list, declared_count)) {
        char *assignment_value = strchr(buffer, '=');
        if (assignment_value == NULL) {
            fprintf(stderr, "!Error, no assignment is in this line: %s\n", buffer);
            exit(EXIT_FAILURE);
        }
//  Move past "=". Remove whitespace and newline to avoid semicolon being added to the next line
                assignment_value++;
                whitespace_newline_removal(assignment_value);
//  Convert the assignment value to a double
                double number = atof(assignment_value);
//  If main has not been added that means the ml file doesn't have functions and the main function can be added without worrying getting mixed with void functions
                if(!main_function_added) {
                   fprintf(c_file_pointer, "\nint main() {\n");
                   main_function_added = true;
                }
//  Declare and initialize the variable
                fprintf(c_file_pointer, "    double %s = %g;\n", identifier, number);
                fprintf(c_file_pointer, "    %s = %s * 1;\n", identifier, identifier);
//  Allocate memory for new declared variable and copy the identifier
                declared_variable_list[declared_count] = malloc(strlen(identifier) + 1);
                if (declared_variable_list[declared_count] == NULL) {
                    fprintf(stderr, "!Error, memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(declared_variable_list[declared_count], identifier);
                declared_count++;
            } else if (strstr(buffer, "<-")) {
//  Handle <-
                printf("%s\n", buffer);
            }
//  Add semicolon, but not to function lines
            if (strncmp(buffer, "function", 8) != 0) {
                add_semicolon(buffer);
            }
        }
//  Add the closing of the main function to the generated C file
    if (strcmp(function_name, "main") == 0) {
       fprintf(c_file_pointer, "    return 0;\n");
    }
    fprintf(c_file_pointer, "}\n");
}

//  Handle command-line arguments
int main(int argc, char *argv[])
{
//  Ensure the correct number of command-line arguments
    if (argc != 2) {
    fprintf(stderr, "!Error, invalid input, the number of arguments should be exactly 2\n");
    return 1;
    }

// Check whether the file is a .ml file
    const char *ml_filename = argv[1];
    if (!ml_extension_validation(ml_filename)) {
        fprintf(stderr, "!Error, unacceptable file type, it must have '.ml' extension: %s\n", ml_filename);
        return 1;
    }

//  Get a PID for the C file
    char c_filename[BUFSIZ];
    snprintf(c_filename, sizeof(c_filename), "ml-%d.c", getpid());

//  Get a PID for the executable file
    char executable_filename[BUFSIZ];
    snprintf(executable_filename, sizeof(executable_filename), "ml-%d", getpid());

//  Open the .ml file for read-access
    FILE *ml_file_pointer = fopen(ml_filename, "r");
//  If the file fails to open, it will generate an error message to stderr and terminate with an EXIT_FAILURE status
    if (ml_file_pointer == NULL) {
        fprintf(stderr, "!Error, unable to open the file: %s\n", ml_filename);
        return 1;
    }

//  Create a .c file for the converted code
    FILE *c_file_pointer = fopen(c_filename, "w");
    if (c_file_pointer == NULL) {
        fprintf(stderr, "!Error, unable to create the .c file: %s\n", c_filename);
        fclose(ml_file_pointer);
        return 1;
    }

//  Convert mini language into C, write to a C file
    mini_language_to_c_conversion(ml_file_pointer, c_file_pointer);

//  Close the files
    fclose(ml_file_pointer);
    fclose(c_file_pointer);

//  Compile the converted C code
    if (!compile_c_code(c_filename, executable_filename)) {
        fprintf(stderr, "!Error, unable to compile the C code: %s\n", c_filename);
        return 1;
    }

//  Execute the compiled C program
    char command[BUFSIZ * 2];
    snprintf(command, sizeof(command), "./%s", executable_filename);
    if (system(command) != 0) {
        fprintf(stderr, "!Error, failed to execute the compiled program: %s\n", executable_filename);
        return 1;
    }

//  Remove the file after execution
    if (remove(c_filename) != 0) {
        fprintf(stderr, "!Error, unable to remove the c_file: %s\n", c_filename);
    }

    if (remove(executable_filename) != 0) {
        fprintf(stderr, "!Error, unable to remove the executable_filename: %s\n", executable_filename);
    }

    return 0;
}
