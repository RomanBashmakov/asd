/* ----------------------------------------------------------------------------
 *            HOLT INTEGRATED CIRCUITS Applications Engineering
 * ----------------------------------------------------------------------------
  File:   device_3220.h

 * Copyright (c) 2011, Holt Integrated Circuits
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY HOLT "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL HOLT BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Opcodes

// READ Fast Opcode read macros - See ERRATA notes
#define MCR_WR_F        0x00  // Ony one that can write 
#define MSR_RD_F        0x08        
#define PIR_F           0x10
#define RPIRH_F         0x18
#define RPIRL_F         0x20
#define PIRPIRHL_F      0x28
#define RPIRHL _F       0x2C
#define RPIRL2_F        0x30
#define FTFH_F          0x38 
#define FTFL_F          0x34 
#define TFFR_F          0x3C    // transmit flags reg


/* Read/Write Register OpCode Macros.
  Use with Write Register 11011AAA - AAAA0000 
  Use with Read  Register 1110AAAA - AAAA0000
  Use with function: ReadRegister(_xx, 0)
-----------------------------------------------
*/ 
#define _MCRW           0x00    // R/W 
#define _MCR            0x01
#define _MSR            0x02
#define NotUsed3        0x03
#define _PIR            0x04
#define NotUsed5        0x05
#define _RPIRL          0x06
#define NotUsed7        0x07
#define _RPIRH          0x08
#define NotUsed9        0x09
#define __PIR           0x0A
#define __RPIRL         0x0B
#define __RPIRH         0x0C
#define _FTFL           0x0D
#define _FTFH           0x0E
#define _TFR            0x0F
#define _IAR0           0x10
#define _IAR1           0x11
#define _IAR2           0x12
#define _IAR3           0x13
#define _IAR4           0x14
#define _IAR5           0x15
#define _IAR6           0x16
#define _IAR7           0x17
#define _IAR8           0x18
#define _IAR9           0x19
#define _IAR10          0x1A
#define _IAR11          0x1B
#define _IAR12          0x1C
#define _IAR13          0x1D
#define _IAR14          0x1E
#define _IAR15          0x1F




#define _BISTFH         0x7A
   
#define _ALOOP          0x49
   

// -----------------------------------------------



#define _ATXC0           0x30
#define _ATXC1           0x31
#define _ATXC2           0x32
#define _ATXC3           0x33
#define _ATXC4           0x34
#define _ATXC5           0x35
#define _ATXC6           0x36
#define _ATXC7           0x37   

#define _ARXC0           0x20
#define _ARXC1           0x21
#define _ARXC2           0x22
#define _ARXC3           0x23
#define _ARXC4           0x24
#define _ARXC5           0x25
#define _ARXC6           0x26
#define _ARXC7           0x27 
#define _ARXC8           0x28
#define _ARXC9           0x29
#define _ARXC10          0x2A
#define _ARXC11          0x2B
#define _ARXC12          0x2C
#define _ARXC13          0x2D
#define _ARXC14          0x2E
#define _ARXC15          0x2F 
   

#define FTFL            0x800D
#define FTFH            0x800E

#if 0
#define PIR             0x800A  // Main Pending interrupt reg
#define RPIRL           0x800B //6
#define RPIRH           0x800C //8
#else // alternates
#define PIR           0x8004 //  0x800A  // Main Pending interrupt reg
#define RPIRL        0x8006 //   0x800B //6
#define RPIRH        0x8008  //  0x800C //8   
   
#endif
 



#define RD_MEMORY_MAP   0x80  // 
#define WR_MEMORY_MAP   0x88  // 
#define RD_MAP          0x90  // 
#define WR_MAP          0x98  // 

#define WR_REGISTER     0XD800  // Writes 0x8000-0x807F
#define RD_REGISTER     0XE000  // Reads  0x8000-0x807F

#define SW_RESET_MODE0        0xFA58  // 3220 software reset opcode
#define SW_RESET_MODE1        0xFA59  // 3220 software reset opcode
#define SW_RESET_MODE2        0xFA5A  // 3220 software reset opcode
#define SW_RESET_MODE3        0xFA5B  // 3220 software reset opcode
#define SW_RESET_MODE4        0xFA5C  // 3220 software reset opcode
#define SW_RESET_MODE5        0xFA5D  // 3220 software reset opcode
#define SW_RESET_MODE6        0xFA5E  // 3220 software reset opcode - Allows RAM BIST access 
 

#define TX0_TTT         0xA0  // TX immediat 0xA0 opcode, channels TX0-TX7 will reference from this
#define FIFO_RD         0xC0  // 3220 Read ARINC FIFO RX0-RX7 will reference from here
#define RD_BLK          0xC8  // Read ARINC blocks RX0-RX7 will reference from here
#define RD_MESG         0xD0  // Read ARINC message RX0-RX7 will reference from here

// Master Control reg macros bit fields
#define A429RX          0x80
#define A429TX          0x40
#define AFLIP           0x08
#define TXMSK           0x04
#define TX7OPT          0x02
#define TXDL256         0x01


// Transmit Control Reg bits
#define RUN             0x80
#define PRESCALE        0X40    // 0=10MS, 1=1MS
#define PARITY_OFF      0x00
#define EVEN_P          0x10
#define ODD_P           0x00
#define SKIP            0x08
#define TRISTATE        0x04

// common control reg bits  
#define SP_50           0x01    // 1=50K SPEED (OVER RIDES RATE SETTING)
#define SP_HI           0x00   
#define SP_LO           0x02
   
// Receive Control Reg bits
#define ENABLE          0x80
#define PARITYEN        0X20   
#define DECODER         0X10
#define SD10            0X08
#define SD9             0X04
   
#define SD10BIT         0X02    // used for setting transmit data
#define SD9BIT          0X01    // "
   


#define TFR0          0x01    // Transmit FIFO Threshold Flags 0X800f
#define TFR1          0x02
#define TFR2          0x04
#define TFR3          0x08
#define TFR4          0x10
#define TFR5          0x20
#define TFR           0x40
#define TFR7          0x80

// 3220 device memory map start addresses  
#define RXDATA          0x0000   
// convenience Receiver Block memory start addresses. See data sheet memory map page 10.
#define RXBLK0          0x0000
#define RXBLK1          0x0400
#define RXBLK2          0x0800
#define RXBLK3          0x0C00
#define RXBLK4          0x1000
#define RXBLK5          0x1400
#define RXBLK6          0x1800
#define RXBLK7          0x1C00
#define RXBLK8          0x2000
#define RXBLK9          0x2400
#define RXBLK10         0x2800
#define RXBLK11         0x2C00
#define RXBLK12         0x3000
#define RXBLK13         0x3400
#define RXBLK14         0x3800
#define RXBLK15         0x3C00 // last address 0x3FFF

// Transmit schedule table start addresses
#define ATXC0TABLE      0x4000
#define ATXC1TABLE      0x4800
#define ATXC2TABLE      0x5000
#define ATXC3TABLE      0x5400
#define ATXC4TABLE      0x5800
#define ATXC5TABLE      0x5C00
#define ATXC6TABLE      0x6000
#define ATXC7TABLE      0x6400


#define RXEN_MAP        0x6800
#define RXEN_MAP_END    0x69FF

#define RXINT_MAP       0x6A00
#define RXINT_ENDMAP    0x6C00

#define REGISTERS_START   0x8000
   
// Interrupot Vector regs
#define IAR0            0x8010
#define IAR1            0x8011
#define IAR2            0x8012
#define IAR3            0x8013
#define IAR4            0x8014
#define IAR5            0x8015
#define IAR6            0x8016
#define IAR7            0x8017
#define IAR8            0x8018
#define IAR9            0x8019
#define IAR10           0x801A
#define IAR11           0x801B
#define IAR12           0x801C
#define IAR13           0x801D
#define IAR14           0x801E
#define IAR15           0x801F

// Read Register macros- See ERRATA
#define _IAR0          0x10
#define _IAR1          0x11
#define _IAR2          0x12
#define _IAR3          0x13
#define _IAR4          0x14
#define _IAR5          0x15
#define _IAR6          0x16
#define _IAR7          0x17
#define _IAR8          0x18
#define _IAR9          0x19
#define _IAR10         0x1A
#define _IAR11         0x1B
#define _IAR12         0x1C
#define _IAR13         0x1D
#define _IAR14         0x1E
#define _IAR15         0x1F

// Fast IAR read opcode equivalents - See ERRATA
#define IAR0_F           0x40
#define IAR1_F           0x44
#define IAR2_F           0x48
#define IAR3_F           0x4C
#define IAR4_F           0x50
#define IAR5_F           0x54
#define IAR6_F           0x58
#define IAR7_F           0x5C
#define IAR8_F           0x60
#define IAR9_F           0x64
#define IAR10_F          0x68
#define IAR11_F          0x6C
#define IAR12_F          0x70
#define IAR13_F          0x74
#define IAR14_F          0x78
#define IAR15_F          0x7C


#define AMFF            0x800C
  

// Interrupt register macros bit fields
#define EEABORTERR      0x80
#define COPYERR         0x40
#define AUTOERR         0x20
#define CHKERR          0x10
#define RAMFAIL         0x08
#define RXRAMNEW        0x04
#define RXFLAG          0x02
#define TXFLAG          0x01


// Transmitter Interrupt regs
#define TFIE            0x804F  // Transmit FIFO INT Enable Register
#define TXIE0           1       // Transmit Flag Int Enables 0-7
#define TXIE1           2
#define TXIE2           4
#define TXIE3           8
#define TXIE4           0x10
#define TXIE5           0x20
#define TXIE6           0x40
#define TXIE7           0x80

#define TFTR0            0x8060 // Transmit FIFO Threshold regs, 0=default FIFO Empty.
#define TFTR1            0x8061
#define TFTR2            0x8062
#define TFTR3            0x8063
#define TFTR4            0x8064
#define TFTR5            0x8065
#define TFTR6            0x8066
#define TFTR7            0x8067
  
#define PIER            0X804A

#define RIERL           0x804B
#define RIERH           0x804C

#define RFIFOIEL         0x804D
#define RFIFOIEH         0x804E

#define ATRIE           0X8035
   
#define ARXC0           0x8020
#define ARXC1           0x8021
#define ARXC2           0x8022
#define ARXC3           0x8023
#define ARXC4           0x8024
#define ARXC5           0x8025
#define ARXC6           0x8026
#define ARXC7           0x8027 
#define ARXC8           0x8028
#define ARXC9           0x8029
#define ARXC10          0x802A
#define ARXC11          0x802B
#define ARXC12          0x802C
#define ARXC13          0x802D
#define ARXC14          0x802E
#define ARXC15          0x802F 

#define ARXC0_R           0x20
#define ARXC1_R           0x21
#define ARXC2_R           0x22
#define ARXC3_R           0x23
#define ARXC4_R           0x24
#define ARXC5_R           0x25
#define ARXC6_R           0x26
#define ARXC7_R           0x27
#define ARXC8_R           0x28
#define ARXC9_R           0x29
#define ARXC10_R          0x2A
#define ARXC11_R          0x2B
#define ARXC12_R          0x2C
#define ARXC13_R          0x2D
#define ARXC14_R          0x2E
#define ARXC15_R          0x2F


#define ATXC0           0x8030
#define ATXC1           0x8031
#define ATXC2           0x8032
#define ATXC3           0x8033
#define ATXC4           0x8034
#define ATXC5           0x8035
#define ATXC6           0x8036
#define ATXC7           0x8037

#define ATXC0_R          0x30   // ReadRegister use
#define ATXC1_R          0x31   // ReadRegister use
#define ATXC2_R          0x32   // ReadRegister use
#define ATXC3_R          0x33   // ReadRegister use
#define ATXC4_R          0x34   // ReadRegister use
#define ATXC5_R          0x35   // ReadRegister use
#define ATXC6_R          0x36   // ReadRegister use
#define ATXC7_R          0x37   // ReadRegister use

#define RRR0            0x8038
#define RRR1            0x8039
#define RRR2            0x803A
#define RRR3            0x803B
#define RRR4            0x803C
#define RRR5            0x803D
#define RRR6            0x803E
#define RRR7            0x803F

#define RRR0_R            0x38
#define RRR1_R            0x39
#define RRR2_R            0x3A
#define RRR3_R            0x3B
#define RRR4_R            0x3C
#define RRR5_R            0x3D
#define RRR6_R            0x3E
#define RRR7_R            0x3F

#define TXSP0           0x8040

// Receive FIFO threshold value addresses
#define FTV0            0x8050  
#define FTV1            0x8051
#define FTV2            0x8052
#define FTV3            0x8053
#define FTV4            0x8054
#define FTV5            0x8055
#define FTV6            0x8056
#define FTV7            0x8057
#define FTV8            0x8058
#define FTV9            0x8059
#define FTV10           0x805A
#define FTV11           0x805B
#define FTV12           0x805C
#define FTV13           0x805D
#define FTV14           0x805E
#define FTV15           0x805F

// Receive FIFO current message count value addresses
// Reads = Receive FIFO message count (0-63)
// Write 0xA5 clears the register
#define FCV0R           0x68    // demonstrate faster Read Register opcode
#define FCV0            0x8068  
#define FCV1            0x8069
#define FCV2            0x806A
#define FCV3            0x806B
#define FCV4            0x806C
#define FCV5            0x806D
#define FCV6            0x806E
#define FCV7            0x806F
#define FCV8            0x8070
#define FCV9            0x8071
#define FCV10           0x8072
#define FCV11           0x8073
#define FCV12           0x8074
#define FCV13           0x8075
#define FCV14           0x8076
#define FCV15           0x8077


#define ALOOP          0x8049 
#define ALOOP_R          0x49 

#define TXSPO          0X8040
#define TXSP1          0X8041
#define TXSP2          0X8042
#define TXSP3          0X8043
#define TXSP4          0X8044
#define TXSP5          0X8045
#define TXSP6          0X8046
#define TXSP7          0X8047
    
#define ARXBIT          0x805F  
   
#define BISTS           0x8078  
#define BISTSREG        0x78
#define BISTFL          0x8079  
#define BISTFH          0x807A  
#define AIFL            0x807B  
#define AIFH            0x807C  
   
// Convenience macros to specify TX (0-7) or RX (0-15) channel numbers
#define CH0     0
#define CH1     1
#define CH2     2
#define CH3     3
#define CH4     4
#define CH5     5
#define CH6     6
#define CH7     7
#define CH8     8
#define CH9     9
#define CH10    10
#define CH11    11
#define CH12    12
#define CH13    13
#define CH14    14
#define CH15    15

#define INT0    1
#define INT1    2
#define INT2    4
#define INT3    8
#define INT4    0x10
#define INT5    0x20
#define INT6    0x40
#define INT7    0x80
#define INT8    0x0100
#define INT9    0x0200
#define INT10   0x0400
#define INT11   0x0800
#define INT12   0x1000
#define INT13   0x2000
#define INT14   0x4000
#define INT15   0x8000



#define LOOPBACK_DEMO 1
#define NORMAL_DEMO 0

// A429 Transmit Scheduler Opcode macros
#define EOS 0        	        // End of sequence (only valid with position 0, Action byte 1)
#define DLY (1 << 4)        	// Delay
#define IM  (2 << 4)        	// Immediate
#define IMC (3 << 4)	        // Immedidate Conditional
#define ID  (4 << 4)	        // Indexed
#define IDC (5 << 4)	        // Indexed Conditional
#define IDS (6 << 4)	        // Indexed SDI
#define IDSC (7 << 4)	        // Indexed Conditional SDI

#define NOP 0

// TWDT
#define WMR 0x807E              // Watchdog Timer Mask register 8-bit
#define WTR 0x807F              // Watchdog Timer reset (value )register 8-bit

// TX FIFO Count registers (count 0-31)
#define TXFCV0            0x8080  
#define TXFCV1            0x8081
#define TXFCV2            0x8082
#define TXFCV3            0x8083
#define TXFCV4            0x8084
#define TXFCV5            0x8085
#define TXFCV6            0x8086
#define TXFCV7            0x8087


   
   
   
   
   
   
