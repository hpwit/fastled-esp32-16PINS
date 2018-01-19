#ifndef __INC_CLOCKLESS_BLOCK_ESP8266_H
#define __INC_CLOCKLESS_BLOCK_ESP8266_H

#define FASTLED_HAS_BLOCKLESS 1

#define PORT_MASK (((1<<USED_LANES_FIRST)-1) & 0x0000FFFFL) //on dit  que l'ion va en faire 10
#define FIX_BITS(bits)  (   ((bits & 0xE0L) << 16) |  ((bits & 0x1EL)<<1)  | (bits & 0x1L)   )
#define  PORT_MASK3 (((1<<USED_LANES_SECOND )-1) & 0x0000FFFFL)
#define PORT_MASK2 FIX_BITS(PORT_MASK3)
#define MIN(X,Y) (((X)<(Y)) ? (X):(Y))
//attempted to use other/more pins for the port
// #define USED_LANES (MIN(LANES,33))
#define USED_LANES (MIN(LANES,16))
#define USED_LANES_FIRST ( (LANES)>8 ? (8) :(LANES) )
#define USED_LANES_SECOND ( (LANES)>8 ? (LANES-8) :(0) )
#define REAL_FIRST_PIN 12
// #define LAST_PIN (12 + USED_LANES - 1)
#define LAST_PIN 35

FASTLED_NAMESPACE_BEGIN

#ifdef FASTLED_DEBUG_COUNT_FRAME_RETRIES
extern uint32_t _frame_cnt;
extern uint32_t _retry_cnt;
#endif

template <uint8_t LANES, int FIRST_PIN, int T1, int T2, int T3, EOrder RGB_ORDER = GRB, int XTRA0 = 0, bool FLIP = false, int WAIT_TIME = 5>
class InlineBlockClocklessController : public CPixelLEDController<RGB_ORDER, LANES, PORT_MASK> {
    typedef typename FastPin<FIRST_PIN>::port_ptr_t data_ptr_t;
    typedef typename FastPin<FIRST_PIN>::port_t data_t;

    data_t mPinMask;
    data_ptr_t mPort;
    CMinWait<WAIT_TIME> mWait;
public:
    virtual int size() { return CLEDController::size() * LANES; }

    virtual void showPixels(PixelController<RGB_ORDER, LANES, PORT_MASK> & pixels) {
	// mWait.wait();
	/*uint32_t clocks = */
	int cnt=FASTLED_INTERRUPT_RETRY_COUNT;
	while(!showRGBInternal(pixels) && cnt--) {
	    ets_intr_unlock();
#ifdef FASTLED_DEBUG_COUNT_FRAME_RETRIES
	    _retry_cnt++;
#endif
	    delayMicroseconds(WAIT_TIME * 10);
	    ets_intr_lock();
	}
	// #if FASTLED_ALLOW_INTTERUPTS == 0
	// Adjust the timer
	// long microsTaken = CLKS_TO_MICROS(clocks);
	// MS_COUNTER += (1 + (microsTaken / 1000));
	// #endif
	
	// mWait.mark();
    }

    template<int PIN> static void initPin() {
	if(PIN >= 0 && PIN <= LAST_PIN) {
	    _ESPPIN<PIN, 1<<(PIN & 0xFF)>::setOutput();
	    // FastPin<PIN>::setOutput();
	}
    }

    virtual void init() {
	// Only supportd on pins 12-15
        // SZG: This probably won't work (check pins definitions in fastpin_esp32)
        void (* funcs[])() ={initPin<12>,initPin<13>,initPin<14>,initPin<15>,initPin<16>,initPin<17>,initPin<18>,initPin<19>,initPin<0>,initPin<2>,initPin<3>,initPin<4>,initPin<5>,initPin<21>,initPin<22>,initPin<23>};
        
       // void (* funcs[])() ={initPin<12>, initPin<13>, initPin<14>, initPin<15>, initPin<4>, initPin<5>};
        
        for (uint8_t i = 0; i < USED_LANES; ++i) {
            funcs[i]();
        }
	mPinMask = FastPin<FIRST_PIN>::mask();
	mPort = FastPin<FIRST_PIN>::port();
	
	// Serial.print("Mask is "); Serial.println(PORT_MASK);
    }

    virtual uint16_t getMaxRefreshRate() const { return 400; }
    
    typedef union {
	uint8_t bytes[8];
	uint16_t shorts[4];
	uint32_t raw[2];
    } Lines;

#define ESP_ADJUST 0 // (2*(F_CPU/24000000))
#define ESP_ADJUST2 0
    template<int BITS,int PX> __attribute__ ((always_inline)) inline static void writeBits(register uint32_t & last_mark, register Lines & b,register Lines & b3, PixelController<RGB_ORDER, LANES, PORT_MASK> &pixels) { // , register uint32_t & b2)  {
	Lines b2 = b;
    Lines b4=b3;
	transpose8x1_noinline(b.bytes,b2.bytes);
    transpose8x1_noinline(b3.bytes,b4.bytes);
	
	register uint8_t d = pixels.template getd<PX>(pixels);
	register uint8_t scale = pixels.template getscale<PX>(pixels);
	
	for(register uint32_t i = 0; i < 8; i++) { //USED_LANES
	    while((__clock_cycles() - last_mark) < (T1+T2+T3));
	    last_mark = __clock_cycles();
        *FastPin<FIRST_PIN>::sport() = (PORT_MASK << REAL_FIRST_PIN ) | ( PORT_MASK2 );
	    
        uint32_t nword =   (  ((uint32_t)(~b2.bytes[7-i]) & PORT_MASK) << REAL_FIRST_PIN ) | (FIX_BITS(  (uint32_t)(~b4.bytes[7-i]) ) & PORT_MASK2) ; //((uint32_t)(~b4.bytes[7-i]) & PORT_MASK2) << 21 );//
	    while((__clock_cycles() - last_mark) < (T1-6));
	    *FastPin<FIRST_PIN>::cport() = nword;
	    
	    while((__clock_cycles() - last_mark) < (T1+T2));
        *FastPin<FIRST_PIN>::cport() = (PORT_MASK << REAL_FIRST_PIN ) | ( PORT_MASK2 );
	    
	  //  b.bytes[i] = pixels.template loadAndScale<PX>(pixels,i,d,scale);
	}
for(register uint32_t i = 0; i < USED_LANES_FIRST; i++) {
    b.bytes[i] = pixels.template loadAndScale<PX>(pixels,i,d,scale);
}
        for(int i = 0; i < USED_LANES_SECOND; i++) {
            b3.bytes[i] = pixels.template loadAndScale<PX>(pixels,i+8,d,scale);
        }
        
   
	/*for(register uint32_t i = USED_LANES; i < 8; i++) {
	    while((__clock_cycles() - last_mark) < (T1+T2+T3));
	    last_mark = __clock_cycles();
	    *FastPin<FIRST_PIN>::sport() = PORT_MASK << REAL_FIRST_PIN;
	    
	    uint32_t nword = ((uint32_t)(~b2.bytes[7-i]) & PORT_MASK) << REAL_FIRST_PIN;
	    while((__clock_cycles() - last_mark) < (T1-6));
	    *FastPin<FIRST_PIN>::cport() = nword;
	    
	    while((__clock_cycles() - last_mark) < (T1+T2));
	    *FastPin<FIRST_PIN>::cport() = PORT_MASK << REAL_FIRST_PIN;
     
     }*/
	
        
        
        
    }

    // This method is made static to force making register Y available to use for data on AVR - if the method is non-static, then
    // gcc will use register Y for the this pointer.
    static uint32_t showRGBInternal(PixelController<RGB_ORDER, LANES, PORT_MASK> &allpixels) {
	
	// Setup the pixel controller and load/scale the first byte
	Lines b0,b1;
	
	for(int i = 0; i < USED_LANES_FIRST; i++) {
	    b0.bytes[i] = allpixels.loadAndScale0(i);
	}
    for(int i = 0; i < USED_LANES_SECOND; i++) {
            b1.bytes[i] = allpixels.loadAndScale0(i+8);
        }
        

    
	allpixels.preStepFirstByteDithering();
	
	ets_intr_lock();
	uint32_t _start = __clock_cycles();
	uint32_t last_mark = _start;
	
	while(allpixels.has(1)) {
	    // Write first byte, read next byte
	    writeBits<8+XTRA0,1>(last_mark, b0, b1,allpixels);
	    
	    // Write second byte, read 3rd byte
	    writeBits<8+XTRA0,2>(last_mark, b0, b1,allpixels);
	    allpixels.advanceData();
	    
	    // Write third byte
        writeBits<8+XTRA0,0>(last_mark, b0,b1, allpixels);
	    
#if (FASTLED_ALLOW_INTERRUPTS == 1)
	    ets_intr_unlock();
#endif
	    
	    allpixels.stepDithering();
	    
#if (FASTLED_ALLOW_INTERRUPTS == 1)
	    ets_intr_lock();
	    // if interrupts took longer than 45µs, punt on the current frame
	    if((int32_t)(__clock_cycles()-last_mark) > 0) {
		if((int32_t)(__clock_cycles()-last_mark) > (T1+T2+T3+((WAIT_TIME-INTERRUPT_THRESHOLD)*CLKS_PER_US))) { ets_intr_unlock(); return 0; }
	    }
#endif
	};
	
	ets_intr_unlock();
#ifdef FASTLED_DEBUG_COUNT_FRAME_RETRIES
	_frame_cnt++;
#endif
	return __clock_cycles() - _start;
    }
};

FASTLED_NAMESPACE_END
#endif
