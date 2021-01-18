//
// Created by shinsya on 2020/12/1.
//

#include "md5.h"
#include "aes128.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

int debug_toolbox_main(){
    char buffer_data[512] = {0};
    char buffer_key[64] = {0};
    char buffer_ec[512] = {0}, buffer_dc[512] = {0};
    printf("Input original data: ");
    gets(buffer_data);
    printf("Input key: ");
    gets(buffer_key);
    Encrypt_AES128(buffer_ec, buffer_data, strlen(buffer_data), buffer_key);
    printf("Encrypted: %s\n", buffer_ec);
    Decrypt_AES128(buffer_dc, buffer_ec, strlen(buffer_data), buffer_key);
    printf("Decrypted: %s\n", buffer_dc);
    getchar();
    return 0;
}

int debug2(){
    using namespace std;
    byte key[16] = {0x2b, 0x7e, 0x15, 0x16,
                    0x28, 0xae, 0xd2, 0xa6,
                    0xab, 0xf7, 0x15, 0x88,
                    0x09, 0xcf, 0x4f, 0x3c};
    byte data[128] = {0};
    char buff[64] = {0}, buff2[16];
    char buff3[128] = {0};
    int data_len;
    cout << "input data: ";
    cin >> buff3;
    data_len = strlen(buff3);
    cout << "Input key: ";
    cin >> buff;
    _core_toolbox_md5_get(buff, (unsigned char*)buff2);
    for(int t = 0; t<16; t++)
        key[t] = buff2[t];

    word w[4*(Nr+1)];		//扩展密钥

    KeyExpansion(key, w);

    for(int i = 0; i<data_len; i++)
        data[i] = buff3[i];
    for(int i = 0; i<data_len / 16 + (data_len%16?1:0); i++)
        AES128_encrypt(&data[i*16], w);

    cout << "encrypted: ";
    for(int i = 0; i<data_len; i++)
        printf("%c", (char)data[i].to_ulong());

    for(int i = 0; i<data_len / 16 + (data_len%16?1:0); i++)
        AES128_decrypt(&data[i*16], w);

    cout << endl << "decrypted: ";
    for(int i = 0; i<data_len; i++)
        printf("%c", (char)data[i].to_ulong());
    return 0;
}

int main(){
    return debug_toolbox_main();
}
