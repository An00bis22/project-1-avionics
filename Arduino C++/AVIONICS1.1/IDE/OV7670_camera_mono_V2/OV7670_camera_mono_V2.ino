
#include <Wire.h>

/* Arduino */
#define D0 A0                     //data lines from OV7670
#define D1 A1
#define D2 A2
#define D3 A3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

/* OV7670  */
#define OV7670 0x21               // OV7670 address
#define PCLK  8                   //pixel clock from OV7670
#define HREF  9                   //horizontal sync from 0V7670 (not used)
#define VSYNC 10                  //vertical sync from OV7670
#define XCLK  11                  //8MHz output to OV7670

/* Flags */
boolean Start = false;

// ====================
// setup()
// ====================
void setup()
{
  /* Disable Arduino's 5 volt internal pullup resistors */
  MCUCR |= (1 << PUD);            // ATmega328P datasheet 14.4.1

  /* configure Arduino inputs */
  pinMode(VSYNC, INPUT);          // OV7670 timing
  pinMode(HREF, INPUT);
  pinMode(PCLK, INPUT);
  pinMode(D0, INPUT);             //OV7670 data
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);

  /* Generate 8MHz XCLK on pin 11 */
  noInterrupts();
  pinMode(XCLK, OUTPUT);
  TCCR2A = (1 << COM2A0) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << WGM22) | (1 << CS10);
  OCR2A = 0;
  interrupts();

  /* Configure serial port */
  Serial.begin(74880);

  /* Initialise the I2C bus */
  Wire.begin();

  /* Restore default register values */
  reset_registers();

  /* Initialise OV7670 camera chip */
  initialise_OV7670();

  /*
    Debug use only ...
    Comment out the main loop() when using.
  */
  //read_registers();                          // Display register contents

}

// =============
// main loop
// =============
void loop()
{
  /* Odd frame */
  if (Start)
  {
    Start = false;                            // Run once
    capture(640, 480);                        // Capture luminance data
  }
}

// ====================
// serial event()
// ====================
void  serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char) Serial.read();
    if ((inChar == 'c') | (inChar == 'C'))
    {
      Start = true;                           // Toggle start flag
    }
  }
}


void capture(int width, int height)

{
  noInterrupts();                                       // Disable interrupts

  /* Locals */
  int column_counter = width;                           // Column counter
  int row_counter = height;                             // Row counter
  byte data;                                            // OV7670 output data

  /* Wait for the frame to start */
  while (!(PINB & B00000100));                          // Wait until VSYNC pin 10 goes high

  /* For each row */
  while (row_counter--)                                 // Row down-counter
  {
    column_counter = width;                             // Reset column down-counter

    /* For each column */
    while (column_counter--)                            // Column down-counter
    {
      /* Skip first byte */
      while (PINB & B00000001);                         // Wait until PCLK pin 8 is low
      while (!(PINB & B00000001));                      // Wait until PCLK pin 8 is high

      /* Read second byte */
      while (PINB & B00000001);                         // Wait until PCLK pin 8 is low
      data = (PIND & B11110000) | (PINC & B00001111);   // Read data
      if (data == '\n') data--;                         // '\n' characters not allowed
      Serial.write(data);                               // Send data to PC
      while (!(PINB & B00000001));                      // Wait until PCLK pin 8 is high
    }
  }
  /* Send '\n' terminator */
  Serial.write('\n');                                   // Send 'linefeed' to PC
  interrupts();                                         // Enable interrupts
}

// ====================
// initialise_OV7670()
// ====================
void initialise_OV7670()
{
  /* PCLK */
  write_register(0x11, B10111111);                      // CLCKRC  Finternal=XCLK/(Bits[5:0]+1)=8MHz/64=0.125MHz (8uS)
  write_register(0x15, B00100000);                      // COM10   Bit[5]=PCLK does not toggle during blanking


  write_register(0x17, 0x13);                // HSTART   B00010011110=158  HSTART[7:0],HREF[2:0]
  write_register(0x18, 0x01);                // HSTOP    B00000001110=14   HSTOP[7:0],HREF[5:3]
  write_register(0x32, 0xB6);                // HREF     B10110110

  // ----- manual gain
 
  write_register(0x13, B00001000);        // COM8    Disable AGC/AWB/AEC (was 0x8F)
  write_register(0x07, B00000000);        // AECHH   AEC[15:10]          (was 0x00)
  write_register(0x10, B00000000);        // AECH    AEC[9:2]            (was 0x40)
  write_register(0x04, B00000000);        // COM1    AEC[1:0]            (was 0x00)
}

// ====================
// read_register()
// ====================
void read_register(byte regID)
{
  // ----- point to register
  Wire.beginTransmission(OV7670);           // I2C address
  Wire.write(regID);                        // Identify the register
  Wire.endTransmission();
  delay(10);                                // The datasheet says 1mS ... at least 3mS needed in practice

  // ----- read data
  Wire.requestFrom(OV7670, 1);              // Request one byte
  if (Wire.available())
  {
    while (!(UCSR0A & (1 << UDRE0)));       // Wait for TX shift register to empty
    show_hex(regID);                        // Display regID
    while (!(UCSR0A & (1 << UDRE0)));       // Wait for TX shift register to empty
    UDR0 = ':';                             // Display ':' delimiter
    while (!(UCSR0A & (1 << UDRE0)));       // Wait for TX shift register to empty
    show_hex(Wire.read());                  // Display data
    while (!(UCSR0A & (1 << UDRE0)));       // Wait for TX shift register to empty
    UDR0 = '\n';                            // Send linefeed
  }
}

// ====================
// read_registers()
// ====================
void read_registers()
{
  for (byte regID = 0x00; regID < 0xCA; regID++)
  {
    read_register(regID);
  }
}

// ====================
// reset_registers()
// ====================
void reset_registers()
{
  write_register(0x12, 0x80);               // See section "8.1.1 Register Reset"
  delay(100);                               // Allow registers to settle
}

// ====================
// show_hex()
// ====================
void show_hex(byte data)
{
  /* separate byte into nibbles */
  byte MSN = data >> 4;                     // Most significant nibble
  byte LSN = data & B00001111;              // Meast significant nibble

  /* display MSN */
  while (!(UCSR0A & (1 << UDRE0)));         // Wait for TX shift register to empty
  if (MSN == B00000000)  UDR0 = '0';
  if (MSN == B00000001)  UDR0 = '1';
  if (MSN == B00000010)  UDR0 = '2';
  if (MSN == B00000011)  UDR0 = '3';
  if (MSN == B00000100)  UDR0 = '4';
  if (MSN == B00000101)  UDR0 = '5';
  if (MSN == B00000110)  UDR0 = '6';
  if (MSN == B00000111)  UDR0 = '7';
  if (MSN == B00001000)  UDR0 = '8';
  if (MSN == B00001001)  UDR0 = '9';
  if (MSN == B00001010)  UDR0 = 'A';
  if (MSN == B00001011)  UDR0 = 'B';
  if (MSN == B00001100)  UDR0 = 'C';
  if (MSN == B00001101)  UDR0 = 'D';
  if (MSN == B00001110)  UDR0 = 'E';
  if (MSN == B00001111)  UDR0 = 'F';

  /* display LSN */
  while (!(UCSR0A & (1 << UDRE0)));         // Wait for TX shift register to empty
  if (LSN == B00000000)  UDR0 = '0';
  if (LSN == B00000001)  UDR0 = '1';
  if (LSN == B00000010)  UDR0 = '2';
  if (LSN == B00000011)  UDR0 = '3';
  if (LSN == B00000100)  UDR0 = '4';
  if (LSN == B00000101)  UDR0 = '5';
  if (LSN == B00000110)  UDR0 = '6';
  if (LSN == B00000111)  UDR0 = '7';
  if (LSN == B00001000)  UDR0 = '8';
  if (LSN == B00001001)  UDR0 = '9';
  if (LSN == B00001010)  UDR0 = 'A';
  if (LSN == B00001011)  UDR0 = 'B';
  if (LSN == B00001100)  UDR0 = 'C';
  if (LSN == B00001101)  UDR0 = 'D';
  if (LSN == B00001110)  UDR0 = 'E';
  if (LSN == B00001111)  UDR0 = 'F';
}

// ====================
// write_register()
// ====================
void write_register(byte regID, byte regVal)
{
  // ----- point to register
  Wire.beginTransmission(OV7670);           // I2C address
  Wire.write(regID);                        // Identify the register
  delay(10);

  // ----- write data to register
  Wire.write(regVal);                       // Write the data
  Wire.endTransmission();
  delay(10);
}
