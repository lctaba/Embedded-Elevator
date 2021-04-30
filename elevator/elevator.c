#include "reg51.h"   //声明单片机寄存器
#include "intrins.h" //使用位移函数

#define uchar unsigned char 
#define uint unsigned int 



//按键对应的P1IO口的电平
#define IN_1_1 0x11
#define IN_1_2 0x21
#define IN_1_3 0x41
#define IN_1_4 0x81
#define IN_2_1 0x12
#define IN_2_2 0x22
#define IN_2_3 0x42
#define IN_2_4 0x82
#define OUT_1_UP 0x14
#define OUT_2_UP 0x24
#define OUT_2_DOWN 0x44
#define OUT_3_UP 0x84
#define OUT_3_DOWN 0x18
#define OUT_4_DOWN 0x28


uchar liftUp1[4] = {0,0,0,0};    //1号上行请求队列
uchar liftDown1[4] = {0,0,0,0};  //1号下行请求队列
uchar liftUp2[4] = {0,0,0,0};    //2号上行请求队列
uchar liftDown2[4] = {0,0,0,0};  //2号下行请求队列
uchar liftUp[4] = {0,0,0,0};     //电梯外部上行请求
uchar liftDown[4] = {0,0,0,0};   //电梯外部下行请求
uchar isUpHandled[4] = {0,0,0,0};   //外部请求是否被处理，1表示被处理
uchar isDownHandled[4] = {0,0,0,0}; //外部请求是否被处理，1表示被处理
uchar currentLevel1 = 1;         //1号电梯当前楼层 
uchar currentLevel2 = 1;         //2号电梯当前楼层 
uchar nextLevel1 = 1;            //1号电梯下一个要到达的楼层
uchar nextLevel2 = 1;            //2号电梯下一个要到达的楼层

bit DIR1 = 1;                    //1号方向,1是向上,0是向下
bit DIR2 = 1;                    //2号方向,1是向上,0是向下
bit IDLE1 = 1;                   //1号是否空闲
bit IDLE2 = 1;                   //2号是否空闲
bit STOP1 = 1;                   //1号是否停止
bit STOP2 = 1;                   //2号是否停止
uchar TIMER1 = 0;                //用于定时1秒，20个周期后归0；
uchar TIMER2 = 0;                //用于定时1秒，20个周期后归0；

//控制数码管的IO口
sbit LED_1_1 = P0^0;
sbit LED_1_2 = P0^1;
sbit LED_2_1 = P0^2;
sbit LED_2_2 = P0^3;


//控制电梯内部LED的IO口
sbit LED_IN_1_1 = P2^0;
sbit LED_IN_1_2 = P2^1;
sbit LED_IN_1_3 = P2^2;
sbit LED_IN_1_4 = P2^3;
sbit LED_IN_2_1 = P2^4;
sbit LED_IN_2_2 = P2^5;
sbit LED_IN_2_3 = P2^6;
sbit LED_IN_2_4 = P2^7;
//控制电梯外部LED的IO口
sbit UP1 = P3^0;
sbit UP2 = P3^1;
sbit UP3 = P3^2;
sbit DOWN2 = P3^3;
sbit DOWN3 = P3^4;
sbit DOWN4 = P3^5;
//控制显示电梯上下LED的IO口
sbit LIFT1UP = P0^4;
sbit LIFT1DOWN = P0^5;
sbit LIFT2UP = P0^6;
sbit LIFT2DOWN = P0^7;
//控制开门的LED的IO口
sbit OPEN1 = P3^6;
sbit OPEN2 = P3^7;

void init(){
    int i = 0;
    for(i=4; i>0; i--){
        liftUp1[i] = 0;
        liftDown1[i] = 0;
        liftUp2[i] = 0;
        liftDown2[i] = 0;
        liftUp[i] = 0;
        liftDown[i] = 0;
    }
    currentLevel1 = 1;         //1号电梯当前楼层 
    currentLevel2 = 1;         //2号电梯当前楼层 
    nextLevel1 = 1;            //1号电梯下一个要到达的楼层
    nextLevel2 = 1;            //2号电梯下一个要到达的楼层
}

/*
 *  初始化定时器 
 */
void initTimer(){
    TMOD = 0x01;
    TH0 = (65536-50000)/256; //设置初值为50000us,50ms的高四位
    TL0 = (65536-50000)%256; //设置初值为50000us,50ms的低四位
    ET0 = 1;                 //开启定时器0的中断
    EA = 1;                  //开启总中断
    TR0 = 1;                 //启动定时器0
    TIMER1 = 0;
}

/*
 *  延迟
 */
void delay(uint time){
    int i=0,j=0;
    for(i=time; i>0; i--){
        for(j=120;j>0;j--){

        }
    }
}
/*
 *  延迟1秒
 */
void delay1s(){
	int i=0, j=0;
    for(i=120; i>0; i--){
        for(j=1000; j>0; j--){

        }
    }
}

/*
 *  延迟20毫秒
 */
void delay20ms(){
	int i=0;
    for(i=2400; i>0; i--){

    }
}

/*
 *  延迟0.5秒,模拟电梯上升
 */
void delay500ms(){
	int i=0, j=0;
    for(i=120; i>0; i--){
        for(j=500; j>0; j--){

        }
    }
}

/*
 *  用于扫描矩阵键盘,获取被按下的按钮信息
 */
uchar keyboardScan(){
	uchar keyResult = 0x00;
	uchar highEnd = 0x00;
	uchar lowEnd = 0x00;
    P1 = 0xf0;   //初始化P1的八个口，前四个置为高电平，后四个置为低电平
    if((P1 & 0xf0) != 0xf0){  //如果P1的高四位不全为1，说明有键按下
        delay20ms();  //延时去抖动
        if((P1 & 0xf0) != 0xf0){  //按钮确实被按下
            highEnd = P1 ^ 0xf0;
            P1 = 0x0f;
            lowEnd = P1 ^ 0x0f;
            return highEnd | lowEnd;
        }
    }
    return 0x00;
}


uchar sum(uchar level[]){
    int i=0;
    uchar levels = 0;
    for(i=4; i>0; i--){
        levels += level[i-1];
    }
    return levels;
}

/*
 *  按键处理事件
 */
void keyProcess(){
    uchar key = keyboardScan();
    switch (key)
    {
    case IN_1_1:
        if(currentLevel1 > 1){
            liftDown1[0] = 1;
        }
        //LED_IN_1_1 = 0;
        break;
    case IN_1_2:
        if(currentLevel1 > 2){
            liftDown1[1] = 1;
        }else if (currentLevel1 < 2){
            liftUp1[1] = 1; 
        }
        //LED_IN_1_2 = 0;
        break;
    case IN_1_3:
        if(currentLevel1 > 3){
            liftDown1[2] = 1;
        }else if (currentLevel1 < 3){
            liftUp1[2] = 1; 
        }
        //LED_IN_1_3 = 0;
        break;
    case IN_1_4:
        if (currentLevel1 < 4){
            liftUp1[3] = 1; 
        }
        //LED_IN_1_4 = 0;
        break;
    case IN_2_1:
        if(currentLevel2 > 1){
            liftDown2[0] = 1;
        }
        //LED_IN_2_1 = 0;
        break;
    case IN_2_2:
        if(currentLevel2 > 2){
            liftDown2[1] = 1;
        }else if (currentLevel2 < 2){ 
            liftUp2[1] = 1;
        }
        //LED_IN_2_2 = 0;
        break;
    case IN_2_3:
        if(currentLevel2 > 3){
            liftDown2[2] = 1;
        }else if (currentLevel2 < 3){ 
            liftUp2[2] = 1;
        }
        //LED_IN_2_3 = 0;
        break;
    case IN_2_4:
        if (currentLevel2 < 4){ 
            liftUp2[3] = 1;
        }
        //LED_IN_2_4 = 0;
        break;


    case OUT_1_UP:
        //if(nextLevel1 != 1 & nextLevel2 != 1){
            liftUp[0] = 1;
        //}
        break;


    case OUT_2_DOWN:
        //if(!(!DIR1 & nextLevel1 == 2) & !(!DIR2 & nextLevel2 == 2)){
            liftDown[1] = 1;
        //}
        break;


    case OUT_2_UP:
        //if(!(DIR1 & nextLevel1 == 2) & !(DIR2 & nextLevel2 == 2)){
            liftUp[1] = 1;
        //}
        break;
    case OUT_3_DOWN:
        //if(!(!DIR1 & nextLevel1 == 3) & !(!DIR2 & nextLevel2 == 3)){
            liftDown[2] = 1;
        //}
        break;
    case OUT_3_UP:
        //if(!(DIR1 & nextLevel1 == 3) & !(DIR2 & nextLevel2 == 3)){
            liftUp[2] = 1;
        //}
        break;
    case OUT_4_DOWN:
        //if(nextLevel1 != 4 & nextLevel2 != 4){
            liftDown[3] = 1;
        //}
        break;
    default:
        break;
    }
}

/*
 *  显示楼层
 */
void display(){
    int i = 0;
    switch (currentLevel1)
    {
    case 1:
        LED_1_1 = 0;
        LED_1_2 = 0;
        break;
    case 2:
        LED_1_1 = 1;
        LED_1_2 = 0;
        break;
    case 3:
        LED_1_1 = 0;
        LED_1_2 = 1;
        break;
    case 4:
        LED_1_1 = 1;
        LED_1_2 = 1;
        break;
    default:
        break;
    }

    switch (currentLevel2)
    {
    case 1:
        LED_2_1 = 0;
        LED_2_2 = 0;
        break;
    case 2:
        LED_2_1 = 1;
        LED_2_2 = 0;
        break;
    case 3:
        LED_2_1 = 0;
        LED_2_2 = 1;
        break;
    case 4:
        LED_2_1 = 1;
        LED_2_2 = 1;
        break;
    default:
        break;
    }
    LED_IN_1_1 = !(liftUp1[0] | liftDown1[0]);
    LED_IN_1_2 = !(liftUp1[1] | liftDown1[1]);
    LED_IN_1_3 = !(liftUp1[2] | liftDown1[2]);
    LED_IN_1_4 = !(liftUp1[3] | liftDown1[3]);
    LED_IN_2_1 = !(liftUp2[0] | liftDown2[0]);
    LED_IN_2_2 = !(liftUp2[1] | liftDown2[1]);
    LED_IN_2_3 = !(liftUp2[2] | liftDown2[2]);
    LED_IN_2_4 = !(liftUp2[3] | liftDown2[3]);
    UP1 = !liftUp[0];
    UP2 = !liftUp[1];
    UP3 = !liftUp[2];
    DOWN2 = !liftDown[1];
    DOWN3 = !liftDown[2];
    DOWN4 = !liftDown[3];
    LIFT1UP = !(sum(liftUp1) == 0);
    LIFT2UP = !(sum(liftDown1) == 0);
    LIFT1DOWN = !(sum(liftUp) == 0);
    LIFT2DOWN = !(sum(liftDown) == 0);
    if(STOP1 & !IDLE1){
        OPEN1 = 0;
    }else{
        OPEN1 = 1;
    }
    
    if(STOP2 & !IDLE2){
        OPEN2 = 0;
    }else{
        OPEN2 = 1;
    }
}

/*
 *  定时器1中断函数
 */
void timerIsr() interrupt 1{
    if(TIMER1 <= 20){
        TH0 = (65536-50000)/256; //设置初值为50000us,50ms的高四位
        TL0 = (65536-50000)%256; //设置初值为50000us,50ms的低四位
        TIMER1 ++;
        if(TIMER1 == 20){
            display();
        }
    }
}

/*
 *  去下一层
 */
void gotoNextLevel1(){
    if(nextLevel1 == currentLevel1){
        int j = 0;
        if(DIR1){
            if(liftUp[currentLevel1-1] == 1){
                STOP1 = 1;
                IDLE1 = 0;
                liftUp1[currentLevel1-1] = 0;
                liftUp[currentLevel1-1] = 0;
                for(j=70;j>0;j--){
                    delay20ms();
                    keyProcess();
                    display();
                }
                //IDLE1 = 1;
                STOP1 = 0;
            }
        }else{
            if(liftDown[currentLevel1-1] == 1){
                STOP1 = 1;
                IDLE1 = 0;
                liftDown1[currentLevel1-1] = 0;
                liftDown[currentLevel1-1] = 0;
                for(j=70;j>0;j--){
                    delay20ms();
                    keyProcess();
                    display();
                }
                //IDLE1 = 1;
                STOP1 = 0;
            }
        }
    }

    if(nextLevel1 > currentLevel1){
        int i=0,j=0;
        DIR1 = 1;
        IDLE1 = 0;
        //LED_IN_1_4 = 0;
        STOP1 = 0;
        for(i=35;i>0;i--){
            delay20ms();
            keyProcess();
            display();
        }
        currentLevel1++;
        liftUp1[currentLevel1-1] = 0;
        liftUp[currentLevel1-1] = 0;
        display();
        if(currentLevel1 == nextLevel1){
            STOP1 = 1;
            for(j=70;j>0;j--){
                delay20ms();
                keyProcess();
                display();
            }
            STOP1 = 0;
            //当所内外部都没有请求的时候，电梯闲置
            if((sum(liftUp1) == 0) & (sum(liftDown1) == 0) & (sum(liftUp) == 0) & (sum(liftDown) == 0)){
                IDLE1 = 1;
            } 
        }
        return;
    }

    if(nextLevel1 < currentLevel1){
        int i=0,j=0;
        DIR1 = 0;
        IDLE1 = 0;
        STOP1 = 0;
        for(i=35;i>0;i--){
            delay20ms();
            keyProcess();
            display();
        }
        currentLevel1--;
        liftDown1[currentLevel1-1] = 0;
        liftDown[currentLevel1-1] = 0;
        display();
        if(currentLevel1 == nextLevel1){
            STOP1 = 1;
            for(j=70;j>0;j--){
                delay20ms();
                keyProcess();
                display();
            }
            STOP1 = 0;
            //当所内外部都没有请求的时候，电梯闲置
            if((sum(liftUp1) == 0) & (sum(liftDown1) == 0) & (sum(liftUp) == 0) & (sum(liftDown) == 0)){
                IDLE1 = 1;
            }
        }
        return;
    }
}

/*
 *  1号电梯工作
 */
void work1(){
    //电梯空闲就会自动扫描外部请求
    if(IDLE1){
        if(DIR1){
            uchar temp = currentLevel1;
            //LIFT1UP = 1;
            while(temp > 0 && !(liftUp[temp-1] | liftDown[temp-1])){
                temp--;
            }
            if(temp > 0){
                if(nextLevel2 != temp){
                    nextLevel1 = temp;
                    //LIFT1UP = 0;
                    //DIR1 = 0;
                }
            }
        }else{
            uchar temp = currentLevel1;
            while(temp < 5 && !(liftUp[temp-1] | liftDown[temp-1])){
                temp++;
            }
            if(temp < 5){
                if(nextLevel2 != temp){
                    nextLevel1 = temp;
                    //DIR1 = 1;
                }
            }
        }
    }
    //电梯方向向上
    if(DIR1){
        uchar temp = currentLevel1;
        //问题出在这里   temp从currentlevel开始，那么外部的低于currentLevel的请求无法处理
        while ((temp < 5) && !(liftUp1[temp-1] | (liftUp[temp-1] & !(DIR1 & nextLevel2 == temp)))){          //当有电梯内部请求，或有另一部电梯未响应的外部请求，则响应
            temp++;
        }
        if(temp < 5){
            nextLevel1 = temp;
            //表示外部请求已被处理
            //liftUp[temp-1] = 0;
        }else{
            DIR1 = 0;
        }
    }else{
        uchar temp = currentLevel1;
        while ((temp > 0) && !(liftDown1[temp-1] | (liftDown[temp-1] & !(!DIR1 & nextLevel2 == temp)))){
            temp--;
        }
        if(temp > 0){
            nextLevel1 = temp;
            //表示外部请求已被处理
            //liftDown[temp-1] = 0;
        }else{
            DIR1 = 1;   
        }
    }
    gotoNextLevel1();
}


/*
 *  去下一层
 */
void gotoNextLevel2(){
    if(nextLevel2 == currentLevel2){
        int j = 0;
        if(DIR2){
            if(liftUp[currentLevel2-1] == 1){
                STOP2 = 1;
                IDLE2 = 0;
                liftUp2[currentLevel2-1] = 0;
                liftUp[currentLevel2-1] = 0;
                for(j=70;j>0;j--){
                    delay20ms();
                    keyProcess();
                    display();
                }
                //IDLE2 = 1;
                STOP2 = 0;
            }
        }else{
            if(liftDown[currentLevel2-1] == 1){
                STOP2 = 1;
                IDLE2 = 0;
                liftDown2[currentLevel2-1] = 0;
                liftDown[currentLevel2-1] = 0;
                for(j=70;j>0;j--){
                    delay20ms();
                    keyProcess();
                    display();
                }
                //IDLE2 = 1;
                STOP2 = 0;
            }
        }
    }

    if(nextLevel2 > currentLevel2){
        int i=0,j=0;
        //LED_IN_1_4 = 0;
        DIR2 = 1;
        IDLE2 = 0;
        STOP2 = 0;
        for(i=35;i>0;i--){
            delay20ms();
            keyProcess();
            display();
        }
        currentLevel2++;
        liftUp2[currentLevel2-1] = 0;
        liftUp[currentLevel2-1] = 0;
        display();
        if(currentLevel2 == nextLevel2){
            STOP2 = 1;
            for(j=70;j>0;j--){
                delay20ms();
                keyProcess();
                display();
            }
            STOP2 = 0;
            //当所内外部都没有请求的时候，电梯闲置
            if((sum(liftUp2) == 0) & (sum(liftDown2) == 0) & (sum(liftUp) == 0) & (sum(liftDown) == 0)){
                IDLE2 = 1;
            } 
        }
        return;
    }

    if(nextLevel2 < currentLevel2){
        int i=0,j=0;
        DIR2 = 0;
        IDLE2 = 0;
        STOP2 = 0;
        for(i=35;i>0;i--){
            delay20ms();
            keyProcess();
            display();
        }
        currentLevel2--;
        liftDown2[currentLevel2-1] = 0;
        liftDown[currentLevel2-1] = 0;
        display();
        if(currentLevel2 == nextLevel2){
            STOP2 = 1;
            for(j=70;j>0;j--){
                delay20ms();
                keyProcess();
                display();
            }
            STOP2 = 0;
            //当所内外部都没有请求的时候，电梯闲置
            if((sum(liftUp2) == 0) & (sum(liftDown2) == 0) & (sum(liftUp) == 0) & (sum(liftDown) == 0)){
                IDLE2 = 1;
            } 
        }
        return;
    }
}

/*
 *  1号电梯工作
 */
void work2(){
    //电梯空闲就会自动扫描外部请求
    if(IDLE2){
        if(DIR2){
            uchar temp = currentLevel2;
            //LIFT1UP = 1;
            while(temp > 0 && !(liftUp[temp-1] | liftDown[temp-1])){
                temp--;
            }
            if(temp > 0){
                if(nextLevel1 != temp){
                    nextLevel2 = temp;
                    //LIFT1UP = 0;
                    //DIR2 = 0;
                }
            }
        }else{
            uchar temp = currentLevel2;
            while(temp < 5 && !(liftUp[temp-1] | liftDown[temp-1])){
                temp++;
            }
            if(temp < 5){
                if(nextLevel1 != temp){
                    nextLevel2 = temp;
                    //DIR2 = 1;
                }
            }
        }
    }
    //电梯方向向上
    if(DIR2){
        uchar temp = currentLevel2;
        while ((temp < 5) && !(liftUp2[temp-1] | (liftUp[temp-1] & !(DIR2 & nextLevel1 == temp)))){
            temp++;
        }
        if(temp < 5){
            nextLevel2 = temp;
            //表示外部请求已被处理
            //liftUp[temp-1] = 0;
        }else{
            DIR2 = 0;
        }
    }else{
        uchar temp = currentLevel2;
        while ((temp > 0) && !(liftDown2[temp-1] | (liftDown[temp-1] & !(!DIR2 & nextLevel1 == temp)))){
            temp--;
        }
        if(temp > 0){
            nextLevel2 = temp;
            //表示外部请求已被处理
            //liftDown[temp-1] = 0;
        }else{
            DIR2 = 1;   
        }
    }
    gotoNextLevel2();
}

void word(){
    work1();
    work2();
}

void main(){
    //init();
	while(1){
        P1 = 0xf0;
        keyProcess();
        display();
        work1();
        work2();
    }
}

        // if(DIR1 & DIR2){                            //两台电梯都向上，选择向上停靠次数最少的电梯
        //     if(sum(liftUp2) > sum(liftUp1)){
        //         liftUp2[0] = 1;
        //     }else{
        //         liftUp1[0] = 1;
        //     }
        // }else if (!DIR1 & DIR2){                    //如果电梯1向下，电梯2向上，选择电梯1
        //     liftUp1[0] = 1;
        // }else if (DIR1 & !DIR2){                    //如果电梯2向下，电梯1向上，选择电梯2
        //     liftUp2[0] = 1;
        // }else{                                      //如果两条电梯都下降，选择最近的一台
        //     if(currentLevel2 > currentLevel1){
        //         liftUp2[0] = 1;
        //     }else{
        //         liftUp1[0] = 1;
        //     }
        // }

        // if(DIR1 & DIR2){                            //两台电梯都向上，选择向上停靠次数最少的电梯
        //     if(sum(liftUp2) > sum(liftUp1)){
        //         liftDown2[1] = 1;
        //     }else{
        //         liftDown1[1] = 1;
        //     }
        // }else if (!DIR1 & DIR2){                    //如果电梯1向下，电梯2向上，
        //     if(currentLevel1 >= 2){                 //如果电梯1当前楼层大于等于2，选择电梯1
        //         liftDown1[1] = 1;
        //     }else{                                  //否则选择电梯2
        //         liftDown2[1] = 1;
        //     }
        // }else if (DIR1 & !DIR2){                    //如果电梯2向下，电梯1向上，
        //     if(currentLevel2 >= 2){                 //如果电梯2当前楼层大于等于2，选择电梯2
        //         liftDown2[1] = 1;
        //     }else{                                  //否则选择电梯1
        //         liftDown1[1] = 1;
        //     }
        // }else{                                      //如果两台电梯都下降，
        //     if(currentLevel1 >= 2 & currentLevel2 >= 2){   //如果两台电梯的高度都大于等于2，选择近的
        //         if(currentLevel2 > currentLevel1){
        //             liftDown2[1] = 1; 
        //         }else{
        //             liftDown1[1] = 1;
        //         }
        //     }else if(currentLevel1 >= 2){           //只有第1部电梯大于2层选择电梯1
        //         liftDown1[1] = 1;
        //     }else if(currentLevel2 >= 2){           //只有第2部电梯大于2层选择电梯2
        //         liftDown2[1] = 1; 
        //     }else{                                  //否则选择向上停靠最少的电梯
        //         if(sum(liftUp2) > sum(liftUp1)){
        //             liftDown2[1] = 1;
        //         }else{
        //             liftDown1[1] = 1;
        //         }
        //     }
        // }


        // if(DIR1 & DIR2){                            //两台电梯都向上
        //     if(currentLevel1 <= 2 & currentLevel2 <= 2){  //两台电梯都在2楼以下，选择最近的电梯
        //         if(currentLevel2 > currentLevel1){  
        //             liftUp2[1] = 1;
        //         }else{
        //             liftUp1[1] = 1;
        //         }
        //     }else if(currentLevel1 <= 2){            //只有1号电梯在2楼以下，选择1号
        //         liftUp1[1] = 1;
        //     }else if(currentLevel2 <= 2){            //只有2号电梯在2楼以下，选择2号
        //         liftUp2[1] = 1;
        //     }else{                                   //两台电梯都在2楼以上，选择向上和向下
        //         if(sum(liftUp2) > sum(liftUp1)){
        //             liftUp2[0] = 1;
        //         }else{
        //             liftUp1[0] = 1;
        //         }
        //     }
        // }else if (!DIR1 & DIR2){                    //如果电梯1向下，电梯2向上，选择电梯1
        //     liftUp1[0] = 1;
        // }else if (DIR1 & !DIR2){                    //如果电梯2向下，电梯1向上，选择电梯2
        //     liftUp2[0] = 1;
        // }else{                                      //如果两条电梯都下降，选择最近的一台
        //     if(currentLevel2 > currentLevel1){
        //         liftUp2[0] = 1;
        //     }else{
        //         liftUp1[0] = 1;
        //     }
        // }