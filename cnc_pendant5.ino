// CNC pendant
// mach3
// Jef Collin
// 2021

// following shortcuts must be assigned in screen 1024
// zero X ALT x
// zero Y ALT y
// zero Z ALT Z
// zero 4 ALT 4
// jog rate 1 191 CTRL 1
// jog rate 0.1 192 CTRL 2
// jog rate 0.01 193 CTRL 3
// jog cont 276 CTRL 4
// jog step 275 CTRL 5
// reset key remappen naar ctrl 0
// ctrl 8 zero tool







#include <Keyboard.h>


// Hardware definitions
#define NUM_COLS 4 // Number of switch columns 
#define NUM_ROWS 5 // Number of switch rows

// Software defined debounce
#define MAX_DEBOUNCE 50

// Keyboard matrix pin assignments
// Pins connected to rows
static const uint8_t RowPins[NUM_ROWS] = {4, 5, 6, 7, 8};
// Pins connected to columns
static const uint8_t ColPins[NUM_COLS] = {18, 19, 20, 21};

// Global variables
// Counter for how long a button has been pressed
static uint8_t debounce_count[NUM_ROWS][NUM_COLS];

// status to avoid release before press
boolean press_active = false;

// shift key pressed
boolean shift_active = false;

// selected axis 0=X 1=Y 2=Z 3=4
byte selected_axis = 0;

// is jog mode set?
boolean jogmode_set = false;

// rotary encoders start

// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20

// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6
#define R_START 0x0

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

unsigned char enc_state1 = R_START;

char enc_pin1 = 2;
char enc_pin2 = 3;

unsigned char enc_result1 = DIR_NONE;
unsigned char enc_pinstate1;
byte enc_mode = 0;
int enc1_counter = 0;


// rotary encoders end

void isr1() {
  unsigned char pinstate = (digitalRead(enc_pin1) << 1) | digitalRead(enc_pin2);
  enc_state1 = ttable[enc_state1 & 0xf][pinstate];
  unsigned char result = enc_state1 & 0x30;
  if (result == DIR_CW) {
    if (enc1_counter < 100) {
      enc1_counter++;
    }
  } else if (result == DIR_CCW) {
    if (enc1_counter > -100) {
      enc1_counter--;
    }
  }
}

void setup()
{

  // Serial.begin(115200);


  // Counter to set up pins
  uint8_t i;

  // Set row scan pins to output and then to HIGH (not active)
  for (i = 0; i < NUM_ROWS; i++)
  {
    pinMode(RowPins[i], OUTPUT);
    digitalWrite(RowPins[i], HIGH);
  }

  // Set column pins to input with pull-up resistors (no need for external pull-up resistors)
  for (i = 0; i < NUM_COLS; i++)
  {
    pinMode(ColPins[i], INPUT_PULLUP);
  }

  // encoder
  pinMode(enc_pin1, INPUT);
  pinMode(enc_pin2, INPUT);

  // Initialize USB keyboard
  Keyboard.begin();

  // Initialize the debounce counter array
  memset(debounce_count, 0, sizeof(debounce_count));

  attachInterrupt(0, isr1, CHANGE);
  attachInterrupt(1, isr1, CHANGE);

  press_active = false;

  Keyboard.releaseAll();

}

void loop()
{

  // Serial.println(shift_active);



  if (enc1_counter != 0) {
    check_jogmode();
    switch (selected_axis)
    {
      case 0: // X
        if (enc1_counter > 0) {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter--;
        }
        else {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter++;
        }
        break;
      case 1: // Y
        if (enc1_counter > 0) {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_UP_ARROW);
          Keyboard.release(KEY_UP_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter--;
        }
        else {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_DOWN_ARROW);
          Keyboard.release(KEY_DOWN_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter++;
        }

        break;
      case 2: // Z
        if (enc1_counter > 0) {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_PAGE_UP);
          Keyboard.release(KEY_PAGE_UP);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter--;
        }
        else {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_PAGE_DOWN);
          Keyboard.release(KEY_PAGE_DOWN);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter++;
        }
        break;
      case 3: // 4

        // todo aanpassen codes

        if (enc1_counter > 0) {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter--;
        }
        else {
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_CTRL);
          enc1_counter++;
        }
        break;
    }

  }




  // Each run through the scan function operates on a single row
  // of the matrix, kept track of using the currentRow variable.
  static uint8_t currentRow = 0;
  static uint8_t currentCol; // for loop counters

  // Select current row
  digitalWrite(RowPins[currentRow], LOW);

  // Scan through switches on this row:
  for (currentCol = 0; currentCol < NUM_COLS; currentCol++)
  {
    // Read the button. If it's pressed, it should be LOW.
    if (digitalRead(ColPins[currentCol]) == LOW)
    {
      if (debounce_count[currentRow][currentCol] < MAX_DEBOUNCE)
      {
        // Increment a debounce counter
        debounce_count[currentRow][currentCol]++;
        // If debounce counter hits MAX_DEBOUNCE
        // Trigger key press
        if ( debounce_count[currentRow][currentCol] == MAX_DEBOUNCE )
        {
          pressMacro(currentRow, currentCol);
          press_active = true;
        }
      }
    }
    else // Otherwise, button is released
    {
      if ( debounce_count[currentRow][currentCol] > 0)
      {
        // Decrement debounce counter
        debounce_count[currentRow][currentCol]--;
        if ( debounce_count[currentRow][currentCol] == 0 )
        { // If debounce counter hits 0
          //        if (press_active) {
          releaseMacro(currentRow, currentCol);
          press_active = false;
          //        }
        }
      }
    }
  }

  // Once done scanning, de-select the row pin
  digitalWrite(RowPins[currentRow], HIGH);

  // Increment currentRow, so next time we scan the next row
  currentRow = (currentRow > NUM_ROWS - 2) ? 0 : currentRow + 1;

}

void pressMacro(uint8_t row, uint8_t col)
{
  switch (row)
  {
    case 0:
      switch (col)
      {
        case 0: // cycle start
          Keyboard.press(KEY_LEFT_ALT);
          Keyboard.press('r');
          break;
        case 1: // feed hold
          Keyboard.press(' ');
          break;
        case 2: // stop / rewind
          if (shift_active) {
            // rewind
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('w');
          }
          else {
            // stop
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('s');
          }
          break;
        case 3: // reset / reset feed override
          if (shift_active) {
            // reset feed override
            Keyboard.press('/');
          }
          else {
            // reset
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('0');
          }
          break;
      }
      break;
    case 1:
      switch (col)
      {
        case 0: // jog 1mm
          check_jogmode();
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('1');
          break;
        case 1: // jog 0.1mm
          check_jogmode();
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('2');
          break;
        case 2: // jog 0.01mm
          check_jogmode();
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('3');
          break;
        case 3: // jog cont / step
          if (shift_active) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('5');
          }
          else {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('4');
          }
          jogmode_set = true;
          break;
      }
      break;
    case 2:
      switch (col)
      {
        case 0: // X axis
          selected_axis = 0;
          press_active = false;
          break;
        case 1: // Y axis
          selected_axis = 1;
          press_active = false;
          break;
        case 2: // Z axis / probe z
          if (shift_active) {
            // todo
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('8');
          }
          else {
            selected_axis = 2;
            press_active = false;
          }
          break;
        case 3: // zero selected axis
          if (shift_active) {
            // zero all
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('x');
            Keyboard.releaseAll();
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('y');
            Keyboard.releaseAll();
            Keyboard.press(KEY_LEFT_ALT );
            Keyboard.press('z');
            Keyboard.releaseAll();
            press_active = false;
          }
          else {
            switch (selected_axis)
            {
              case 0: // zero X
                Keyboard.press(KEY_LEFT_ALT);
                Keyboard.press('x');
                break;
              case 1: // zero Y
                Keyboard.press(KEY_LEFT_ALT);
                Keyboard.press('y');
                break;
              case 2: // zero Z
                Keyboard.press(KEY_LEFT_ALT );
                Keyboard.press('z');
                break;
            }
          }
          break;
      }
      break;
    case 3:
      switch (col)
      {
        case 0: // shift
          shift_active = true;
          press_active = false;
          break;
        case 1: // goto 0
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press('o');
          break;
        case 2: // jog +Y
          Keyboard.press(KEY_UP_ARROW);
          break;
        case 3: // jog +Z / feed+
          if (shift_active) {
            Keyboard.press(KEY_F11);
          }
          else {
            Keyboard.press(KEY_PAGE_UP);
          }
          break;
      }
      break;
    case 4:
      switch (col)
      {
        case 0: // jog -X
          Keyboard.press(KEY_LEFT_ARROW);
          break;
        case 1: // jog +X
          Keyboard.press(KEY_RIGHT_ARROW);
          break;
        case 2: // jog -Y
          Keyboard.press(KEY_DOWN_ARROW);
          break;
        case 3: // jog -Z / feed-
          if (shift_active) {
            Keyboard.press(KEY_F10);
          }
          else {
            Keyboard.press(KEY_PAGE_DOWN);
          }
          break;
      }
      break;
  }
}

void releaseMacro(uint8_t row, uint8_t col)
{
  // check for shift pressed
  if ( row == 3 and col == 0) {
    shift_active = false;
  }

  Keyboard.releaseAll();


}

// check if jogmode is set, otherwise set to continous
void check_jogmode(void) {
  if (!jogmode_set) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('5');
    jogmode_set = true;
  }
}
