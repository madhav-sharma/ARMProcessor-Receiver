
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

void SystemInit(void);
void Initialize_ADC_IQ(uint16_t ADC1_mode, uint32_t ADC_Sample_Rate);
void Initialize_Test_Pins();
void Initialize_USART(uint32_t baudrate);
int16_t ADC1_SAMPLE_Fifo_Get(int16_t *);
void plot_data(float array[], int16_t size);
void find_average(float st[4][100], float *average);
int find_start(float average[100]);
void determine_Sample(float samples[100], float st[4][100], int index);
int determine_databit1(float *samples);
void find_time(int *data, int *time_info);


float getsample();

#define N 4
#define fs 48000
int sine_table[N];
int cosine_table[N];
float pi = 3.141592654;
float st[4][100]; //matrix with the dimensions 4X100 is created to store 4 seconds of data
float average[100], samples1[100], samples2[100], samples3[100], liveSamples[100];
int minData[1000];

int main(void)
{
    int i;
    int j=0;
    int q;
    int ctr;
    int time_info[4];

    
    SystemInit();
    Initialize_USART(2000000);
    Initialize_ADC_IQ(0, fs); // km--for ultrasonic 48000 for WWVB
    Initialize_Test_Pins();
    
    for (ctr=0; ctr<N; ctr++)
    {
        sine_table[ctr] = 512*sin(2.0*pi*ctr/(float)N);
        cosine_table[ctr]=512*cos(2.0*pi*ctr/(float)N);
        //printf("sine_table[ %i]= %i \n",ctr,sine_table[ctr]);
        //printf("cosine_table[ %i]= %i \n",ctr,sine_table[ctr]);
    }
    
    
    
    
    for (i = 0; i < 100; i++)
    {
        average[i] = 0;
        samples1[i] = 0;
        samples2[i] = 0;
        samples3[i] = 0;
        liveSamples[i] = 0;
    }
    
    /*if (inputfile == NULL)
     {
     printf("error reading file\n");
     exit(0);
     }*/
    
    
    for (j = 0; j < 4; j++)
    {
        
        for (i = 0; i < 100; i++)
        {
            
            st[j][i] = getsample();
            //printf("st[%i][%i]=%lf\n", j, i,st[j][i]);
        }
        plot_data(st[j],100);
    }
    
    find_average(st, average);
    
    int index1 = find_start(average);
    
    //printf("index1=%i\n",index1);
    
    int index2 = index1 + 100;
    int index3 = index1 + 200;
    
    
    determine_Sample(samples1, st, index1);
    determine_Sample(samples2, st, index2);
    determine_Sample(samples3, st, index3);
    
    int databit1 = determine_databit1(samples1);
    int databit2 = determine_databit1(samples2);
    int databit3 = determine_databit1(samples3);
    
    int liveDatabit;
    
    
    
    minData[0] = databit1;
    minData[1] = databit2;
    minData[2] = databit3;
    
    j=0;
    
    while (1){
        
        //if (getsample()!= NULL)break;
        
        for (i = 0; i < 100; i++)st[j][i]=getsample();
        
        find_average(st, average);
        
        int liveIndex = find_start(average);
        
        if (j == 0) liveIndex = liveIndex + 300;
        else if (j == 2) liveIndex = liveIndex + 100;
        else if (j == 3) liveIndex = liveIndex + 200;
        
        determine_Sample(liveSamples, st, liveIndex);
        liveDatabit = determine_databit1(liveSamples);
        
        /*printf("%i \n", liveIndex);
         printf("%i \n", liveDatabit);*/
        
        minData[q] = liveDatabit;
        
        if (minData[q] == 2 && minData[q - 1] == 2){
            if (q > 59){
                find_time(&(minData[q - 60]), time_info);
                printf("Year: %d    Day: %d    Hour: %d    Minute: %d\n", time_info[0], time_info[1], time_info[2], time_info[3]);
            }
        }
        /*for (int i = 0; i < 100; i++){
         printf("liveSamples[%i]=%lf\n", i, liveSamples[i]);
         }*/
        
        plot_data(liveSamples,100);
        printf("%i \n", liveDatabit);
        q++;
        j++;
        if (j == 4)j = 0;
        //if (q==60)q=0;
        
    }
    
    /*printf("%i \n", liveDatabit);*/
    
    /*for (int i = 0; i <20; i++){
    	printf("minData[%i]=%i\n", i, minData[i]);
    	}*/
    
    /*printf("%i \n", databit1);
    	printf("%i \n", databit2);
    	printf("%i \n", databit3);
    	*/
    
    /*for (int i = 0; i < 100; i++){
    	printf("average[%i]=%lf\n", i, average[i]);
    	}
     */
    getchar();
    
    
    return 0;
    
}   // END OF MAIN FUNCTION

void find_average(float st[4][100], float *average){
    int i;
    for (i = 0; i < 100; i++){
        average[i] = (st[0][i] + st[1][i] + st[2][i] + st[3][i]) / 4;
    }
    
}

int find_start(float average[100])
{
    
    
    
    
    int i;
    float value= 0.0;
    float test= 0.0;
    int index;
    
    for (i = 0; i < 99; i++)
    {
        value = average[i + 1] - average[i];
        if (value < 0 && value< test)
        {
            test = value;
            index= i + 1;
        }
    }
    if (average[0] - average[99] < test){
        index= 0;
    }
    return index;
    
    
}


//         FILLS ARRAY WITH 100 SAMPLES

void determine_Sample(float samples[100], float st[4][100], int index){
    int counter;
    int i = 0;
    int r=0;
    if (index < 100){
        r = 0;
    }
    else if (index < 200 && index>99){
        r = 1;
        index = index - 100;
    }
    else if (index<300 && index >199){
        r = 2;
        index = index - 200;
    }
    else if (index>299 && index<400){
        r = 3;
        index = index - 300;
    }
    else{
        printf(" index ERROR ");
    }
    
    for (counter = index; counter < 100; counter++){
        samples[i] = st[r][counter];
        i++;
    }
    r++;
    if (r == 4){
        r = 0;
    }
    for (counter = 0; counter < index; counter++){
        samples[i] = st[r][counter];
        i++;
        
    }
    
}


//          FINDS THE TIME FROM THE DATA BITS

int determine_databit1(float *samples){
    int n;
    int databit;
    
    float average1 = 0; float average2 = 0; float average3 = 0; float average4 = 0;
    
    for (n = 0; n < 100; n++)
    {
        if (n < 20){
            average1 = samples[n] + average1;
        }
        else if (n < 50){
            average2 = samples[n] + average2;
        }
        else if (n < 80){
            average3 = samples[n] + average3;
        }
        else if (n < 100){
            average4 = samples[n] + average4;
        }
    }
    average1 = average1 / (float)20;
    average2 = average2 / (float)30;
    average3 = average3 / (float)30;
    average4 = average4 / (float)20;
    
    if (average4>2 * average3 && average4 > 2 * average2 && average4 > 2 * average1){
        
        databit = 2;
    }
    else if (average3 > 2 * average2 && average3 > 2 * average1 && average4 > 2 * average2 && average4 > 2 * average1){
        
        databit = 1;
    }
    else if (average2 > 2 * average1 && average3 > 2 * average1 && average4 > 2 * average1){
        
        databit = 0;
    }
    else {
        databit = 3; //error bit
        printf(" bit ERROR ");
    }
    
    return databit;
}


//          FINDS THE TIME FROM THE DATA BITS


void find_time(int *data, int *time_info)
{
    time_info[3] = data[1] * 40 + data[2] * 20 + data[3] * 10 + data[5] * 8 + data[6] * 4 + data[7] * 2 + data[8] * 1;
    
    time_info[2] = data[12] * 20 + data[13] * 10 + data[15] * 8 + data[16] * 4 + data[17] * 2 + data[18] * 1;
    
    time_info[1] = data[22] * 200 + data[23] * 100 + data[25] * 80 + data[26] * 40 + data[27] * 20 + data[28] * 10 + data[30] * 8 + data[31] * 4 + data[32] * 2 + data[33] * 1;
    
    time_info[0] = data[45] * 80 + data[46] * 40 + data[47] * 20 + data[48] * 10 + data[50] * 8 + data[51] * 4 + data[52] * 2 + data[53] * 1;
}


//          GETS FIRST N_hundredth=48000 (32000, 400) SAMPLES
//          MIXES SAMPLES WITH SINE AND COSINE

float getsample()
{
    int N1;
    int16_t ut;
    float ft;
    int i;
    float utc, utc_sum2;
    float uts, uts_sum2;
    float utc_Total = 0;
    float uts_Total = 0;
    int ctr = 0;
    
    N1=fs/100;
    
    for (i = 0; i < N1; i++){
        
        while (ADC1_SAMPLE_Fifo_Get(&ut)==0);
        
        uts = ut*sine_table[ctr] / 512.0;
        utc = ut*cosine_table[ctr] / 512.0;
        uts_Total = uts_Total + uts;
        utc_Total = utc_Total + utc;
        
        ctr++;
        if (ctr == N)ctr = 0;
    }
    
    //fscanf(fp_input, "%e",&ut);
    //GETS A SAMPLE FROM ADC FIFO
    utc_sum2 = (utc_Total / 320)*(utc_Total / 320);
    uts_sum2 = (uts_Total / 320)*(uts_Total / 320);
    ft = sqrt(utc_sum2 + uts_sum2);
    
    
    
    
    return ft;
}

void InitPlotSupport(void);
void GoTo(uint16_t, uint16_t, uint16_t);
void DrawLine(uint16_t, uint16_t);
void printstring(uint16_t x, uint16_t y, uint16_t step, uint16_t rotation, uint16_t color, char *cp);
#define XSF 1
void plot_data(float array[], int16_t size)
{
    int16_t x_offset, y_offset;
    int16_t ctr, y, index, penup;
    int16_t SPS;
    
    InitPlotSupport();
    
    x_offset = 100; y_offset= 100;
    index = 0;
    penup = 1;
    SPS = size;
    
    for (ctr=0; ctr<SPS; ctr++) {
        y = array[index];
        y=y/5.0;
        if (penup == 1) {
            GoTo(2, XSF*ctr+x_offset, y+y_offset);
            penup = 0;
        }
        else {
            DrawLine(XSF*ctr+x_offset, y+y_offset);
        }
        if (++index >= SPS) {
            index -= SPS;
        }
    }
}
