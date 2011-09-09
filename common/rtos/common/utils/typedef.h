//
#ifndef TYPEDEFS

#define TYPEDEFS

#ifndef true
   #define true	        -1
#endif
#ifndef false
   #define false	0
#endif

   #define null 	((void*)0)

  typedef unsigned char  uint8;
  typedef unsigned short uint16;
  typedef unsigned int   uint32;
  typedef signed short   int16;
  typedef signed int     int32;

  #ifndef bool
  typedef signed int            bool;

  #endif
  typedef unsigned char DEVICE_ADDRESS_TYPE ;
  union WORDBYTE
  {
    uint16 word;
    struct
    {
      unsigned lo:8;
      unsigned hi:8;
    };
  };
#endif





