import meter.*;
import processing.serial.*;

Serial port;
String[] list;

Meter m, m2;

void setup() {
  size(840, 540);
  background(0);
  
  port = new Serial(this, "COM3", 19200);

  fill(120, 50, 0);
  m = new Meter(this, 200, 200);
  // Adjust font color of meter value  
  m.setTitleFontSize(22);
  m.setTitleFontName("Times New Roman bold");
  m.setTitle("Ambient Temperature (C)");
  m.setDisplayDigitalMeterValue(true);
  
  // Meter Scale
  String[] scaleLabelsT = {"0", "5", "10", "15", "20", "25", "30", "35", "40" , "45" , "50" , "100"};
  m.setScaleLabels(scaleLabelsT);
  m.setScaleFontSize(18);
  m.setScaleFontName("Times New Roman bold");
  m.setScaleFontColor(color(200, 30, 70));
  
  m.setArcColor(color(141, 113, 178));
  m.setArcThickness(10);
  m.setMaxScaleValue(32);
  
  m.setNeedleThickness(3);
  
  m.setMinInputSignal(-10);
  m.setMaxInputSignal(100);

  // A second meter for reference

  int mx = m.getMeterX();
  int my = m.getMeterY();
  int mw = m.getMeterWidth();
  
  m2 = new Meter(this, mx + mw + 200, my);
  m2.setTitleFontSize(22);
  m2.setTitleFontName("Times New Roman bold");
  m2.setTitle("True Air Speed (m/s)");
  m2.setDisplayDigitalMeterValue(true);
  
  String[] scaleLabelsH = {"0", "5", "10", "15", "20", "25", "30"};
  m2.setScaleLabels(scaleLabelsH);
  m2.setScaleFontSize(18);
  m2.setScaleFontName("Times New Roman bold");
  m2.setScaleFontColor(color(200, 30, 70));
  
  m2.setArcColor(color(141, 113, 178));
  m2.setArcThickness(10);
  m2.setMaxScaleValue(40);
  
  m2.setNeedleThickness(3);
  
  m2.setMinInputSignal(-10);
  m2.setMaxInputSignal(100);
  
}

public void draw() {
  
  textSize(30);
  fill(0, 255, 0);
//  text("Ambient Temperature", 270, 50);
  
  if (port.available() > 0) {
    String val = port.readString();
    list = split(val, ',');
    float temperature = float(list[0]);
    float accelerometer_x = float(list[1]);

    
    println("RAW CPU Temp: " + temperature + " C  " + "Raw Sensor Velocity: " + accelerometer_x + " m/s");
    
    m.updateMeter(int(temperature));
    m2.updateMeter(int(accelerometer_x));
 
  }
}
