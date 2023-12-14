#include <can-serial.h>
#include <mcp2515_can.h>
#include <mcp_can.h>

/* Constants */
#define SPEED_SENSOR_PIN 3             // Speed Sesnor Pin
#define CAN_HAT_CS_PIN 9               // CAN HAT Pin
#define RPM_SENSOR_MAX 1800            // Max. RPM of sensor [1/min]

/* Global Variables */ 
const unsigned long can_id = 0x100;    // CAN Device address
const int can_dlc = 2;                 // CAN message data length (number of bytes in frame)
uint8_t data[can_dlc];                 // CAN message payload
unsigned short rpm_sensor;             // rounds per minute of speed sensor 
volatile byte pulses;                  // number of pulses within one measurment periode
unsigned long millis_before;           // privous time of measurement
double sample_rate;                    // sample rate in ms (the time between two calculations)
const unsigned int pulse_per_turn = 20;// Speedsensor Disc Resolution = 20 slots

mcp2515_can CAN(CAN_HAT_CS_PIN);

void setup() {
  rpm_sensor = 0;
  pulses = 0;
  millis_before = 0;
  // calculate sample rate (Nyquist) in ms
  sample_rate = (1/(2*(RPM_SENSOR_MAX/60)))*1000; 
  // set speedsensor as input
  pinMode(SPEED_SENSOR_PIN, INPUT);
  // attach interrupt to pin with ISR function count and trigger on falling edge
  attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN), count, FALLING);
  // Start CAN interface with 500KBps sampling rate
  CAN.begin(CAN_500KBPS);
  Serial.begin(9600);
}

/*send rpm to CAN*/
void send_to_can() {
  // shift 8 bits to the right and mask with 0xFF to get the first byte
  data[0] = (rpm_sensor >> 8) & 0xFF; 
  // mask with 0xFF to get the second byte     
  data[1] = rpm_sensor & 0xFF;          
  // send to CAN Bus   
  CAN.sendMsgBuf(can_id, CAN_STDID ,can_dlc, data);
}

/*ISR to count the pulses*/
void count() {
  pulses++;
}

void read_rpm_sensor(){
  // run for sample rate
  if (millis() - millis_before >= sample_rate) {
    //Interrupts are disabled to prevent the pulse counter from being changed during the calculation
    detachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN)); 
    // calculate rpm of sensor
    rpm_sensor = (60*1000/pulse_per_turn) / (millis() - millis_before) * pulses;
    // save time when the last calculation was done
    millis_before = millis(); 
    // reset pulse counter 
    pulses = 0; 
    // Interrupts are enabled again
    attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN), count, FALLING);
    }
}

void print_to_serial_monitor(){
  Serial.print("rpm: ");
  Serial.println(rpm_sensor);
  Serial.print("can frame: ");
  Serial.print(data[0]); 
  Serial.print(" | ");
  Serial.println(data[1]);
}

void loop() {
  read_rpm_sensor();
  send_to_can();
  print_to_serial_monitor();
  delay(1000);
}