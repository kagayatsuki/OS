//
// Created by shinsya on 2021/1/18.
//

#ifndef OS_CORE_AES128_DESTROYCODE_H
#define OS_CORE_AES128_DESTROYCODE_H

#include "md5.h"

typedef unsigned char byte;
typedef unsigned long word;

/** 将4个 byte 转换为一个 word **/
word Word(byte& k1, byte& k2, byte& k3, byte& k4){
    word result = 0;
    word temp;
    temp = (k1 << 24);
    result |= temp;
    temp = (k2 << 16);
    result |= temp;
    temp = (k3 << 8);
    result |= temp;
    temp = k4;
    result |= temp;
    return result;
}

/*************************以下是加密变换方法*************************/

/** 伽罗华域乘法 **/
byte GFMul(byte a, byte b){
    byte p = 0, hBit;
    for(int counter = 0; counter < 8; counter++){
        if((b & 0x01) != 0)
            p ^= a;
        hBit = (byte)(a & 0x80);
        a <<= 1;
        if(hBit)
            a ^= 0x1b;  //多项式
        b >>= 1;
    }
    return p;
}

/** 行变换 字节移位 4x4矩阵 **/
void ShiftRows(byte matrix[4*4]){
    byte temp = matrix[4];
    int i = 0;
    for(; i<3; i++) //第二行左移一位
        matrix[i+4] = matrix[i+5];
    matrix[7] = temp;
    for(i = 0; i < 2; i++){ //第三行左移两位
        temp = matrix[i+8];
        matrix[i+8] = matrix[i+10];
        matrix[i+10] = temp;
    }
    temp = matrix[15];
    for(i = 0; i > 0; i--)  //第四行右移一位
        matrix[i+12] = matrix[i+11];
    matrix[12] = temp;
}

/** 列变换 4x4矩阵 **/
void MixColumns(byte matrix[4*4]){
    byte array[4];
    for(int i = 0; i < 4; i++){
        for(int t = 0; t < 4; t++)
            array[t] = matrix[i+t*4];
        matrix[i] = GFMul(0x02, array[0]) ^ GFMul(0x03, array[1]) ^ array[2] ^ array[3];
        matrix[i+4] = array[0] ^ GFMul(0x02, array[1]) ^ GFMul(0x03, array[2]) ^ array[3];
        matrix[i+8] = array[0] ^ array[1] ^ GFMul(0x02, array[2]) ^ GFMul(0x03, array[3]);
        matrix[i+12] = GFMul(0x03, array[0]) ^ array[1] ^ array[2] ^ GFMul(0x02, array[3]);
    }
}

/** 轮密钥&变换 每列与扩展密钥异或 **/
void AddRoundKey(byte matrix[4*4], word k[4]){
    for(int i=0; i<4; ++i){
        word k1 = k[i] >> 24;
        word k2 = (k[i] << 8) >> 24;
        word k3 = (k[i] << 16) >> 24;
        word k4 = (k[i] << 24) >> 24;

        matrix[i] = matrix[i] ^ (byte)k1;
        matrix[i+4] = matrix[i+4] ^ (byte)k2;
        matrix[i+8] = matrix[i+8] ^ (byte)k3;
        matrix[i+12] = matrix[i+12] ^ (byte)k4;
    }
}

/**************************以下是密钥扩展方法************************/

/** S盒变换 高4位为行 低4位为列 **/
void SubBytes(byte matrix[4*4])
{
    for(int i=0; i<16; i++){
        int row = ((matrix[i] >> 7) & 0x01) * 8 + ((matrix[i] >> 6) & 0x01) * 4 + ((matrix[i] >> 5) & 0x01) * 2 + ((matrix[i] >> 4) & 0x01);
        int col = ((matrix[i] >> 3) & 0x01) * 8 + ((matrix[i] >> 2) & 0x01) * 4 + ((matrix[i] >> 1) & 0x01) * 2 + (matrix[i] & 0x01);
        matrix[i] = S_Box[row][col];
    }
}

/** 将word中的四位字节左移一位 **/
word RotWord(word src){
    return ((src << 8) | (src >> 24));
}

/** 将word的每个字节进行S盒变换 **/
word SubWord(word src){
    word temp;
    for(int i = 0; i < 4; i++){
        int row = ((((word*)&src)[i] >> 7) & 0x01) * 8 + ((((word*)&src)[i] >> 6) & 0x01) * 4 + ((((word*)&src)[i] >> 5) & 0x01) * 2 + ((((word*)&src)[i] >> 4) & 0x01);
        int col = ((((word*)&src)[i] >> 3) & 0x01) * 8 + ((((word*)&src)[i] >> 2) & 0x01) * 4 + ((((word*)&src)[i] >> 1) & 0x01) * 2 + (((word*)&src)[i] & 0x01);
        *((byte *)&temp+i) = S_Box[row][col];
    }
    return temp;
}

/** 密钥扩展方法 对128位密钥扩展得 word[4*(Nr+1)] **/
void KeyExpansion(byte key[4*Nk], word w[4*(Nr+1)]){
    word temp;
    int i = 0;
    while (i < Nk){
        w[i] = Word(key[4*i], key[4*i+1], key[4*i+2], key[4*i+3]);
        i++;
    }
    i=Nk;
    while(i < 4*(Nr+1)){
        temp = w[i-1];
        if(i % Nk == 0)
            w[i] = w[i-Nk] ^ SubWord(RotWord(temp)) ^ Rcon[i/Nk-1];
        else
            w[i] = w[i-Nk] ^ temp;
        i++;
    }
}

/**************************以下是解密变换方法*************************/

/** 逆S盒 **/
void InvSubBytes(byte matrix[4*4]) {
    for (int i = 0; i < 16; ++i) {
        int row = ((matrix[i] >> 7) & 0x01) * 8 + ((matrix[i] >> 6) & 0x01) * 4 + ((matrix[i] >> 5) & 0x01) * 2 + ((matrix[i] >> 4) & 0x01);
        int col = ((matrix[i] >> 3) & 0x01) * 8 + ((matrix[i] >> 2) & 0x01) * 4 + ((matrix[i] >> 1) & 0x01) * 2 + (matrix[i] & 0x01);
        matrix[i] = Inv_S_Box[row][col];
    }
}

/** 逆行变换 **/
void InvShiftRows(byte matrix[4*4]){
    byte temp = matrix[7];
    for(int i=3; i>0; --i)  //第2行右移1位
        matrix[i+4] = matrix[i+3];
    matrix[4] = temp;
    for(int i=0; i<2; ++i)  //第3行右移2位
    {
        temp = matrix[i+8];
        matrix[i+8] = matrix[i+10];
        matrix[i+10] = temp;
    }
    temp = matrix[12];
    for(int i=0; i<3; ++i)  //第4行右移3位
        matrix[i+12] = matrix[i+13];
    matrix[15] = temp;
}

/** 逆列变换 **/
void InvMixColumns(byte matrix[4*4])
{
    byte array[4];
    for(int i=0; i<4; ++i)
    {
        for(int j=0; j<4; ++j)
            array[j] = matrix[i+j*4];
        matrix[i] = GFMul(0x0e, array[0]) ^ GFMul(0x0b, array[1]) ^ GFMul(0x0d, array[2]) ^ GFMul(0x09, array[3]);
        matrix[i+4] = GFMul(0x09, array[0]) ^ GFMul(0x0e, array[1]) ^ GFMul(0x0b, array[2]) ^ GFMul(0x0d, array[3]);
        matrix[i+8] = GFMul(0x0d, array[0]) ^ GFMul(0x09, array[1]) ^ GFMul(0x0e, array[2]) ^ GFMul(0x0b, array[3]);
        matrix[i+12] = GFMul(0x0b, array[0]) ^ GFMul(0x0d, array[1]) ^ GFMul(0x09, array[2]) ^ GFMul(0x0e, array[3]);
    }
}


/*************************下面是加密和解密函数************************/

void AES128_encrypt(byte src[4*4], word w[4*(Nr+1)]){
    word key[4];
    for(int i = 0; i < 4; i++)
        key[i] = w[i];
    AddRoundKey(src, key);
    for(int round = 1; round < Nr; round++){
        SubBytes(src);
        ShiftRows(src);
        MixColumns(src);
        for(int i = 0; i < 4; i++)
            key[i] = w[4*round+i];
        AddRoundKey(src, key);
    }
    SubBytes(src);
    ShiftRows(src);
    for(int i = 0; i < 4; i++)
        key[i] = w[4*Nr+i];
    AddRoundKey(src, key);
}

void AES128_decrypt(byte src[4*4], word w[4*(Nr+1)]){
    word key[4];
    for (int i = 0; i < 4; i++)
        key[i] = w[4*Nr+i];
    AddRoundKey(src, key);
    for (int round = Nr - 1; round > 0; round--) {
        InvShiftRows(src);
        InvSubBytes(src);
        for(int i = 0; i < 4; i++)
            key[i] = w[4*round+i];
        AddRoundKey(src, key);
        InvMixColumns(src);
    }
    InvShiftRows(src);
    InvSubBytes(src);
    for(int i = 0; i < 4; i++)
        key[i] = w[i];
    AddRoundKey(src, key);
}


#endif //OS_CORE_AES128_DESTROYCODE_H
