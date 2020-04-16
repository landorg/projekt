#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <math.h>

//Code aus der Elegoo_ftf.cpp

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29


#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

//Touch For New ILI9341 TP
#define TS_MINX 120
#define TS_MAXX 900

#define TS_MINY 70
#define TS_MAXY 920

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3 //Ausgang
#define LCD_CD A2 //Ausgang
#define LCD_WR A1 //Ausgang
#define LCD_RD A0 //Ausgang
// optional
#define LCD_RESET A4 //Ausgang

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0xDEFB

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define PENRADIUS 3 // Orig. 3

// ------------------------------------ Initialisiere die Variablen --------------------------------

long crontimer = millis();
String runtime = "00:00:00";

int t_sec = 0;
int t_min = 0;
int t_hour = 0;

int fbutton_id = 0;
int clickcounter = 0;
int screen_id = 0;
int value_bar = 0;

boolean runstop = true;

int analogPin = 15;
int pachse1;
int value_desk6 = 0;
float bar_va = 3;

//Datentyp für Druck im Sensor festlegen
float pressure; // final pressure
float pressure_last_displayed;

#define MINPRESSURE 2
#define MAXPRESSURE 1000

int screen_open_time = -1;

// Sensor Achse 1 Einkalibrieren
int offset = 108; // zero pressure calibrate      108 perfekt
// 115    bei 0,5 soll - ist 0,4
// je größer der wert, desto kleiner am display unten

int fullScale = 1160; // max pressure calibrate   1160 perfekt
// je größer der wert, desto kleiner am display


//-----------------------------------------------------CODE-------------------------
//-----------------------------------------------------Treiber anzeigen-------------------------

void setup(void) {

  // Code Relaisplatte Config - Pin 40 Digital -> Relaisplatte IN1
  pinMode (40, OUTPUT);
  pinMode (41, OUTPUT);
  pinMode (42, OUTPUT);
  pinMode (43, OUTPUT);
  pinMode (44, OUTPUT);
  pinMode (45, OUTPUT);
  pinMode (46, OUTPUT);
  pinMode (47, OUTPUT);

  digitalWrite(40, HIGH);
  digitalWrite(41, HIGH);
  digitalWrite(42, HIGH);
  digitalWrite(43, HIGH);
  digitalWrite(44, HIGH);
  digitalWrite(45, HIGH);
  digitalWrite(46, HIGH);
  digitalWrite(47, HIGH);

  // Code Display
  Serial.begin(9600);
  Serial.println(pachse1);

  tft.reset();
  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
  //  Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
  //  Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
  //  Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
  //  Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
  //  Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x8357) {
  //  Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
  //  Serial.println(F("Found 0x9341 LCD driver"));
  } else {
  //  Serial.print(F("Unknown LCD driver chip: "));
  //  Serial.println(identifier, HEX);
  //  Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
  //  Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
  //  Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
  //  Serial.println(F("If using the breakout board, it should NOT be #defined!"));
  //  Serial.println(F("Also if using the breakout, double-check that all wiring"));
  //  Serial.println(F("matches the tutorial."));
    identifier = 0x9341;
  }

  tft.begin(identifier);
  tft.setRotation(2);             // Bildschirm drehen - Orig. 1 USB rechts, mit 2 USB Oben
  tft.fillScreen(BLACK);


  pressure = max((analogRead(A15) - offset) * 13.7895  / (fullScale - offset) + 0.07, 0);
  pressure_last_displayed = pressure;

  screen_main();
}


//================================================ LOOP ====================================================

void printn(float value, int digits) {

    char buffer[4];     //ex 8

    dtostrf(value, 2, digits, buffer);      // ## Roli erklären bitte - für zweite Kommastelle: (pressure,2, 2, buffer);
    Serial.print("value: ");
    Serial.println(value);
    Serial.print("buffer: ");
    Serial.println(buffer);

    tft.println(buffer);
}

void loop() {
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  //  ============================================ Tasten ================================
  //  =========================================== Hauptmenü ==============================

  // wenn nichts gedrückt wird
  // Code neu
  if (!(p.x > 0 && p.y > 0) && (screen_id == 0)) {
    // pressure conversion
    pressure = max((analogRead(A15) - offset) * 13.7895  / (fullScale - offset) + 0.07, 0);
    Serial.print("pressure: ");
    Serial.println(pressure);
    
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.setCursor(15, 200);    //x,y


    // wenn der unterschied >= 0.1..
    if ((pressure - pressure_last_displayed) * -1 >= 0.1) {

      // nur wenn upgedated wird den wert von pld updaten
      pressure_last_displayed = pressure;

      // schon die 2/3 befehle die des display updaten oder?
      // warum des delay?
      tft.fillRect(15, 200, 100, 40, BLACK);
      printn(pressure, 1);

    }
  }

  // alt if
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = tft.height() - map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    // Display received X an Y value from touch
    // Serial.print("("); Serial.print(p.x);
    // Serial.print(", "); Serial.print(p.y);
    // Serial.println(")");


    //=============================================================================================================
    // Touchfelder erzeugen
    //=============================================================================================================

    //================================== Screen 0 (Main) Felder ===============================
    //Gehe zum Main-Screen
    if (screen_id == 0) {
      //  Größer Linie 4 ok
      if (p.x > 0 && p.y > 250) {
        fbutton_id = 1;
        screen_f1();                          //  Gehe zu Screen 1
      }

      if (p.x > 0 && p.y < 160 && p.y > 70)     // ex else if
        {
        fbutton_id = 2;
        screen_open_time = millis();
        screen_f2();
        }


    //================================== Screen 1 Felder ===============================
    //==================================  Betriebsmodi   ===============================
    } else if (screen_id == 1) {
      //Setup
      if (p.x > 0 && p.y > 287) {
        value_desk6 = 6;
        screen_setup();
      }

      //Drucklos
      if (p.x > 0  && p.y < 287 && p.y > 247) {
        value_desk6 = 5;
        screen_main();
      }

      //Manuell
      if (p.x > 0  && p.y < 247 && p.y > 207) {
        value_desk6 = 4;
        screen_main();
      }

      //Trailer
      if (p.x > 0  && p.y < 207 && p.y > 167) {
        value_desk6 = 3;
        screen_main();
      }

      //Offroad
      if (p.x > 0  && p.y < 167 && p.y > 127) {
        value_desk6 = 2;
        screen_main();
      }

      //Komfort
      if (p.x > 0  && p.y < 127 && p.y > 87) {
        value_desk6 = 1;
        screen_main();
      }

      //Normal
      if (p.x > 0  && p.y < 87 && p.y > 47) {
        value_desk6 = 0;
        screen_main();
      }

    //================================      Screen 1       ===============================
    //===============================   Felder - Achse 1   ===============================
    } else if (screen_id == 2) {
      if (p.x > 0 && p.y < 30 ) {
        // dann gehe zu Hauptmenu
        screen_main();
      }

      if (p.x > 0 && p.x < 115 && p.y > 255) {
        if (bar_va >= 9.90) {
          bar_va = 9.90;
        } else {
          bar_va = bar_va + 0.1;
          screen_open_time = millis();
          digitalWrite(40, LOW);
          (100);                 // Entprellen
          screen_f22();
          digitalWrite(40, HIGH);
        }
      }

      if (p.x > 115 && p.y > 255) {
        if (bar_va < 0.10) {
          bar_va = 0;
          (500);

        } else {
          bar_va = bar_va - 0.1;
          screen_open_time = millis();
          digitalWrite(41, LOW);
          delay(100);                 // Entprellen
          screen_f22();
          digitalWrite(41, HIGH);
        }
      }

      //================================== Screen 3 Felder ===============================
      //F3 screen action
      } else if (screen_id == 3) {

        if (p.x > 200 && p.x < 250 && p.y < 155 && p.y > 95) {
          clickcounter++;
          screen_main();
        }

        if (p.x > 130 && p.x < 170 && p.y < 225 && p.y > 13) {
          value_bar = (216 - p.y + 8) / 2.1;
          tft.fillRect(15 + value_bar * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE);
          tft.fillRect(15, 100, value_bar * 2.9, 30, RED);
          tft.fillRect(120, 40, 120, 40 , BLACK);
          tft.setCursor(125, 50);
          tft.setTextColor(WHITE);
          tft.setTextSize(3);
          tft.print(value_bar);
          tft.println("%");
        }
      }

    //F4 screen action
    if (p.x > 280  && p.y <= 60 && p.y >= 0) {
      fbutton_id = 4;
      if (runstop == true) {
        runstop = false;
      } else {
        runstop = true;
      }
      screen_main();
    }
  }

  delay(500);
}

//===============================================Hauptmenü==============================================================
void screen_main() {
  screen_id = 0;

  // Ab erster Linie Löschen x1,y1,x1,y2
  tft.fillRect(0, 21, 250, 300, BLACK);          

  // Version
  tft.setCursor(5, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("kfz-technik.at");

  //Logo
  tft.setCursor(145, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Rev. 2020-01-19");

  //1. Linie
  tft.drawLine(0, 20, 320, 20, WHITE);     //D line x1,y1,x1,y2

  // Zeile 2
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(5, 30);
  tft.println("Kessel (bar):");           // Druck anzeigen

  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.setCursor(15, 45);    //x,y
  tft.println("10.0");


  //2. Linie
  tft.drawLine(0, 70, 320, 70, WHITE);     //Draw first horizontal line x1,y1,x1,y2

  //Druck Vorderachse - IST
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.setCursor(5, 80);
  tft.println("Vorderachse");

  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(5, 90);
  tft.println("Balgdruck (bar)");

  tft.setCursor(135, 90);
  tft.println("Härte (0-30)");

  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.setCursor(15, 110);    //x,y
  tft.println(bar_va);

  tft.setTextColor(GREEN);
  tft.setCursor(145, 110);    //x,y
  tft.println("15");

  //3. Linie
  tft.drawLine(0, 160, 320, 160, WHITE);     //Draw first horizontal line x1,y1,x1,y2


  //Druck Hinterachse - IST
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.setCursor(5, 170);
  tft.println("Hinterachse");

  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(5, 180);
  tft.println("Balgdruck (bar)");

  tft.setCursor(135, 180);
  tft.println("Beladung (kg)");

  //===============================================Druck Raum==============================================================

  tft.setTextSize(4);
  tft.setTextColor(GREEN);
  tft.setCursor(145, 200);    //x,y
  tft.println("150");

  //4.  Linie
  tft.drawLine(0, 250, 320, 250, WHITE);     //Draw first horizontal line x1,y1,x1,y2

  // Modus
  tft.setTextColor(BLUE);
  tft.setTextSize(1);

  tft.setCursor(5, 260);
  tft.println("Betriebsart:");                  // Betriebsmodus

  tft.setTextColor(GREEN);
  tft.setTextSize(4);
  tft.setCursor(15, 280);    //x,y


  //Variable für Betriebsart darstellen
  if (value_desk6 == 0) {
    tft.println ("Normal");

  } else if (value_desk6 == 1) {
    tft.println ("Komfort");

  } else if (value_desk6 == 2) {
    tft.println ("Offroad");

  } else if (value_desk6 == 3) {
    tft.println ("Trailer");

  } else if (value_desk6 == 4) {
    tft.println ("Manuell");

  } else if (value_desk6 == 5) {
    tft.println ("Drucklos");
  }
}



//================================================ Menü F1 ==============================================
//============================================= Betriebsmodus ==============================================

void screen_f1() {
  screen_id = 1;

  // tft.fillScreen(BLACK);

  tft.fillRect(0, 21, 250, 300, BLACK);          // Ab erster Linie Löschen x1,y1,x1,y2

  // Version
  tft.setCursor(5, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("kfz-technik.at");

  //Logo
  tft.setCursor(145, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Rev. 2020-01-19");

  //1. Linie
  tft.drawLine(0, 20, 320, 20, WHITE);         //D line x1,y1,x1,y2

  // Zeile 2
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(5, 30);
  tft.println("Betriebsart:");
  tft.setTextSize(4);

  // Farben abhängig von Auswahl

  if (value_desk6 == 0) {
    tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }

  tft.setCursor(15, 50-3);    //x,y
  tft.println("Normal");

  if (value_desk6 == 1) {
    tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }
  tft.setCursor(15, 90-3);    //x,y
  tft.println("Komfort");

  if (value_desk6 == 2) {
      tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }
  tft.setCursor(15, 130-3);    //x,y
  tft.println("Offroad");

  if (value_desk6 == 3) {
    tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }
  tft.setCursor(15, 170-3);    //x,y
  tft.println("Trailer");

  if (value_desk6 == 4) {
    tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }
  tft.setCursor(15, 210-3);    //x,y
  tft.println("Manuell");

  if (value_desk6 == 5) {
    tft.setTextColor(GREEN);
  } else {
    tft.setTextColor(WHITE);
  }
  tft.setCursor(15, 250-3);    //x,y
  tft.println("Drucklos");


  tft.setTextColor(GRAY);
  tft.setCursor(15, 290-3);    //x,y
  tft.println("Setup");
}


//=============================================== Menü F2 ==============================================================
//=============================================== Achse 1 ==============================================================

/*void screen_bar_va()
  {
tft.fillRect(15, 110, 40, 40, BLACK)
  }
*/

void screen_f2() {

  screen_id = 2;

  tft.fillRect(15, 110, 80, 50, BLACK);
  tft.setTextColor(YELLOW);               //Schreibe Wert in Gelb
  tft.setTextSize(4);
  tft.setCursor(15, 110);    //x,y
  tft.println(bar_va);

  tft.fillRect(0, 251, 250, 300, BLACK);          // Ab 2. Linie Löschen x1,y1,x1,y2

  tft.fillRect(5, 257, 110, 56, GRAY);       // Button Feld Grau -> x,y,Gesamt nach x, Gesamt nach y
  tft.fillRect(5, 257, 110, 2, WHITE);
  tft.fillRect(5, 257, 2, 56, WHITE);

  tft.setCursor(47, 266);                   // + Button
  tft.setTextColor(BLACK);
  tft.setTextSize(6);
  tft.println("+");

  tft.fillRect(124, 257, 110, 56, GRAY);       // Button Feld Grau -> x,y,Gesamt nach x, Gesamt nach y
  tft.fillRect(124, 257, 110, 2, WHITE);
  tft.fillRect(124, 257, 2, 56, WHITE);

  tft.setCursor(167, 266);                   // - Button
  tft.setTextColor(BLACK);
  tft.setTextSize(6);
  tft.println("-");
}

//screen_id = 22;
void screen_f22() {
  tft.fillRect(15, 110, 80, 50, BLACK);
  tft.setTextColor(YELLOW);               //Schreibe Wert in Gelb
  tft.setTextSize(4);
  tft.setCursor(15, 110);    //x,y
  tft.println(bar_va);
}

/*
  tft.fillRect(115, 150, 80, 40, GRAY);
  tft.setCursor(138, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.println("OK");
*/


/*
  tft.setCursor(220, 50);
 tft.setTextColor(WHITE);
  tft.setTextSize(3);

  tft.print(value_bar);
  tft.setCursor(250, 50);
  tft.println("%");
  tft.fillRect(15 + value_bar * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE); // tft.fillRect(15 + value_bar * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE);
  tft.fillRect(15, 100, value_bar * 2.9, 30, RED);  //  tft.fillRect(15, 100, value_bar * 2.9, 30, RED);

  */


void screen_setup () {
  tft.fillRect(0, 21, 250, 300, BLACK);          // Ab erster Linie Löschen x1,y1,x1,y2

  // Version
  tft.setCursor(5, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("kfz-technik.at");

  //Logo
  tft.setCursor(145, 5);    // x,y
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Rev. 2020-01-19");

  //1. Linie
  tft.drawLine(0, 20, 320, 20, WHITE);         //D line x1,y1,x1,y2

  // Zeile 2
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(5, 30);
  tft.println("Einstellung:");
  tft.setTextSize(4);
}

//============================================================== Screen F2 ==============================================
/*
void screen_f2() {
  screen_id = 2;
  tft.fillScreen(BLACK);
  tft.setCursor(85, 50);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.println("Status:");
  //Generate OK button
  tft.fillRect(115, 150, 80, 40, WHITE);
  tft.setCursor(138, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.println("OK");
  tft.fillCircle(115, 150,100,WHITE);
  tft.fillCircle(115, 150,90,BLACK);
}
*/

/*void screen_f3() {
  screen_id = 3;
  tft.fillScreen(BLACK);
 tft.setCursor(70, 50);
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.print("Zuluft: ");
  tft.setCursor(220, 50);
 tft.setTextColor(WHITE);
  tft.setTextSize(3);

  tft.print(value_bar);
  tft.setCursor(250, 50);
  tft.println("%");
  tft.fillRect(15 +
 * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE); // tft.fillRect(15 + value_bar * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE);
  tft.fillRect(15, 100, value_bar * 2.9, 30, RED);  //  tft.fillRect(15, 100, value_bar * 2.9, 30, RED);
  //Generate OK button
  tft.fillRect(115, 150, 80, 40, WHITE);  // tft.fillRect(115, 150, 80, 40, WHITE);
  tft.setCursor(138, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.println("OK");
}*/

//============================================================== Screen F3 ==============================================
void screen_f3() {
  screen_id = 3;
  tft.fillScreen(BLACK);
  tft.setCursor(125, 50);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.print(value_bar);
  tft.println("%");
  tft.fillRect(15 + value_bar * 2.9, 100, 290 - value_bar * 2.9, 30, WHITE);
  tft.fillRect(15, 100, value_bar * 2.9, 30, RED);
  //Generate OK button
  tft.fillRect(115, 150, 80, 40, WHITE);
  tft.setCursor(138, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.println("OK");
}
