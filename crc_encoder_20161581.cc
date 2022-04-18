#include<cstdio>
#include<cstdlib>
#include<string.h>
char encoding_buf[100000000];
void print_bin(unsigned int bit_stream){
    for(int i=sizeof(unsigned int)*8-1;i>=0;i--){
        printf("%d",(bit_stream>>i)&1);
    }
    printf("\n");
}
void bin_to_str(unsigned int bit_stream, int size, char* str){
    for(int i=size-1;i>=0;i--){
        str[size-1-i]=((bit_stream>>i)&1)+'0';
    }
    str[size]='\0';
    return;
}
unsigned int str_to_bin( char * str){
    int len=strlen(str);
    unsigned int bit_stream=0;
    for(int i=0;i<len;i++){
        bit_stream= bit_stream<<1;
        if(str[i]=='1')
            bit_stream+=1;
    }
    return bit_stream;
}
int find_msb_offset(unsigned int bit_str){
    int off=0;
    unsigned int tmp=bit_str;
    while(tmp!=0){
        tmp=tmp>>1;
        off++;
    }
    return off;
}

unsigned int modulo_div(unsigned int dividend,unsigned int divisor){
    int dividend_msb_off=find_msb_offset(dividend);
    int divisor_msb_off=find_msb_offset(divisor);
    int off_diff=dividend_msb_off-divisor_msb_off;
    unsigned int remainder=0;
    remainder=dividend;
    while(off_diff>=0){
        int remainder_off=find_msb_offset(remainder);
        /*
        printf("divid: ");
        print_bin(remainder);
        printf("divis: ");
        print_bin(divisor<<off_diff);
        */
        if(remainder_off==(off_diff+divisor_msb_off)){
            remainder=remainder^(divisor<<off_diff);
        }
        off_diff--;
    }
    /*
    printf("remai: ");
    print_bin(remainder);
    */
    return remainder;
}

unsigned int encode_datacode(unsigned int datacode,unsigned int generator,int dataword_size){
    unsigned int codeword=0;
    int divisor_msb_off=find_msb_offset(generator);// highest set bit 
    int dividend=datacode;

    if(dataword_size==4){ //for high 4 bit
        dividend=dividend>>4;
    }

    dividend=dividend<<(divisor_msb_off-1);
    unsigned int remainder=modulo_div(dividend,generator);
    codeword=dividend^remainder;

    if(dataword_size==8)    // char is 8bit and dataword size is 8bit.
        return codeword;

    codeword=codeword<<(dataword_size+divisor_msb_off-1);

    dividend=datacode&0xf; // for low 4 bit
    dividend=dividend<<(divisor_msb_off-1);
    remainder=modulo_div(dividend,generator);
    codeword=codeword|(dividend^remainder);
    return codeword;
}

int main(int argc, char* argv[]){
    if(argc!=5){
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        return 0;
    }
    char* generator_str=argv[3];
    char* dataword_size_str=argv[4];
    int dataword_size;
    FILE* input_file_ptr=NULL;
    FILE* output_file_ptr=NULL;
    if((input_file_ptr=fopen(argv[1],"r"))==NULL){
        printf("input file open error.\n");
        fclose(input_file_ptr);
        return 0;
    }
    if((output_file_ptr=fopen(argv[2],"w"))==NULL){
        printf("output file open error.\n");
        fclose(input_file_ptr);
        fclose(output_file_ptr);
        return 0;
    }
    if((strcmp(dataword_size_str,"4")!=0)&&(strcmp(dataword_size_str,"8")!=0)){
        printf("dataword size must be 4 or 8\n");
        fclose(input_file_ptr);
        fclose(output_file_ptr);
        return 0;
    }
    else{
        dataword_size=atoi(dataword_size_str);
    }


    unsigned int generator=0;
    generator=str_to_bin(generator_str);


    int total_encoding_size=0;
    char input_char;
    int generator_size=find_msb_offset(generator);
    int tmp_encoding_size=(8/dataword_size)*(generator_size-1)+8;
    while(true){
        input_char=fgetc(input_file_ptr);
        if(input_char==EOF){
            break;
        }
        char dataword_buf[10];
        bin_to_str((unsigned int)input_char,8,dataword_buf);

        unsigned int codeword=encode_datacode((unsigned int)input_char,generator,dataword_size);
        bin_to_str(codeword,tmp_encoding_size,encoding_buf+total_encoding_size);
        total_encoding_size+=tmp_encoding_size;
    }
    

    unsigned int padding_bit_num=8-(total_encoding_size%8);
    if(padding_bit_num==8)
        padding_bit_num=0;
    fprintf(output_file_ptr,"%c",(char)padding_bit_num);

    
    char print_buf[10];
    //bin_to_str((char)padding_bit_num,8,print_buf);
    //printf("%s",print_buf);
   
    int bin=0;
    for(int i=0;i<8-padding_bit_num;i++){
        bin=bin<<1;
        bin=bin+encoding_buf[i]-'0';
    }
    fprintf(output_file_ptr,"%c",(char)bin);
    
    //bin_to_str((char)bin,8,print_buf);
    //printf("%s",print_buf);
    
    int enc_cnt=0;
    bin=0;
    for(int i=8-padding_bit_num;i<total_encoding_size;i++){
        bin=bin<<1;
        bin+=encoding_buf[i]-'0';
        enc_cnt++;
        if(enc_cnt==8){
            fprintf(output_file_ptr,"%c",bin);
            enc_cnt=0;
            
            //bin_to_str((char)bin,8,print_buf);
            //printf("%s",print_buf);
            
            bin=0;
        }
    }

    /*
    char padding_num_str[10];
    bin_to_str(padding_bit_num,8,padding_num_str);
    //printf("%s\n",padding_num_str);
    for(int i=0;i<8;i++){
        fputc(padding_num_str[i],output_file_ptr);
    }
    for(int i=0;i<padding_bit_num;i++){
        fputc('0',output_file_ptr);
    }
    for(int i=0;i<total_encoding_size;i++){
        fputc(encoding_buf[i],output_file_ptr);
    }
    */
    fclose(input_file_ptr);
    fclose(output_file_ptr);

    return 0;
}