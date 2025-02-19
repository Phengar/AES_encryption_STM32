#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const unsigned char sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 
};

static const unsigned char rcon[11] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 
};

static const unsigned char message[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static const unsigned char key[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};


void print_key_16(unsigned char *keyW){ //debug function
    for(int i=0; i<16; i++){
        printf("%x", keyW[i]);
    }
}


//multGalois isn't optimal as the only values of b are 1,2 and 3

/*unsigned char multGalois(unsigned char a, unsigned char b){
    unsigned char p=0;
    for (int i=0; i<8; i++){
        if ((b & 0x01) == 0x01){
            p^=a;
        }
        if ((a & 0x80) == 0x80){
            a<<=1;
            a^=0x1b;
        }else{
            a<<=1;
        }
        b>>=1;
    }
    return p;
}*/


// implementation of Galois multiplication optimised for b=1,2,3

unsigned char multGalois_opti(unsigned char a, unsigned char b){//si ça c'est pas de l'optimisation
    unsigned char p;
    switch(b){
        case 0x01: return a;    //multiplication by 1=00000001
        case 0x02: return ((a&0x80) == 0x80) ? (a<<1)^0x1b : a<<1; //multiplication by 2==00000010
        case 0x03: return ((a&0x80) == 0x80) ? a^(a<<1)^0x1b : a^(a<<1); //multiplication by 3==00000001
        default: printf("\n error in Galois mult: b=%x\n",b); exit(1); //catch errors
    }
}

void mixColumns_4(unsigned char * keyPart){ //multiply Galois matrix with vector keyPart
    unsigned char save[4]={keyPart[0], keyPart[1], keyPart[2], keyPart[3]}, mix[4]={0x02, 0x03, 0x01, 0x01}; //saves the part of the key to modify the key directly without losing data
    for (int i=0;i<4;i++){ 
        keyPart[0]=0;
        keyPart[1]=0;
        keyPart[2]=0;
        keyPart[3]=0;
    }
    for (int i=0;i<4;i++){ // consecutive multiplication of the part of the key
        keyPart[0]^=multGalois_opti(save[i],mix[i]);
        keyPart[1]^=multGalois_opti(save[i], mix[(i+3)%4]);
        keyPart[2]^=multGalois_opti(save[i], mix[(i+2)%4]);
        keyPart[3]^=multGalois_opti(save[i], mix[(i+1)%4]);
    }
}

void mixColumns_16(unsigned char *State){// multiply each 4bytes vector one by one with mixColumns_4
    unsigned char temp[4];
    for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            temp[j]=State[i*4+j];
        }
        mixColumns_4(temp);
        for (int j=0; j<4; j++){
            State[i*4+j]=temp[j];
        }
    }
}

void rotate_key(unsigned char * keyPart){ //rotation on 4 bytes part of the key
    unsigned char temp=keyPart[0];
    for (int i=0; i<3; i++){
        keyPart[i]=keyPart[i+1];
    }
    keyPart[3]=temp;
}

void SBox_4(unsigned char *keyPart){ //Sbox for the key schedule
    for(int i=0; i<4; i++){
        keyPart[i]=sbox[keyPart[i]];
    }
}

void SBox_16(unsigned char *State){ //Sbox for the state
    for(int i=0; i<16; i++){
        State[i]=sbox[State[i]];
    }
}

void ShiftRows(unsigned char *State){ //Rotations for the state
    unsigned char temp1=State[1],temp2=State[2];
    State[1]=State[5]; State[5]=State[9]; State[9]=State[13]; State[13]=temp1;
    temp1=State[6]; State[2]=State[10]; State[6]=State[14]; State[10]=temp2; State[14]=temp1;
    temp1=State[15]; State[15]=State[11]; State[11]=State[7]; State[7]=State[3]; State[3]=temp1;
}

void RCon(unsigned char *keyPart, int i){ //Rcon operation for key schedule
    keyPart[0] ^= rcon[i];
}

void keySch(unsigned char *keyPart, int i){ //KEY SCHEDULE
    rotate_key(keyPart);
    SBox_4(keyPart);
    RCon(keyPart,i);
}

void AddRoundKey(unsigned char *a, unsigned char *b){ //modify a to a^b
    for (int i=0; i<16; i++){
        a[i] ^= b[i];
    }
}

int main(int argc, unsigned char *argv[]){
   if(argc<3){ //catch error
       printf("please supplie the message to encode and the key");
       exit(1);
   }
    if(sizeof(argv[2])/sizeof(unsigned char)!=16){ //catch error
        printf("size of key is %d when it should be 16\n", sizeof(argv[2])/sizeof(unsigned char));
    }
    if(sizeof(argv[1])/sizeof(unsigned char)!=16){ //catch error
        printf("size of message is %d when it should be 16\n", sizeof(argv[2])/sizeof(unsigned char));
    }
    for( int i=0; i<16; i++){// for tests on static constants
        argv[2][i]=key[i];
        argv[1][i]=message[i];
    }
    printf("round[0].input ");
    print_key_16(argv[1]);
    printf("\n");
    printf("round[0].k_sch ");
    print_key_16(argv[2]);
    printf("\n");
    AddRoundKey(argv[1],argv[2]);
    for (int i=1;i<11; i++){
        printf("round[%d].start ", i);
        print_key_16(argv[1]);
        printf("\n");
        SBox_16(argv[1]);  //SBOX
        printf("round[%d].s_box ", i);
        print_key_16(argv[1]);
        printf("\n");
        ShiftRows(argv[1]);  //SHIFTROWS
        printf("round[%d].s_row ", i);
        print_key_16(argv[1]);
        printf("\n");
        if(i != 10){
            mixColumns_16(argv[1]);  //MIXCOLUMNS
            printf("round[%d].m_col ", i);
            print_key_16(argv[1]);
            printf("\n");
        }
        unsigned char save[4];
        for (int k=0; k<4; k++){
            save[k]=argv[2][12+k];
        }
        keySch(save,i);  //ROUNDKEYS
        for (int j=0; j<4; j++){
            argv[2][j]^= save[j];
            argv[2][4+j]^= argv[2][j];
            argv[2][8+j]^= argv[2][4+j];
            argv[2][12+j]^= argv[2][8+j];
        }
        printf("round[%d].k_sch ", i);
        print_key_16(argv[2]);
        printf("\n");
        AddRoundKey(argv[1],argv[2]); //ADDROUNDKEY
    }
    printf("Encrypted message: ");
    print_key_16(argv[1]);  //DECRYPTED
}
