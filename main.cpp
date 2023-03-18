#include "mbed.h"

DigitalOut r1(P0_14);   //1k
DigitalOut r2(P0_6);    //10k
DigitalOut r3(P0_12);   //100k
DigitalOut r4(P0_16);   //1meg
I2C i2c(P0_10,P0_11);

void lcd_init(int adr);     //lcd init func
const int lcd_adr = 0x7C;   //lcd i2c adr 0x7C
void char_disp(int adr, int8_t position, char data);
void val_disp(int adr, int8_t position, int8_t digit,int val);
void ohm_disp(int adr, int8_t position);

#define intv 0.1
#define res 0.0625  //resolution 2.048/2^16
const int sps=0b10;     //0b00->240sps,12bit 0b01->60sps,14bit 0b10->15sps,16bit
const int pga=0;     //0->x1 1->x2 2->x4 3->x8
const int adc_adr = 0xd0;
char buf[2],set[1];
int16_t raw_val;
float val_f,dut_f;
uint32_t dut;
uint8_t ch,ovf;
void range (uint8_t ch);    //range R sw func.

int main(){
    wait(0.2);
    lcd_init(lcd_adr);
    set[0]=(0b1001<<4)+(sps<<2)+pga;//0x98
    i2c.write(adc_adr,set,1);
    r1=0;
    r2=0;
    r3=0;
    r4=0;
    wait(0.2);
    char_disp(lcd_adr,4,',');
    ohm_disp(lcd_adr,0x40+7);
    while(1){
        while(1){
            range(ch);
            wait(0.1);
            i2c.read(adc_adr|1,buf,2);  //adc read
            raw_val=(buf[0]<<8)+buf[1];
            val_f=raw_val*res;  //mV expression
            if((val_f>500)&&(val_f<1500)) break;
            else if (val_f<=500&&(ch==0||ch==1)) ch++;
            else if (val_f>=1500&&(ch==1||ch==2)) ch--;
            else if(val_f>=1500&&ch==0) break;
            else if(val_f<=500&&ch==2) break;
        }

        if(ch==0)dut_f=10000*(2048-val_f)/val_f;
        else if(ch==1)dut_f=100000*(2048-val_f)/val_f;
        else if(ch==2)dut_f=1000000*(2048-val_f)/val_f;
        
        if(val_f<10)ovf=1;
        else ovf=0;

        dut=(uint32_t)dut_f;
        val_disp(lcd_adr,0,4,dut/1000);
        val_disp(lcd_adr,5,3,dut%1000);

        if(ovf==1){
            char_disp(lcd_adr,0x40+0,'!');
        }else{
            char_disp(lcd_adr,0x40+0,' ');
        }
        //wait(intv);
    } 
}

//range R sw
void range (uint8_t ch){
    if(ch==0){
        r1=0;
        r2=1;
        r3=0;
        r4=0;
    }else if(ch==1){
        r1=0;
        r2=0;
        r3=1;
        r4=0;
    }else if(ch==2){
        r1=0;
        r2=0;
        r3=0;
        r4=1;
    }else if(ch==3){
        r1=0;
        r2=0;
        r3=0;
        r4=0;
    }
}

//disp char func
void char_disp(int adr, int8_t position, char data){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]='@';
    buf[1]=data;
    i2c.write(adr,buf, 2);
}

//disp char func
void ohm_disp(int adr, int8_t position){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]='@';
    buf[1]=30;
    i2c.write(adr,buf, 2);
}

//disp val func
void val_disp(int adr, int8_t position, int8_t digit, int val){
    char buf[2];
    char data[4];
    int8_t i;
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    data[3]=0x30+val%10;        //1
    data[2]=0x30+(val/10)%10;   //10
    data[1]=0x30+(val/100)%10;  //100
    data[0]=0x30+(val/1000)%10; //1000
    buf[0]='@';
    for(i=0;i<digit;++i){
        buf[1]=data[i+4-digit];
        i2c.write(adr,buf, 2);
    }
}

//LCD init func
void lcd_init(int adr){
    char lcd_data[2];
    lcd_data[0] = 0x0;
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x39;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x14;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x70;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x56;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x6C;
    i2c.write(adr, lcd_data, 2);
    wait(0.2);
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x0C;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x01;
    i2c.write(adr, lcd_data, 2);
}
