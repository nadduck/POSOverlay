#ifndef PixelFEDInterface_h
#define PixelFEDInterface_h

#include <bitset>
#include <map>
#include <vector>
    
#include "CalibFormats/SiPixelObjects/interface/PixelPh1FEDCard.h"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

class PixelFEDInterface {
 public:
  struct FitelRegItem {
    uint8_t fAddress;
    uint8_t fDefValue;
    uint8_t fValue;
    char fPermission;
  };

  typedef std::map < std::string, FitelRegItem > FitelRegMap;

  typedef std::bitset<48> enbable_t;

  PixelFEDInterface(Ph2_HwInterface::RegManager*);
  ~PixelFEDInterface();

  void set_Printlevel(int level) { Printlevel = level; }
  int get_Printlevel() const { return Printlevel; }

  pos::PixelPh1FEDCard& getPixelFEDCard() { return pixelFEDCard; }
  void setPixelFEDCard(const pos::PixelPh1FEDCard& aPixelFEDCard) { pixelFEDCard = aPixelFEDCard; }

  void set_fitel_fn_base(const std::string& b) { fitel_fn_base = b; }

  int setup(const std::string& fileName); 
  int setup(pos::PixelPh1FEDCard& pfc); 
  int setupFromDB(pos::PixelPh1FEDCard& pfc) { return setup(pfc); }
  int setup();  // run the setup 

  std::string getBoardType();
  void getFEDNetworkParameters();
  void getBoardInfo();
  void disableFMCs();
  void enableFMCs();
  void FlashProm( const std::string & strConfig, const char* pstrFile );
  void JumpToFpgaConfig( const std::string & strConfig );
  std::vector<std::string> getFpgaConfigList();
  void DeleteFpgaConfig( const std::string & strId );
  void DownloadFpgaConfig( const std::string& strConfig, const std::string& strDest);
  void checkIfUploading();
  void EncodeFitelReg( const FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId , std::vector<uint32_t>& pVecReq );
  void DecodeFitelReg( FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId, uint32_t pWord );
  void i2cRelease(uint32_t pTries);
  bool polli2cAcknowledge(uint32_t pTries);
  bool WriteFitelBlockReg(std::vector<uint32_t>& pVecReq);
  bool ReadFitelBlockReg(std::vector<uint32_t>& pVecReq);
  std::pair<bool, std::vector<double> > ReadADC( const uint8_t pFMCId, const uint8_t pFitelId);

  void loadFPGA(); // (re)Loads the FPGA with the program in the EEPROM
  void reset(); // resets everything
  void resetFED(); // reset FED (LRES + CLRES + fake event,center OOS counters + error fifos)

  // arm OSD readback from roc; wait 31 triggers and call read
  void armOSDFifo(int channel, int rochi, int roclo);
  uint32_t readOSDFifo(int channel);

  void readPhases(bool verbose, bool override_timeout);

  std::vector<uint32_t> readTransparentFIFO();
  int drainTransparentFifo(uint32_t* data);
  std::vector<uint32_t> readSpyFIFO();
  int drainSpyFifo(uint32_t* data);
  void readFIFO1();
  int drainFifo1(uint32_t* data);
  int drainTBMFifo(uint32_t* data);
  int drainErrorFifo(uint32_t* data);
  int drainTemperatureFifo(uint32_t* data);
  int drainTTSFifo(uint32_t *data);
  void SelectDaqDDR( uint32_t pNthAcq );
  std::vector<uint32_t> ReadData(uint32_t pBlockSize );
  int spySlink64(uint64_t* data);

  bool isWholeEvent(uint32_t nTries=100000);
  bool isNewEvent(uint32_t nTries=100000);
  int enableSpyMemory(const int enable);

  uint32_t get_VMEFirmwareDate();
  uint32_t get_FirmwareDate(int);
  
  bool loadFedIDRegister();
  bool setFedIDRegister(uint32_t value);
  uint32_t getFedIDRegister();

  bool loadControlRegister();
  bool setControlRegister(uint32_t value);
  uint32_t getControlRegister();

  bool loadModeRegister();
  bool setModeRegister(uint32_t value);
  uint32_t getModeRegister();

  void set_PrivateWord(uint32_t pword);

  void resetSlink();

  // Check for channels that don't match FEDCard
  // Passes bitsets which indicate which channel(s) didn't match
  // If something doesn't match, it usually indicates an SEU
  bool checkFEDChannelSEU();
  void incrementSEUCountersFromEnbableBits(std::vector<int>&, enbable_t, enbable_t);
  void resetEnbableBits();
  // Check to see if any channel as too many SEUs
  bool checkSEUCounters(int);
  void storeEnbableBits();  
  void resetSEUCountAndDegradeState(void);
  bool runDegraded(void) {return runDegraded_;}

  void testTTSbits(uint32_t data, int enable);

  void setXY(int X, int Y);
  int getXYCount();
  void resetXYCount();

  int getNumFakeEvents();
  void resetNumFakeEvents();

  uint32_t readEventCounter();
  uint32_t getFifoStatus();
  uint32_t linkFullFlag();
  uint32_t numPLLLocks();
  uint32_t getFifoFillLevel();
  uint64_t getSkippedChannels();
  uint32_t getErrorReport(int ch);
  uint32_t getTimeoutReport(int ch);

  int TTCRX_I2C_REG_READ(int Register_Nr);
  int TTCRX_I2C_REG_WRITE(int Register_Nr, int Value);

  void setBlockSize(uint32_t s) { fBlockSize = s; }
  uint32_t getBlockSize() const { return fBlockSize; }

 private:
  int Printlevel; //0=critical only, 1=all error,2& =info, 4&param file info
  pos::PixelPh1FEDCard pixelFEDCard;

  typedef uhal::ValWord<uint32_t> valword;
  typedef uhal::ValVector<uint32_t> valvec;
  Ph2_HwInterface::RegManager* const regManager;

  enum { FMC0_Fitel0, FMC0_Fitel1, FMC1_Fitel0, FMC1_Fitel1, nFitels};
  FitelRegMap fRegMap[nFitels];
  int FitelMapNum( int cFMCId, int cFitelId ) const { return 2*cFMCId + cFitelId; }
  std::string fitel_fn_base;
  std::string fRegMapFilename[nFitels];
  void LoadFitelRegMap( int cFMCId, int cFitelId );
  void ConfigureFitel(int cFMCId, int cFitelId , bool pVerifLoop );

  unsigned long long fNthAcq; // for keeping track of reading from ddr0/1
  uint32_t fBlockSize; // sigh
  std::string fStrDDR;
  std::string fStrDDRControl;
  std::string fStrFull;
  std::string fStrReadout;

  // Keep track of the expected status of the FED channels
  // and the status of the FED channels last time we checked
  enbable_t enbable_expected, enbable_last;

  // Keep track of the number of SEUs in each channel
  std::vector<int> num_SEU;

  // keep track of degraded state
  bool runDegraded_;
};

#endif
