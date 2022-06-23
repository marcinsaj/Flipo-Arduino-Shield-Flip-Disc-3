//nano 33 iot


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
  pinMode(DIN_PIN, OUTPUT);
  digitalWrite(DIN_PIN, LOW);
 
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
 
  pinMode(CLK_PIN, OUTPUT);
  digitalWrite(CLK_PIN, LOW);

  pinMode(EN_VF, OUTPUT);
  digitalWrite(EN_VF, LOW);

  ClearController();            // Clear all Flipo #3 Controllers
delay(5000);
  PrepareCurrentPulse();        // Prepare current pulse
}

void loop()
{  
  for(int i = 0; i < 12; i++)
  {
    SetDisc(i);
    delay(10); 
  }

  for(int i = 0; i < 12; i++)
  {
    ResetDisc(i);
    delay(10);  
  }
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
  while (value < 900);                  // ~3V this voltage means that the current pulse is ready
 
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
  
  delayMicroseconds(1500);                           // Flip-disc required 1ms current pulse to flip
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
