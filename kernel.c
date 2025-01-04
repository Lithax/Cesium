#include <stdint.h>
#include <stdarg.h>
#define KEYBOARD_PORT 0x60
#define BUFFER_SIZE 128
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Declare the Multiboot header section
__attribute__((section(".multiboot"))) 
const uint32_t multiboot_header[] = {
    0x1BADB002,          // Magic number
    0x0,                 // Flags (0 for now)
    -(0x1BADB002 + 0x0)  // Checksum (must sum to zero)
};

// Function Declarations
static uint8_t inb(uint16_t port);
char scan_code_to_ascii(uint8_t scan_code);
void putchar(char c);
void buffer_put(char c);
char buffer_get(void);
void keyboard_handler(void);
void readLine(char *buffer, int size);
void process_command(char *command);
void clear_buffer(void);
void itoa(int num, char *str);
void printf(const char *format, ...);


// Global variables
char *video_memory = (char *)0xb8000;
int cursor = 0; // Tracks the current position in video memory
char input_buffer[BUFFER_SIZE];
volatile int buffer_head = 0;
volatile int buffer_tail = 0;

void buffer_put(char c) {
    input_buffer[buffer_head] = c;
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;

    // Prevent buffer overflow
    if (buffer_head == buffer_tail) {
        buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    }
}

char buffer_get() {
    if (buffer_head == buffer_tail) {
        return 0; // Buffer is empty
    }

    char c = input_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    return c;
}

// Simplified scan code to ASCII conversion
char scan_code_to_ascii(uint8_t scan_code) {
    static char key_map[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    };

    if (scan_code < 128) {
        return key_map[scan_code];
    }
    return 0;
}

// Keyboard interrupt handler
void keyboard_handler() {
    uint8_t scan_code = inb(KEYBOARD_PORT);
    char ascii = scan_code_to_ascii(scan_code);

    if (ascii) {
        buffer_put(ascii);
    }
}

void readLine(char *buffer, int size) {
    int i = 0;
    char c;

    while (i < size - 1) {
        while ((c = buffer_get()) == 0) {
            __asm__("hlt"); // Halt to save power
        }

        // Handle backspace
        if (c == '\b' && i > 0) {
            i--;
            putchar('\b'); // Remove last character
        } else if (c == '\n') {
            buffer[i] = '\0';
            putchar('\n');
            return;
        } else if (c != '\b') {
            buffer[i++] = c;
            putchar(c); // Echo character
        }
    }

    buffer[i] = '\0'; // Ensure null-termination
}

// Print a character to the screen
void putchar(char c) {
    if (c == '\n') {
        cursor = (cursor / SCREEN_WIDTH + 1) * SCREEN_WIDTH; // Move to next line
    } else {
        video_memory[cursor * 2] = c;      // Character
        video_memory[cursor * 2 + 1] = 0x07; // Attribute byte (white on black)
        cursor++;

        if (cursor >= SCREEN_WIDTH * SCREEN_HEIGHT) {
            cursor = 0; // Wrap around to top if at the end
        }
    }
}

// Print formatted output
void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int i = 0;
    while (format[i] != '\0') {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++;
            switch (format[i]) {
                case 'c': { 
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': { 
                    const char *str = va_arg(args, const char *);
                    while (*str) {
                        putchar(*str++);
                    }
                    break;
                }
                case 'd': { 
                    int num = va_arg(args, int);
                    char buffer[16];
                    itoa(num, buffer);
                    char *str = buffer;
                    while (*str) {
                        putchar(*str++);
                    }
                    break;
                }
                default:
                    putchar('%');
                    putchar(format[i]);
            }
        } else {
            putchar(format[i]); 
        }
        i++;
    }
    va_end(args);
}

// Converts integer to string
void itoa(int num, char *str) {
    int i = 0;
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    do {
        str[i++] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);
    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

int strcmp(char* str1, char* str2) {
    if(sizeof(str1) != sizeof(str2)) {
        return 1;
    } else {
        for() {
            
        }
    }
}

// Process commands
void process_command(char *command) {
    if (strcmp(command, "hello") == 0) {
        printf("Hello, World!\n");
    } else if (strcmp(command, "version") == 0) {
        printf("OS Version: 1.0\n");
    } else if (strcmp(command, "author") == 0) {
        printf("Author: Lithax\n");
    } else {
        printf("Command not recognized: %s\n", command);
    }
}

// Main program
void _start(void) {
    char command_buffer[BUFFER_SIZE];

    printf("Welcome to Lithax OS!\n");

    while (1) {
        printf("lithax $ ");
        readLine(command_buffer, BUFFER_SIZE);
        process_command(command_buffer);
    }
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
