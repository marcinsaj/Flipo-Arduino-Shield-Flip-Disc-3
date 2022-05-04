// Marcin Saj Flipo Project - https://Flipo.io
// https://flipo.io/project/arduino-shield-flip-disc-3
// https://github.com/marcinsaj/Flipo-Arduino-Shield-Flip-Disc-3
// https://flipo.io/project/controller-flip-disc-3
// https://github.com/marcinsaj/Flipo-Controller-Flip-Disc-3
// https://flipo.io/project/pulse-current-power-supply
// https://github.com/marcinsaj/Flipo-Pulse-Current-Power-Supply
//
// This example demonstrates how to use Arduino Shield with 4 Flipo#3 modules
// Buttons - short & long press
//
// Hardware:
// Arduino Shield Flip-disc 3
// 4 x Flipo#3 modules for 3 flip disc - https://flipo.io/project/controller-flip-disc-3
// Pulse Current Power Supply Module for flip discs - https://flipo.io/project/pulse-current-power-supply
// Arduino 5V Nano/Every

#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2

const uint8_t BUTTON_PINS[6] = {9, 8, 7, 6, 5, 4};
boolean DiscState[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
Bounce * buttons = new Bounce[6];

unsigned long buttonPressTimeStamp;

// Declaration of the controller control inputs
#define DIN_PIN     12
#define EN_PIN      11
#define CLK_PIN     10

// Declaration of the PCPS module control input/output
#define EN_VF       A0    // Turn ON/OFF charging PCPS module
#define FB_VF       A1    // Measurement output for checking if the current pulse is ready

uint8_t *discModePointer;
uint16_t value = 0;

// Bit notation for Flipo#3 controller - set flip-discs
// Always active only two bits corresponding to control outputs 
// Check controller documentation and schematic
uint8_t setDisc[]=
{
  0b10000010,
  0b10010000,
  0b10100000  
};

// Bit notation for Flipo#3 controller - reset flip-discs
uint8_t resetDisc[]=
{
  0b00000101,
  0b01000100,
  0b00001100
};

void setup() 
{
  for (int i = 0; i < 6; i++) 
  {
    buttons[i].attach(BUTTON_PINS[i] , INPUT_PULLUP);     // Setup the bounce instance for the current button
    buttons[i].interval(50);                              // Interval in ms
  }

  pinMode(DIN_PIN, OUTPUT);
  digitalWrite(DIN_PIN, LOW);
 
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
 
  pinMode(CLK_PIN, OUTPUT);
  digitalWrite(CLK_PIN, LOW);

  pinMode(EN_VF, OUTPUT);
  digitalWrite(EN_VF, LOW);

  ClearController();            // Clear all Flipo #3 Controllers
  PrepareCurrentPulse();        // Prepare current pulse
  SetAllDiscs();
}

void loop()
{
  for (int i = 0; i < 6; i++)  
  {
    // Update the Bounce instance :
    buttons[i].update();
    
    if (buttons[i].rose()) 
    {
      buttonPressTimeStamp = millis();
    }

    if ( i < 3 && buttons[i].fell()) 
    {
      if (millis() - buttonPressTimeStamp < 500) 
      {
        ToggleDisc(i);
      }
      else
      {
        ToggleDisc(i+3);
      }
    }

    if ( i >= 3 && buttons[i].fell()) 
    {
      if (millis() - buttonPressTimeStamp < 500) 
      {
        ToggleDisc(i+6);
      }
      else
      {
        ToggleDisc(i+3);
      }
    }
  }
}

void SetAllDiscs(void)
{
  for(int i = 0; i < 12; i++)
  {
    SetDisc(i);
    delay(100);
  }
}

void ToggleDisc(uint8_t discNumber)
{
  DiscState[discNumber] = !DiscState[discNumber];

  if(DiscState[discNumber] == HIGH) SetDisc(discNumber);
  if(DiscState[discNumber] == LOW) ResetDisc(discNumber);
}

// Clear all Flipo #3 Controllers
void ClearController(void)
{
  ShiftOutDataStart();

  for(int i = 0; i < 4; i++)
  {
    ShiftOutData(0);
  }

  ShiftOutDataEnd();  
}

// First charging - setup call
void PrepareCurrentPulse(void)
{
  CurrentPulseON();
} 

void CurrentPulseON()
{
  digitalWrite(EN_VF, HIGH);            // Turn ON PCPS module- charging begin

  do {value = analogRead(FB_VF);}       // Measure the voltage of the accumulated charge
  while (value < 500);                  // ~2.5V this voltage means that the current pulse is ready
 
  digitalWrite(EN_VF, LOW);             // Turn OFF PCPS module- charging complete
}

void CurrentPulseOFF(void)
{
  ClearController();                    // Clear all Flipo #3 Controllers
}

void SetDisc(uint8_t discNumber)
{
  discModePointer = setDisc;            // Set pointer to the setDisc bit array
  ModeDisc(discNumber);
}

void ResetDisc(uint8_t discNumber)
{
  discModePointer = resetDisc;          // Set pointer to the resetDisc bit array
  ModeDisc(discNumber);
}

void ModeDisc(uint8_t discNumber)
{
  CurrentPulseON();                     // Prepare current pulse

  ShiftOutDataStart();                  // Transfer data begin   

  // Turn on flip-disc controller corrsponding outputs
  for(int i = 3; i >= 0; i--)
  {
    if(discNumber / 3 == i) ShiftOutData(discModePointer[discNumber % 3]);
    else ShiftOutData(0);
  }

  ShiftOutDataEnd();                    // Transfer data complete
  
  delay(1);                             // Flip-disc required 1ms current pulse to flip
  CurrentPulseOFF();                    // Absolutely required!
                                        // This function here turns off the current pulse 
                                        // and cleans the controller outputs    
}

void ShiftOutDataStart(void)
{
  digitalWrite(EN_PIN, LOW);            // Transfer data begin    
}

void ShiftOutData(uint8_t discNumber)
{
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, discNumber);   // Send data to the controller
}

void ShiftOutDataEnd(void)
{
  digitalWrite(EN_PIN, HIGH);           // Transfer data complete
}
