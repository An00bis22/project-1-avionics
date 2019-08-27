import meter.*;
import processing.serial.*;

Serial port;
String[] list;

Meter m, m2;

void setup() {
  size(950, 400);
  background(0);
  
  port = new Serial(this, "COM5", 19200);

  fill(120, 50, 0);
  m = new Meter(this, 25, 100);
  // Adjust font color of meter value  
  m.setTitleFontSize(22);
  m.setTitleFontName("Times New Roman bold");
  m.setTitle("Ambient Temperature (C)");
  m.setDisplayDigitalMeterValue(true);
  
  // Meter Scale
  String[] scaleLabelsT = {"0", "5", "10", "15", "20", "25", "30", "35", "40" , "45" , "50"};
  m.setScaleLabels(scaleLabelsT);
  m.setScaleFontSize(18);
  m.setScaleFontName("Times New Roman bold");
  m.setScaleFontColor(color(200, 30, 70));
  
  m.setArcColor(color(141, 113, 178));
  m.setArcThickness(10);
  m.setMaxScaleValue(39.3);
  
  m.setNeedleThickness(3);
  
  m.setMinInputSignal(0);
  m.setMaxInputSignal(50);

  // A second meter for reference
  
}

public void draw() {
  
  textSize(30);
  fill(0, 255, 0);
//  text("Ambient Temperature", 270, 50);
  
  if (port.available() > 0) {
    String val = port.readString();
    list = split(val, ',');
    float temperature = float(list[0]);

    
    println("RAW CPU Temp: " + temperature + " C  " );
    
    m.updateMeter(int(temperature));
 
  }
}
