/**********************************************************************
  OV7670_camera_mono_V2.pde
  Code by lingib
  https://www.instructables.com/member/lingib/instructables/
  Last update 6 December 2018

  This program converts the data stream from "OV7670_camera_mono.ino" into a monochrome image
 
  Change log 6/12/2018:
  Grayscale corrected 
  pixels[i] = color(int(byteBuffer[i])); //now int() ... was byte

  ----------
  COPYRIGHT
  ----------
  This code is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License. If
  not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

// **************************************
//  Globals
// **************************************
import processing.serial.*;                                  // Library
Serial myPort;                                               // Serial object

/* Buffer */
final int maxBytes = 307200 ;                                // 640*480 bytes
byte[] byteBuffer = new byte[maxBytes+1];
int byteCount = 0;
int lf = 10;                                                  // Terminator (linefeed)

/* Image */
int count = 0;                                                // Image number when saving file

// **************************************
//  Setup
// **************************************
void setup() 
{
  size(640, 480);

  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[0], 1000000);      //1Mbps
}

// **************************************
//  Draw (main loop)
// **************************************
void draw() 
{
  /* Capture the entire frame to the byteBuffer[] */
  if (myPort.available() > 0)
  {
    byteCount = myPort.readBytesUntil(lf, byteBuffer);      // Wait here until linefeed received

    /* Display results */
    if (byteCount > 0)
    {
      println(byteCount);                                   // Should read 307201
      byteCount = 0;                                        // Prevent infinite loop
    }
  }

  /* Display the byteBuffer[] contents */
  loadPixels();
  for (int i = 0; i < maxBytes; i++ ) 
  {
    pixels[i] = color(int(byteBuffer[i]));
  }
  updatePixels();
}

// **************************************
//  Keypress
// **************************************
void keyPressed()
{
  /*
    PRESS:
    'c' = capture image to screen
    's' = save image to file
  */
  myPort.write(key);
  
  /* Save numbered image to processing folder */
  if ((key == 's') | (key == 'S'))
  {
    save("image_" + count + ".jpg");
    count++;
 }
}
