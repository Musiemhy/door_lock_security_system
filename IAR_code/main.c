#include <NXP/iolpc2124.h>
#include <string.h>

#define RS (1 << 8)
#define EN (1 << 9)
#define MOTOR_IN1 (1 << 10)  // P0.10
#define MOTOR_IN2 (1 << 11)  // P0.11

// Define correct password
char password[] = "1234";

// LCD command/data function
void lcd_cmd(char a);
void lcd_data(char a);
void lcd_string(char *str);
// Motor control function
void motor_open();
void motor_close();
void motor_stop();
//Delay and getting input from keypad functions
void delay_ms(unsigned int time);
char get_keypad_input(void);

void lcd_init(void) {
    lcd_cmd(0x38);  // 2 lines, 5x7 matrix
    lcd_cmd(0x0C);  // Display ON, Cursor OFF
    lcd_cmd(0x06);  // Entry mode
    lcd_cmd(0x01);  // Clear display
    delay_ms(2);
}

int main(void) {
    int i;
    char input[5];
    
    IO0CLR = 0xFFFFFFFF;  // Clear all pins before setting directions

    // Configure LCD and keypad ports
    IO0DIR |= 0x000003FF; // LCD control and data pins (P0.0–P0.9) as output
    IO1DIR |= (1 << 19) | (1 << 20) | (1 << 21) | (1 << 22); // Rows of keypad in the ARM
    IO1DIR &= ~((1 << 16) | (1 << 17) | (1 << 18));          // Columns of keypad in the ARM
    IO0DIR |= MOTOR_IN1 | MOTOR_IN2;  // Set motor control pins as output
    motor_stop();                     // Ensure motor is OFF initially

    lcd_init();
    lcd_string("Welcome!");
    delay_ms(2);
    lcd_cmd(0x01); // Clear screen

    while (1) {
        lcd_cmd(0x01);
        lcd_string("Enter Password:");
        lcd_cmd(0xC0);

        for (i = 0; i < 4; i++) {
            input[i] = get_keypad_input();
            lcd_data(input[i]);
        }
        input[4] = '\0';

        if (strcmp(input, password) == 0) {
            lcd_cmd(0x01);
            lcd_string("Access Granted");
            motor_open();
            delay_ms(1000);
            motor_stop();
            // delay_ms(200);
            // motor_close();
            // delay_ms(300);
            // motor_stop();
            break;
        } else {
            lcd_cmd(0x01);
            lcd_string("Wrong Password, Please try again!");
            delay_ms(200);
        }
    }

    while (1); // Stop here after success
}

// LCD send command
void lcd_cmd(char a) {
    IO0CLR = 0xFF;
    IO0SET = a;
    IO0CLR = RS;
    IO0SET = EN;
    delay_ms(2);
    IO0CLR = EN;
}

// LCD send data
void lcd_data(char a) {
    IO0CLR = 0xFF;
    IO0SET = a;
    IO0SET = RS;
    IO0SET = EN;
    delay_ms(2);
    IO0CLR = EN;
    IO0CLR = RS;
}

// Send string to LCD
void lcd_string(char *str) {
    while (*str)
        lcd_data(*str++);
}

void motor_open() {
    IO0SET = MOTOR_IN1;
    IO0CLR = MOTOR_IN2;
}

void motor_close() {
    IO0SET = MOTOR_IN2;
    IO0CLR = MOTOR_IN1;
}

void motor_stop() {
    IO0CLR = MOTOR_IN1 | MOTOR_IN2;
}

// Delay function
void delay_ms(unsigned int time) {
    unsigned int i, j;
    for (i = 0; i < time; i++)
        for (j = 0; j < 1000; j++);
}

// Keypad scan function
char get_keypad_input(void) {
    const char keys[4][3] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}
    };

    unsigned int row_pins[4] = {19, 20, 21, 22};
    unsigned int col_pins[3] = {16, 17, 18};
    int row, col;

    while (1) {
        for (row = 0; row < 4; row++) {
            // Set all rows HIGH
            IO1SET = (1 << 19) | (1 << 20) | (1 << 21) | (1 << 22);
            // Pull one row LOW
            IO1CLR = (1 << row_pins[row]);

            delay_ms(1); // Allow line to settle

            for (col = 0; col < 3; col++) {
                if ((IO1PIN & (1 << col_pins[col])) == 0) {
                    while ((IO1PIN & (1 << col_pins[col])) == 0); // Wait for release
                    return keys[row][col];
                }
            }
        }
    }
}
