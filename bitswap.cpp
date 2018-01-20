#define FASTLED_INTERNAL
#include "FastLED.h"

/// Simplified form of bits rotating function.  Based on code found here - http://www.hackersdelight.org/hdcodetxt/transpose8.c.txt - rotating
/// data into LSB for a faster write (the code using this data can happily walk the array backwards)
void transpose8x1_noinline(unsigned char *A, unsigned char *B) {
  uint32_t x, y, t;

  // Load the array and pack it into x and y.
  y = *(unsigned int*)(A);
  x = *(unsigned int*)(A+4);

  // pre-transform x
  t = (x ^ (x >> 7)) & 0x00AA00AA;  x = x ^ t ^ (t << 7);
  t = (x ^ (x >>14)) & 0x0000CCCC;  x = x ^ t ^ (t <<14);

  // pre-transform y
  t = (y ^ (y >> 7)) & 0x00AA00AA;  y = y ^ t ^ (t << 7);
  t = (y ^ (y >>14)) & 0x0000CCCC;  y = y ^ t ^ (t <<14);

  // final transform
  t = (x & 0xF0F0F0F0) | ((y >> 4) & 0x0F0F0F0F);
  y = ((x << 4) & 0xF0F0F0F0) | (y & 0x0F0F0F0F);
  x = t;

  *((uint32_t*)B) = y;
  *((uint32_t*)(B+4)) = x;
}


void transpose16x1_noinline(unsigned char *A, uint16_t *B) {
    uint32_t  x, y, x1,y1,t;
    
    // Load the array and pack it into x and y.
    /*  y = (*(unsigned char*)(A) & 0xffff ) |  ((*(unsigned char*)(A+4) & 0xffffL )<<16) ;
     x = (*(unsigned char*)(A+8)& 0xffff ) |  ((*(unsigned char*)(A+12) & 0xffffL )<<16) ;
     y1 = (*(unsigned char*)(A+2)& 0xffff ) |  ((*(unsigned char*)(A+6) & 0xffffL )<<16);
     x1 = (*(unsigned char*)(A+10)& 0xffff )| ((*(unsigned char*)(A+14) & 0xffffL )<<16);*/
    //printf("%d\n",*(unsigned int*)(A+4));
    
    
    y = *(unsigned int*)(A);
    x = *(unsigned int*)(A+4);
    y1 = *(unsigned int*)(A+8);
    x1 = *(unsigned int*)(A+12);
    
    
    
    
    // pre-transform x
    t = (x ^ (x >> 7)) & 0x00AA00AA;  x = x ^ t ^ (t << 7);
    t = (x ^ (x >>14)) & 0x0000CCCC;  x = x ^ t ^ (t <<14);
    t = (x1 ^ (x1 >> 7)) & 0x00AA00AA;  x1 = x1 ^ t ^ (t << 7);
    t = (x1 ^ (x1 >>14)) & 0x0000CCCC;  x1 = x1 ^ t ^ (t <<14);
    // pre-transform y
    t = (y ^ (y >> 7)) & 0x00AA00AA;  y = y ^ t ^ (t << 7);
    t = (y ^ (y >>14)) & 0x0000CCCC;  y = y ^ t ^ (t <<14);
    t = (y1 ^ (y1 >> 7)) & 0x00AA00AA;  y1 = y1 ^ t ^ (t << 7);
    t = (y1 ^ (y1 >>14)) & 0x0000CCCC;  y1 = y1 ^ t ^ (t <<14);
    
    
    // final transform
    t = (x & 0xF0F0F0F0) | ((y >> 4) & 0x0F0F0F0F);
    y = ((x << 4) & 0xF0F0F0F0) | (y & 0x0F0F0F0F);
    x = t;
    
    t = (x1 & 0xF0F0F0F0) | ((y1 >> 4) & 0x0F0F0F0F);
    y1 = ((x1 << 4) & 0xF0F0F0F0) | (y1 & 0x0F0F0F0F);
    x1 = t;
    
    
    
    *((uint16_t*)B) = (uint16_t)((y & 0xff) |  (  (y1 & 0xff) << 8 ) )   ;
    *((uint16_t*)(B+1)) = (uint16_t)(((y & 0xff00) |((y1&0xff00) <<8))>>8);
    *((uint16_t*)(B+2)) = (uint16_t)(((y & 0xff0000) |((y1&0xff0000) <<8))>>16);
    *((uint16_t*)(B+3)) = (uint16_t)(((y & 0xff000000) >>8 |((y1&0xff000000) ))>>16);
    
    *((uint16_t*)B+4) =(uint16_t)( (x & 0xff) |((x1&0xff) <<8));
    *((uint16_t*)(B+5)) = (uint16_t)(((x & 0xff00) |((x1&0xff00) <<8))>>8);
    *((uint16_t*)(B+6)) = (uint16_t)(((x & 0xff0000) |((x1&0xff0000) <<8))>>16);
    *((uint16_t*)(B+7)) = (uint16_t)(((x & 0xff000000) >>8 |((x1&0xff000000) ))>>16);
    
}

uint32_t fixbit(uint32_t bit,uint32_t MASK,uint32_t  LANES )
{
    uint32_t res=0;
    char cnt=0;
    uint32_t d=1;
    //const  uint32_t l=1<<LANES;
    // Serial.printf("bit %d\n",bit);
   /* for(int i=0;i<32;i++)
    {
        if(MASK & (1<<i) )
        {
            //Serial.printf("ic %d\n",i);
            //Serial.printf("bits %d\n",bit & (1<<cnt));
            //Serial.printf("bitxs %d\n",(bit & (1<<cnt))<<(i-cnt));
           res = res  | ( (bit & d ) << (i-cnt));
            cnt++;
            d=d<<1;
           if (d==LANES)
                return res ;
        }*/
    for (int i=0;i< LANES;i++)
    {
        if(bit & (1<<i)) //on a une 1 a ecrire
        {
            res= res |  ((MASK&(MASK-1) ) ^  MASK) ;
            
        }
        MASK=MASK&(MASK-1);
    }
     //Serial.printf("bit %#010x:\n",bit);
    //Serial.printf("resultat %#010x:\n",res);
    
    return res;
}


uint32_t createMask(char *tab,int size)
{
    if((!tab)  |  ( size <=0))
        return  0;
    uint32_t msk=0;
    //printf("taille totale %luu\n",sizeof(*tab));
    for (int i=0;i<size;i++)
    {
        msk=msk+(1<<tab[i]);
        //printf("%d\n",i);
    }
    return msk;
}
