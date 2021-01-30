/******************************************************************************************
/*	mySD.cpp
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	implements used functions of SD
******************************************************************************************/
#include "mySD.h"
#include "SdFat.h"
#include "BufferedPrint.h"
#include "FreeStack.h"
#include <WiFi101.h>
#include "tcp.h"

#include "TimerSetup.h"
#include "myrtc.h"


char binName[] = LOG_FILE_NAME;
//------------------------------------------------------------------------------
// Pin definitions.
//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
const int8_t ERROR_LED_PIN = 0;

// SD chip select pin.
const uint8_t SD_CS_PIN = CS_SD;



// Maximum length name including zero byte.
const size_t NAME_DIM = 40;

//------------------------------------------------------------------------------
// FIFO size definition. Use a multiple of 512 bytes for best performance.
//
const size_t FIFO_SIZE_BYTES = 4*512;
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_adc.bin"
//==============================================================================
const uint32_t MAX_FILE_SIZE = MAX_FILE_SIZE_MiB << 20;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_SPEED)



SdFat32 sd;
typedef File32 file_t;


const size_t BLOCK_MAX_COUNT = DATA_DIM16;

typedef block16_t block_t;


// Size of FIFO in blocks.
size_t const FIFO_DIM = FIFO_SIZE_BYTES/sizeof(block_t);
block_t* fifoData;
volatile size_t fifoCount = 0; // volatile - shared, ISR and background.
size_t fifoHead = 0;  // Only accessed by ISR during logging.
size_t fifoTail = 0;  // Only accessed by writer during logging.
// Pointer to current buffer.
block_t* isrBuf = nullptr;
// overrun count
uint16_t isrOver = 0;



file_t binFile;
file_t csvFile;

MyRTC* myrtc;
void setmyrtc(MyRTC * addr){
  myrtc = addr;
} 

//==============================================================================
// Error messages stored in flash.
#define error(msg) (Serial.println(F(msg)),errorHalt())
#define assert(e) ((e) ? (void)0 : error("assert: " #e))
//------------------------------------------------------------------------------
//
void fatalBlink() {
  while (true) {
    if (ERROR_LED_PIN >= 0) {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(200);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(200);
    }
  }
}
//------------------------------------------------------------------------------
void errorHalt() {
  // Print minimal error data.
  // sd.errorPrint(&Serial);
  // Print extended error info - uses extra bytes of flash.
  sd.printSdError(&Serial);
  // Try to save data.
  binFile.close();
  fatalBlink();
}

//------------------------------------------------------------------------------
void printUnusedStack() {
  Serial.print(F("\nUnused stack: "));
  Serial.println(UnusedStack());
}


void initSD(){
    Serial.println("init SD");
    #if !ENABLE_DEDICATED_SPI
        Serial.println(F(
            "\nFor best performance edit SdFatConfig.h\n"
            "and set ENABLE_DEDICATED_SPI nonzero"));
    #endif  // !ENABLE_DEDICATED_SPI
    
    if (!sd.begin(SD_CONFIG)) {
        error("sd.begin failed");
    }
    Serial.println("SD present");
}

//------------------------------------------------------------------------------
// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  block_t* pd;
  metadata_t* pm;
  uint32_t t0 = millis();
  // Use fast buffered print class.
  BufferedPrint<file_t, 64> bp(&csvFile);
  block_t binBuffer[FIFO_DIM];
  assert(sizeof(block_t) == sizeof(metadata_t));
  binFile.rewind();
  uint32_t tPct = millis();
  bool doMeta = true;
  while (!Serial.available()) {
    pd = binBuffer;
    int nb = binFile.read(binBuffer, sizeof(binBuffer));
    if (nb < 0) {
      error("read binFile failed");
    }
    size_t nd = nb/sizeof(block_t);
    if (nd < 1) {
      break;
    }
    if (doMeta) {
      doMeta = false;
      pm = (metadata_t*)pd++;
      if (PIN_COUNT != pm->pinCount) {
        error("Invalid pinCount");
      }
      bp.print(F("Interval,"));
      float intervalMicros = 1.0e6*pm->sampleInterval/(float)pm->cpuFrequency;
      bp.print(intervalMicros, 4);
      bp.println(F(",usec"));
      for (uint8_t i = 0; i < PIN_COUNT; i++) {
        if (i) {
          bp.print(',');
        }
        bp.print(F("pin"));
        bp.print(pm->pinNumber[i]);
      }
      bp.println(";");
      if (nd-- == 1) {
        break;
      }
    }
    for (size_t i = 0; i < nd; i++, pd++) {
      if (pd->overrun) {
        bp.print(F("OVERRUN,"));
        bp.println(pd->overrun);
      }
      for (size_t j = 0; j < pd->count; j += PIN_COUNT) {
        for (size_t i = 0; i < PIN_COUNT; i++) {
          bp.print(pd->data[i + j]);
          if(i<PIN_COUNT){
            bp.print(",");
          }
          // if (!bp.printField(pd->data[i + j], i == (PIN_COUNT-1) ? '\n' : ',')) {
          //   error("printField failed");
          // }
        }
        bp.print(";");
      }
      bp.println();
    }
    if ((millis() - tPct) > 1000) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
      }
    }
  }
  if (!bp.sync() || !csvFile.close()) {
    error("close csvFile failed");
  }
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
}



void csvOverTcp(){
  Serial.println("Sending now: ");
  WiFiClient tcp = giveclient();
  if(tcp.connected()){
  uint8_t lastPct = 0;
  uint32_t tPct = millis();
  uint32_t t0 = millis();

  char csvName[NAME_DIM];
  binFile.getName(csvName, sizeof(csvName));
    char* dot = strchr(csvName, '.');
  if (!dot) {
    error("no dot in binName");
  }
  strcpy(dot + 1, "csv");
  csvFile.open(csvName, O_RDONLY);

  char line[1000];
  while (csvFile.available()) {
    int n = csvFile.fgets(line, sizeof(line));
    if (n <= 0) {
      error("fgets failed");
    }
    if (line[n-1] != '\n' && n == (sizeof(line) - 1)) {
      error("line too long");
    }
    if (!tcp.println(line)) {
      error("parseLine failed");
    }
    if ((millis() - tPct) > 1000) {
      uint8_t pct = csvFile.curPosition()/(csvFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
      }
    }
  }
  tcp.println("end of transmission");
  csvFile.close();
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
  }
}

//------------------------------------------------------------------------------
void createBinFile() {
  binFile.close();
  while (sd.exists(binName)) {
    char* p = strchr(binName, '.');
    if (!p) {
      error("no dot in filename");
    }
    while (true) {
      p--;
      if (p < binName || *p < '0' || *p > '9') {
        error("Can't create file name");
      }
      if (p[0] != '9') {
        p[0]++;
        break;
      }
      p[0] = '0';
    }
  }
  Serial.print(F("Opening: "));
  Serial.println(binName);
  if (!binFile.open(binName, O_RDWR | O_CREAT)) {
    error("open binName failed");
  }
  Serial.print(F("Allocating: "));
  Serial.print(MAX_FILE_SIZE_MiB);
  Serial.println(F(" MiB"));
  if (!binFile.preAllocate(MAX_FILE_SIZE)) {
    error("preAllocate failed");
  }
}

//------------------------------------------------------------------------------
bool createCsvFile() {
  char csvName[NAME_DIM];

  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return false;
  }
  binFile.getName(csvName, sizeof(csvName));
  char* dot = strchr(csvName, '.');
  if (!dot) {
    error("no dot in binName");
  }
  strcpy(dot + 1, "csv");
  if (!csvFile.open(csvName, O_WRONLY|O_CREAT|O_TRUNC)) {
    error("open csvFile failed");
  }
  Serial.print(F("Writing: "));
  Serial.print(csvName);
  Serial.println(F(" - type any character to stop"));
  return true;
}

//------------------------------------------------------------------------------
void openBinFile() {
  char name[NAME_DIM];
  clearSerialInput();
  Serial.println(F("Enter file name"));
  if (!serialReadLine(name, sizeof(name))) {
    return;
  }
  if (!sd.exists(name)) {
    Serial.println(name);
    Serial.println(F("File does not exist"));
    return;
  }
  binFile.close();
  if (!binFile.open(name, O_RDWR)) {
    Serial.println(name);
    Serial.println(F("open failed"));
    return;
  }
  Serial.println(F("File opened"));
}
//------------------------------------------------------------------------------
// Print data file to Serial
void printData() {
  block_t buf;
  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return;
  }
  binFile.rewind();
  if (binFile.read(&buf , sizeof(buf)) != sizeof(buf)) {
    error("Read metadata failed");
  }
  Serial.println(F("Type any character to stop"));
  delay(1000);
  while (!Serial.available() &&
         binFile.read(&buf , sizeof(buf)) == sizeof(buf)) {
    if (buf.count == 0) {
      break;
    }
    if (buf.overrun) {
      Serial.print(F("OVERRUN,"));
      Serial.println(buf.overrun);
    }
    for (size_t i = 0; i < buf.count; i++) {
      Serial.print(buf.data[i], DEC);
      if ((i+1)%PIN_COUNT) {
        Serial.print(',');
      } else {
        Serial.println();
      }
    }
  }
  Serial.println(F("Done"));
}


void bufferData(uint16_t d){
  if (!isrBuf) {
    if (fifoCount < FIFO_DIM) {
      isrBuf = fifoData + fifoHead;
    } else {
      // no buffers - count overrun
      if (isrOver < 0XFFFF) {
        isrOver++;
      }
      return;
    }
  }
  // Store ADC data.
  isrBuf->data[isrBuf->count++] = d;

  // Check for buffer full.
  if (isrBuf->count >= BLOCK_MAX_COUNT) {
    fifoHead = fifoHead < (FIFO_DIM - 1) ? fifoHead + 1 : 0;
    fifoCount++;
    // Set buffer needed and clear overruns.
    isrBuf = nullptr;
    isrOver = 0;
  }
}

//------------------------------------------------------------------------------
// log data
void logData() {
  Serial.print("the block max count is: ");
  Serial.println(BLOCK_MAX_COUNT);

  uint32_t t0;
  uint32_t t1;
  uint32_t overruns =0;
  uint32_t count = 0;
  uint32_t maxLatencyUsec = 0;
  size_t maxFifoUse = 0;
  block_t fifoBuffer[FIFO_DIM];

  metadata_t* meta = (metadata_t*)fifoBuffer;
  meta->adcFrequency = 50;
  meta->cpuFrequency = 42000;
  meta->pinCount = 2;
  meta->pinNumber[0] = 0; 
  meta->pinNumber[1] = 1; 
  meta->recordEightBits = 0;
  meta->sampleInterval = 12;

  if (sizeof(metadata_t) != binFile.write(fifoBuffer, sizeof(metadata_t))) {
    error("Write metadata failed");
  }
  fifoCount = 0;
  fifoHead = 0;
  fifoTail = 0;
  fifoData = fifoBuffer;
  // Initialize all blocks to save ISR overhead.
  memset(fifoBuffer, 0, sizeof(fifoBuffer));

  Serial.println(F("Logging - type any character to stop"));
  // Wait for Serial Idle.
  Serial.flush();
  delay(10);

  t0 = millis();
  t1 = t0;
  // Start logging interrupts.
  uint16_t wert = 0;
  int addieren = 1;
  ts_startTimer();
  while (1) {
    uint32_t m;
    noInterrupts();
    size_t tmpFifoCount = fifoCount;
    interrupts();
    if (tmpFifoCount) {
      block_t* pBlock = fifoData + fifoTail;
      // Write block to SD.
      m = micros();
      if (sizeof(block_t) != binFile.write(pBlock, sizeof(block_t))) {
        error("write data failed");
      }
      m = micros() - m;
      t1 = millis();
      if (m > maxLatencyUsec) {
        maxLatencyUsec = m;
      }
      if (tmpFifoCount >maxFifoUse) {
        maxFifoUse = tmpFifoCount;
      }
      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun) {
        overruns += pBlock->overrun;
        if (ERROR_LED_PIN >= 0) {
          digitalWrite(ERROR_LED_PIN, HIGH);
        }
      }
      // Initialize empty block to save ISR overhead.
      pBlock->count = 0;
      pBlock->overrun = 0;
      fifoTail = fifoTail < (FIFO_DIM - 1) ? fifoTail + 1 : 0;

      noInterrupts();
      fifoCount--;
      interrupts();
      
      if (binFile.curPosition() >= MAX_FILE_SIZE) {
          ts_stopTimer();
      }
    }
    if (Serial.available() || myrtc->getStopLogging()) {
      // Stop ISR interrupts.
      ts_stopTimer();
      break;
    }

  }
  Serial.println();
  // Truncate file if recording stopped early.
  if (binFile.curPosition() < MAX_FILE_SIZE) {
    Serial.println(F("Truncating file"));
    Serial.flush();
    if (!binFile.truncate()) {
      error("Can't truncate file");
    }
  }
  Serial.print(F("Max write latency usec: "));
  Serial.println(maxLatencyUsec);
  Serial.print(F("Record time sec: "));
  Serial.println(0.001*(t1 - t0), 3);
  Serial.print(F("Sample count: "));
  Serial.println(count);
  Serial.print(F("Overruns: "));
  Serial.println(overruns);
  Serial.print(F("FIFO_DIM: "));
  Serial.println(FIFO_DIM);
  Serial.print(F("maxFifoUse: "));
  Serial.println(maxFifoUse + 1);  // include ISR use.
  Serial.println(F("Done"));
}

void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}

bool serialReadLine(char* str, size_t size) {
  size_t n = 0;
  while(!Serial.available()) {
  }
  while (true) {
    int c = Serial.read();
    if (c < ' ') break;
    str[n++] = c;
    if (n >= size) {
      Serial.println(F("input too long"));
      return false;
    }
    uint32_t m = millis();
    while (!Serial.available() && (millis() - m) < 100){}
    if (!Serial.available()) break;
  }
  str[n] = 0;
  return true;
}

void readls(){
  Serial.println(F("ls:"));
  sd.ls(&Serial, LS_DATE | LS_SIZE);
}