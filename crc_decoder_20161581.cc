#include<cstdio>
#include<cstdlib>
#include<string.h>
unsigned char input_byte[10000000];
char codeword_str[100000000];
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
unsigned int str_to_bin( char* str){
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
void shift_pad(int padding_bit_num){
    
}

unsigned int modulo_div(unsigned int dividend,unsigned int divisor){
    int dividend_msb_off=find_msb_offset(dividend);
    int divisor_msb_off=find_msb_offset(divisor);
    int off_diff=dividend_msb_off-divisor_msb_off;
    unsigned int remainder=0;
    remainder=dividend;
    while(off_diff>=0){
        int remainder_off=find_msb_offset(remainder);
        if(remainder_off==(off_diff+divisor_msb_off)){
            /*
            printf("divid: ");
            print_bin(remainder);
            printf("divis: ");
            print_bin(divisor<<off_diff);
            */
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
int main(int argc, char* argv[]){
    if(argc!=6){
        printf("usage: ./crc_encoder input_file output_file result_file generator dataword_size\n");
        return 0;
    }
    char*generator_str=argv[4];
    char*dataword_size_str=argv[5];
    int dataword_size;
    
    FILE* input_file_ptr=NULL;
    FILE* output_file_ptr=NULL;
    FILE* result_file_ptr=NULL;
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
    if((result_file_ptr=fopen(argv[3],"w"))==NULL){
        printf("result file open error.\n");
        fclose(input_file_ptr);
        fclose(output_file_ptr);
        fclose(result_file_ptr);
        return 0;
    }
    if((strcmp(dataword_size_str,"4")!=0)&&(strcmp(dataword_size_str,"8")!=0)){
        printf("dataword size must be 4 or 8\n");
        fclose(input_file_ptr);
        fclose(output_file_ptr);
        fclose(result_file_ptr);
        return 0;
    }
    else{
        dataword_size=atoi(dataword_size_str);
    }

    /*
    char padding_byte[10];
    for(int i=0;i<8;i++){
        padding_byte[i]=fgetc(input_file_ptr);
    }
    padding_byte[8]='\0';
    int padding_bit_num=str_to_bin(padding_byte);
    */
    unsigned char input_char;
    fscanf(input_file_ptr,"%c",&input_char);
    int padding_bit_num=(int)input_char;
    /*
    for(int i=0;i<padding_bit_num;i++){
        char tmp=fgetc(input_file_ptr);
    }*/
    unsigned int generator=str_to_bin(generator_str);
    int generator_size=find_msb_offset(generator);

    int codeword_size=dataword_size+generator_size-1;
    char codeword_buf[20];
    int cnt=0;
    int err_cnt=0;
    int codeword_cnt=0;
    int character_size=0;
    unsigned int character_bit=0;

    int byte_idx=0;
    while(fscanf(input_file_ptr,"%c",&input_char)!=EOF){
        input_byte[byte_idx]=input_char<<padding_bit_num;
        if(byte_idx>0){
            input_byte[byte_idx-1]=input_byte[byte_idx-1]|(input_char>>(8-padding_bit_num));
            bin_to_str(input_byte[byte_idx-1],8,codeword_str+(byte_idx-1)*8);
        }
        byte_idx++;
    }
    //char print_buf[20];
    //printf("lst: ");
    bin_to_str(input_byte[byte_idx-1],8,codeword_str+(byte_idx-1)*8);
    //printf("codeword: %s\n",codeword_str);
    //printf("codewordlen: %d\n",strlen(codeword_str));
    //printf("%c\n",codeword_str[strlen(codeword_str)-1]);
    //bin_to_str(input_byte[byte_idx-1],8-padding_bit_num,codeword_str+(byte_idx-1)*8);
    //printf("last : %s\n",codeword_str+(byte_idx)*8);
    //printf("size : %d\n",(byte_idx)*8-padding_bit_num);
    for(int i=0;i<((byte_idx)*8)-padding_bit_num;i++){
        codeword_buf[cnt++]=codeword_str[i];
        if(cnt==codeword_size){
            codeword_buf[cnt]='\0';
            codeword_cnt++;
            unsigned int codeword=str_to_bin(codeword_buf);
            unsigned int remainder=modulo_div(codeword,generator);
            if(remainder!=0){
                err_cnt++;
            }
            unsigned int dataword=codeword>>(generator_size-1);
            char dataword_buf[10];
            bin_to_str(dataword,dataword_size,dataword_buf);
            
            character_size+=dataword_size;
            if(character_size==8){
                character_bit=character_bit|dataword;
                fputc(character_bit,output_file_ptr);
                //printf("%c\n",character_bit);
                character_size=0;
                character_bit=0;
            }
            else{
                character_bit=dataword;
                character_bit=character_bit<<dataword_size;
            }
            cnt=0;
        }
    }
    fprintf(result_file_ptr,"%d %d",codeword_cnt,err_cnt);
    fclose(input_file_ptr);
    fclose(output_file_ptr);
    fclose(result_file_ptr);
}