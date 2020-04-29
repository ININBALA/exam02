#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);

Timer stop;
InterruptIn btnRecord(SW2);
InterruptIn btnFlag(SW3);
DigitalOut redLED(LED1);

EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
Thread t1, t2;

int m_addr = FXOS8700CQ_SLAVE_ADDR1;
int idR[32] = {0};
int indexR = 0;
int ledblink;
float t = 0, total0 = 0, total1 = 0;

void blink(){
    redLED = !redLED;
}
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char tmp = addr;
   i2c.write(m_addr, &tmp, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}


void record(void) {
   t = 0.1;
   float x=0.0, y=0.0, z=0.0, dis0 = 0, dis1 = 0;
   int16_t acc16;
   uint8_t res[6];
   FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
   acc16 = (res[0] << 6) | (res[1] >> 2);
   if (acc16 > UINT14_MAX/2)
     acc16 -= UINT14_MAX;
   x = ((float)acc16) / 4096.0f;
   acc16 = (res[2] << 6) | (res[3] >> 2);
   if (acc16 > UINT14_MAX/2)
     acc16 -= UINT14_MAX;
   y = ((float)acc16) / 4096.0f;
   acc16 = (res[4] << 6) | (res[5] >> 2);
   if (acc16 > UINT14_MAX/2)
     acc16 -= UINT14_MAX;
   z = ((float)acc16) / 4096.0f;  
   dis0 = x * 9.8 * t * t / 2;
   total0 += dis0;
   dis1 = x * 9.8 * t * t / 2;
   total1 += dis0;
   printf("%1.4f  %1.4f  %1.4f  %1.4f  %1.4f\n",x ,y ,z, total0, total1 );
}
void flagWrong(void) {printf("---delete---\n");}
void stopRecord(void) {
    queue1.cancel(ledblink);
    redLED = 1;
    for (auto &i : idR)
        queue1.cancel(i);
}
void startRecord(void) {
 // printf("---start---\n");
  idR[indexR++] = queue1.call_every(100,record);
  ledblink = queue1.call_every(300, blink);
  indexR = indexR % 32;
  queue1.call_in(10100, stopRecord);
}

void initFXOS8700Q(void) {
  uint8_t data[2];
  FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
  data[1] |= 0x01;
  data[0] = FXOS8700Q_CTRL_REG1;
  FXOS8700CQ_writeRegs(data, 2);
}

int main() {
    redLED = 1;
    initFXOS8700Q();
    t1.start(callback(&queue1, &EventQueue::dispatch_forever));
   // t2.start(callback(&queue2, &EventQueue::dispatch_forever));
    btnRecord.fall(queue1.event(startRecord));
    //btnFlag.fall(queue.event(flagWrong));
    //btnRecord.fall(queue2.event(test));
}