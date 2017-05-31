#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    GOZDE DOGAN                                                                                                                                                                                          //
//     131044019                                                                                                                                                                                                 //
//                                                                                                                                                                                                                           //
//    MICROPROCCESSOR BIL334 PROJE                                                                                                                                                 //
//                                                                                                                                                                                                                           //
////////////////////////////////////////////////////////////////////////////// DOGRU YANLIS SORU CEVAP OYUNU ///////////////////////////////////////////////////////////////////////                                                                                                                                                                                                                           //
//    Description:                                                                                                                                                                                                 //
//                           LCD ye 9 soru gelecek. Bu sorular�n cevaplanmas� beklenecek.                                                                             //
//                           Verilen cevaplara gore oyunun kazan�l�p kazan�lmad��� yine LCD de oyuncuya bildirilecek                                //
//                           LCD kullan�larak ekrana sorular yazd�r�ld�.                                                                                                                     //
//                           Cevaplar Dip Switch kullan�l�rak (PORTH dan) al�nd�. (1 > TRUE, 2 > FALSE)                                                       //
//                           Cevaplar�n do�ru ve yanl�� olmas�na g�re buzzer �tt�r�ld�.                                                                                      //
//                           Buzzer output compare kullan�larak �tt�r�ld�.                                                                                                              //
//                           Oyun bitince PORTB nin de yard�m�yla 7-Segment te do�ru ve yanl�� cevap sayisi g�sterildi.                           //
//                           Interruptla birlikte ger�ekle�tirilen serial communication ile soru say�s� elde tutuldu.                                         //
//                           Son olarak yine LCD de oyuncuya kazan�p kazanmad��� bildirildi.                                                                           //
//                           Ve oyun bitti!                                                                                                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//funtion prototype
void puts2lcd(char *ptr);
void put2lcd(char c, char type);
void delay_50us(int n);  //for lcd
void delay_1ms(int n);    // for lcd
void delay_1s(int n);  //for timer over flow
void openlcd(void);
void printLCD(char *q, int size);    // sorunun LCDye getirildigi kisim
void wrongAnswer(void);  //Yanlis cevap verildiginde LCD ye yazar
void trueAnswer(void);       // dogru cevap verildiginde LCD ye yazar
void print7Segment(void);    // 7-segment te sonuclari yazar
void SC0_INIT(void);              // soru sayisini serial communication ile tutar
interrupt (((0x10000-Vsci0)/2)-1) void SCI0_ISR(void);     //Serial communication islemini interrup ile yapar



//global variables
unsigned int trueAns = 0;
unsigned int falseAns = 0;
unsigned int index = 0;
unsigned int value = 0;

// LCD i�in tan�mlanan de�i�kenler
#define LCD_DAT PORTK
#define LCD_DIR DDRK
#define LCD_E 0x02
#define LCD_RS 0x01
#define CMD 0
#define DATA 1 

// sorular�n oldu�u 2D array
char *questions[40] = { "AGRI DAGI ERZURUMDADIR",      //38  -F
                                      "2X2'NIN SONUCU 8 EDER",                                       //22  -F
                                      "MIKRO KELIMESI 5 HARFTEN OLUSUR",               //32  -T
                                      "TBMM 1820 DE KURULMUSTUR",                            //25   -F
                                      "ATATURK ILK CUMHURBASKANIDIR",                    //29    -T
                                      "DONUSUM YAZARI FRANZ KAFKA'DIR",                  //31   -T
                                      "4-2'NIN SONUCU 2 EDER",                                        //22  -T
                                      "LINKEDLIST BIR VERI YAPISIDIR",                            //30 -T
                                     "QUICK SORT MERGE SORT'TAN DAHA HIZLIDIR" };  //39 -T                                      
                                      

// LCD de g�sterebilmek i�in her sorunun size'�n�n tutuldu�u array
int sizes[9] = {23, 22, 32, 25, 29, 31, 22, 30, 39};

// ekrana yaz�lacak di�er de�i�kenler
char *SORRY = "SORRY!! YOU LOST!!!";
char *CONGR = "CONGRATULATIONS!! YOU WIN!!!";

char *WRONG = "WRONG ANSWER!";
char *CORRECT = "CORRECT ANSWER!";


void main(void)
{               
      //int i;
     DDRH=0x00;
     
     TSCR1=0x80;    //output compare, timer over flow
     TSCR2=0x00;    //interrupt
   
     ///output compare for buzzer
     TIOS= TIOS | 0x20;

     TIE= 0x20;  //interrupt enable 5. kanal = buzzer
     TFLG1 = 0x20;      //output compare icin 5. kanal ayarlandi, buzzer otecek
     
      DDRB = 0xFF;
    
     //SC0_INIT(); //serial comminication ayarlari yapildi, interrupt burada kullanilacak
    // index = 0;
                                                        
     __asm(CLI);
    // QUESTIONS IN LCD
    for(; ;){
      if(index < 9){   
      // ask question
        printLCD(questions[index], sizes[index]);     // soru ekrana yaz�ld�
        
         PORTB = index;              // soru say�s� PORTB de g�sterildi
        
        delay_1s(5);  // 5saniye PORTH a deger girilmesi bekleniyor
         switch(index){    //Sorular�n cevaplar�na g�re gerekli i�lemler yap�ld�.
            case 0:   //F
            case 1:  //F
            case 3:  //F
            
               if(PTH == 1)           //1>> T            // Sorunun do�ru cevab� yanl��sa ve user T girdiyse
              {
                 TCTL1 =0x04;   ///buzzer open           // buzzer �ter
                 TC5 = TCNT+ (word)2000;
                 delay_1s(3);                                      // 3saniye
                 TCTL1 = 0x00; //buzzer close
                 
                  falseAns++;                   // yanl�� verilen cevap say�s� artar
                  wrongAnswer();             // LCD de yanl�� cevap verdi�i yaz�l�r
              }
              else if(PTH == 2)      //2>> F         // Sorunun do�ru cevab� do�ruysa ve user F girdiyse
              {       
                 TCTL1 =0x04;   ///buzzer open         // buzzer �ter
                 TC5 = TCNT+ (word)9600;
                 delay_1s(3);                                      // 3 saniye 
                 TCTL1 = 0x00; //buzzer close          // 3 saniye sonra buzzer kapat�l�r.
                 
                  trueAns++;
                  trueAnswer();
              }
              break;
              
            case 2: //T
            case 4:  //T
            case 5:  //T
            case 6:  //T
            case 7:
            case 8: //T
              if(PTH == 1)           //1>> T
              {         
                 TCTL1 =0x04;   ///buzzer open
                 TC5 = TCNT+ (word)9600;
                 delay_1s(3);
                 TCTL1 = 0x00; //buzzer close
                 
                 trueAns++; 
                 trueAnswer();            
              }
              else if(PTH == 2)      //2>> F
              {      
                TCTL1 =0x04;   ///buzzer open 
                TC5 = TCNT+ (word)2000;
                delay_1s(3);
                TCTL1 = 0x00; //buzzer close
                
                falseAns++;
                wrongAnswer();
              }
              break;
          }    
        
        index++;   // index bir sonraki soru i�in artt�r�ld�
      }
      //dogru cevap verildiginde buzzer oter, diger soruya gecince buzzer clear edildi
     // TCTL1 =0x00;   //clear buzzer
        
      if(index >= 9) {      
          
          break;     // index soru say�s�na geldiyse bu i�lemler biter.  
      }
    }
     
     //dogru ve yanlis sayilari 7-segment te �nce so�ru say�s� sonra yanl�� say�s� �eklinde s�rayla gosterildi.
     print7Segment(); 
          
     // OYUNUN SONU
     if(trueAns >= falseAns){
      printLCD(CONGR, 29);   // Do�ru say�s� yanl�� say�s�ndan fazla veya e�it ise kazan�l�r ve bu lcd de g�sterilir.
     } else{
      printLCD(SORRY, 20);      // yanl�� say�s� do�ru say�s�ndan fazla ise kaybedilir ve bu lcd de g�sterilir.
     }
     
    
    __asm(swi);

	EnableInterrupts;

  for(;;) {
    _FEED_COP(); 
  }   
}


///////////////////////FUNCTIONSS////////////////////////////////
void printLCD(char *q, int size){     // gelen stringi lcd de g�sterir
    int i;
    openlcd();
    for(i=0; i<size; ++i){     
      puts2lcd(&q[i]);
      delay_1ms(200);
      put2lcd(0x01, CMD);
    }  
}
  
void wrongAnswer(void){     // "WRONG ANSWER" YAZISINI LCD YE YAZAR
    int k; 
    openlcd();
    for(k=0; k<14; ++k)
    {       
        //put2lcd(0x0C, CMD);
        puts2lcd(&WRONG[k]);
        delay_1ms(200);
        put2lcd(0x01, CMD);
     }
}


void trueAnswer(void){        // "CORRECT ANSWER" YAZISINI LCD YE YAZAR
  int k;
  openlcd();
  for(k=0; k<13; ++k)
    {       
        //put2lcd(0x0C, CMD);
        puts2lcd(&CORRECT[k]);
        delay_1ms(200);
        put2lcd(0x01, CMD);
     }
}


void print7Segment(void){       /// 7-segment te  do�ru yanl�� say�s� g�sterilir. 
     char *result = "Display correct answer and wrong aswer in 7-segment";  
     char *result1 = "CORRECT Answer";
     char *result2= "WRONG Answer"; 
     DDRB=0xFF; //PORTB output
     DDRP=0xFF;  //7-segment output
     PTP=0x0E;   //open seven segment 0. led  for true answer 
     
     
     printLCD(result, 52);
     
     printLCD(result1, 15);
     
     switch(trueAns){
      case 0:
        PORTB = 0x3F;   // do�ru say�s�na g�re 7-Segmente de�er yaz�r�ld�
        break;
      case 1:
        PORTB = 0x06;
        break;
      case 2:
        PORTB = 0x5B;
        break;
      case 3:
        PORTB = 0x4F;
        break;
      case 4:
        PORTB = 0x0E;
        break;
      case 5:
        PORTB = 0x6D;
        break;
      case 6:
        PORTB = 0x7D;
        break;
      case 7:
        PORTB = 0x07;
        break;
      case 8:
        PORTB = 0x7F;
        break;
      case 9:
        PORTB = 0x6F;
        break;
     }
      
     delay_1s(2);  // 2 saniye beklenildi
     
     PORTB = 0x00;     // ve PORTB 0 yap�ld� b�ylece 7-segment te hi�bir �ey g�z�kmedi.
      
     PTP=0x07; //open seven segment 3. led  for false answer
     
     printLCD(result2, 13);
     
     switch(falseAns){
      case 0:
        PORTB = 0x3F;
        break;
      case 1:
        PORTB = 0x06;
        break;
      case 2:
        PORTB = 0x5B;
        break;
      case 3:
        PORTB = 0x4F;
        break;
      case 4:
        PORTB = 0x0E;
        break;
      case 5:
        PORTB = 0x6D;
        break;
      case 6:
        PORTB = 0x7D;
        break;
      case 7:
        PORTB = 0x07;
        break;
      case 8:
        PORTB = 0x7F;
        break;
      case 9:
        PORTB = 0x6F;
        break;
     }
     delay_1s(2); 
     
     PORTB = 0x00;
     PTP = 0x00; //7-segment yani PORTP clear edildi
}

void SC0_INIT(void){
  SCI0BDH = 0x00; // baud rate
  SCI0BDL = 0x00; // 0
  SCI0CR1 = 0x00;  //8-bit, no partiy
  SCI0CR2 = 0x8C; //interrupt,  transmition and receive 
}

/*interrupt (((0x10000-Vsci0)/2)-1) void SCI0_ISR(void){
    if(0x80 & SCI0SR1)
      SCI0DRL = index+1;
      
    if(0x20 & SCI0SR1)
      value = SCI0DRL;   
} */


//DELAY for LCD
void delay_50us(int n) {              //microsaniye
     unsigned int i=0, j=0;
     for(i=0; i<n ; ++i)
        for(j=0; j<20; ++j);
}


void delay_1ms(int n) {                //milisaniye
     unsigned int i=0, j=0;
     for(i=0; i<n ; ++i)
        for(j=0; j<4000; ++j);
}

// output compare with timer over flow
void delay_1s(int n){
  unsigned int overflow=0;
  while(overflow<n*366){
      TFLG2=0x80;
      while(!(TFLG2 & 0x80));
      overflow = overflow +1;
   }
}


//////////////////////// LCD FUNCTIONS (slaytlardan al�nd�) //////////////
void openlcd(void){
  
       LCD_DIR=0xFF;                      
       delay_1ms(100);
       put2lcd(0x28,CMD);
       put2lcd(0x0F,CMD);
       put2lcd(0X06,CMD);
       put2lcd(0x01,CMD);
             
}


void puts2lcd(char *ptr)               //stringi almasi icin 
{
  while(*ptr)
  {
       put2lcd(*ptr, DATA);
       delay_50us(1);
       ++ptr; 
  } 
}


void put2lcd(char c, char type)      //istenilen karakterlerde string 2 parametreli
{
      char chi_1, chi_2;
      chi_1= c & 0x0F;
      chi_2= c & 0xF0;
      
      chi_1= chi_1<<2;       //shift
      chi_2= chi_2>>2;
      
       LCD_DAT &= (~LCD_RS);
      
      
      if(type== DATA)
          LCD_DAT = chi_2 | LCD_RS;        //high
      else
          LCD_DAT = chi_2;
      
      LCD_DAT |= LCD_E;
      
      __asm(nop);
      __asm(nop);
      __asm(nop);
      
      
      
      LCD_DAT &= (~LCD_E);
      
         LCD_DAT &= (~LCD_RS);
      
      if(type==DATA)                         //low
        LCD_DAT = chi_1 | LCD_RS;
      else
        LCD_DAT = chi_1;
      
      
      LCD_DAT |= LCD_E;
      
      __asm(nop);
      __asm(nop);
      __asm(nop);
      
      
      LCD_DAT &= (~LCD_E);
      delay_50us(10);
      delay_1ms(2);
      
}
