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
//                           LCD ye 9 soru gelecek. Bu sorularýn cevaplanmasý beklenecek.                                                                             //
//                           Verilen cevaplara gore oyunun kazanýlýp kazanýlmadýðý yine LCD de oyuncuya bildirilecek                                //
//                           LCD kullanýlarak ekrana sorular yazdýrýldý.                                                                                                                     //
//                           Cevaplar Dip Switch kullanýlýrak (PORTH dan) alýndý. (1 > TRUE, 2 > FALSE)                                                       //
//                           Cevaplarýn doðru ve yanlýþ olmasýna göre buzzer öttürüldü.                                                                                      //
//                           Buzzer output compare kullanýlarak öttürüldü.                                                                                                              //
//                           Oyun bitince PORTB nin de yardýmýyla 7-Segment te doðru ve yanlýþ cevap sayisi gösterildi.                           //
//                           Interruptla birlikte gerçekleþtirilen serial communication ile soru sayýsý elde tutuldu.                                         //
//                           Son olarak yine LCD de oyuncuya kazanýp kazanmadýðý bildirildi.                                                                           //
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

// LCD için tanýmlanan deðiþkenler
#define LCD_DAT PORTK
#define LCD_DIR DDRK
#define LCD_E 0x02
#define LCD_RS 0x01
#define CMD 0
#define DATA 1 

// sorularýn olduðu 2D array
char *questions[40] = { "AGRI DAGI ERZURUMDADIR",      //38  -F
                                      "2X2'NIN SONUCU 8 EDER",                                       //22  -F
                                      "MIKRO KELIMESI 5 HARFTEN OLUSUR",               //32  -T
                                      "TBMM 1820 DE KURULMUSTUR",                            //25   -F
                                      "ATATURK ILK CUMHURBASKANIDIR",                    //29    -T
                                      "DONUSUM YAZARI FRANZ KAFKA'DIR",                  //31   -T
                                      "4-2'NIN SONUCU 2 EDER",                                        //22  -T
                                      "LINKEDLIST BIR VERI YAPISIDIR",                            //30 -T
                                     "QUICK SORT MERGE SORT'TAN DAHA HIZLIDIR" };  //39 -T                                      
                                      

// LCD de gösterebilmek için her sorunun size'ýnýn tutulduðu array
int sizes[9] = {23, 22, 32, 25, 29, 31, 22, 30, 39};

// ekrana yazýlacak diðer deðiþkenler
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
        printLCD(questions[index], sizes[index]);     // soru ekrana yazýldý
        
         PORTB = index;              // soru sayýsý PORTB de gösterildi
        
        delay_1s(5);  // 5saniye PORTH a deger girilmesi bekleniyor
         switch(index){    //Sorularýn cevaplarýna göre gerekli iþlemler yapýldý.
            case 0:   //F
            case 1:  //F
            case 3:  //F
            
               if(PTH == 1)           //1>> T            // Sorunun doðru cevabý yanlýþsa ve user T girdiyse
              {
                 TCTL1 =0x04;   ///buzzer open           // buzzer öter
                 TC5 = TCNT+ (word)2000;
                 delay_1s(3);                                      // 3saniye
                 TCTL1 = 0x00; //buzzer close
                 
                  falseAns++;                   // yanlýþ verilen cevap sayýsý artar
                  wrongAnswer();             // LCD de yanlýþ cevap verdiði yazýlýr
              }
              else if(PTH == 2)      //2>> F         // Sorunun doðru cevabý doðruysa ve user F girdiyse
              {       
                 TCTL1 =0x04;   ///buzzer open         // buzzer öter
                 TC5 = TCNT+ (word)9600;
                 delay_1s(3);                                      // 3 saniye 
                 TCTL1 = 0x00; //buzzer close          // 3 saniye sonra buzzer kapatýlýr.
                 
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
        
        index++;   // index bir sonraki soru için arttýrýldý
      }
      //dogru cevap verildiginde buzzer oter, diger soruya gecince buzzer clear edildi
     // TCTL1 =0x00;   //clear buzzer
        
      if(index >= 9) {      
          
          break;     // index soru sayýsýna geldiyse bu iþlemler biter.  
      }
    }
     
     //dogru ve yanlis sayilari 7-segment te önce soðru sayýsý sonra yanlýþ sayýsý þeklinde sýrayla gosterildi.
     print7Segment(); 
          
     // OYUNUN SONU
     if(trueAns >= falseAns){
      printLCD(CONGR, 29);   // Doðru sayýsý yanlýþ sayýsýndan fazla veya eþit ise kazanýlýr ve bu lcd de gösterilir.
     } else{
      printLCD(SORRY, 20);      // yanlýþ sayýsý doðru sayýsýndan fazla ise kaybedilir ve bu lcd de gösterilir.
     }
     
    
    __asm(swi);

	EnableInterrupts;

  for(;;) {
    _FEED_COP(); 
  }   
}


///////////////////////FUNCTIONSS////////////////////////////////
void printLCD(char *q, int size){     // gelen stringi lcd de gösterir
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


void print7Segment(void){       /// 7-segment te  doðru yanlýþ sayýsý gösterilir. 
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
        PORTB = 0x3F;   // doðru sayýsýna göre 7-Segmente deðer yazýrýldý
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
     
     PORTB = 0x00;     // ve PORTB 0 yapýldý böylece 7-segment te hiçbir þey gözükmedi.
      
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


//////////////////////// LCD FUNCTIONS (slaytlardan alýndý) //////////////
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
