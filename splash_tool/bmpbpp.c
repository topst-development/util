/****************************************************************************
* FileName? : bmpbpp.c
* Description : 
*****************************************************************************
*
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not 
limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source 
code shall constitute any express or implied warranty of any kind, including
without limitation, any warranty of merchantability, fitness for a particular 
purpose or non-infringement of any patent, copyright or other third party 
intellectual property right. No warranty is made, express or implied, regarding 
the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability 
arising from, out of or in connection with this source code or the use in the 
source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure 
Agreement between Telechips and Company.
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


static int get_file_size (int fd){

    return lseek(fd, 0 , SEEK_END);
}

int main(int argc, char **argv)
{
    int bmpbpp = 0;
    
    if (argc == 2){
        int sz , fd;
        //BITMAPHEADER bitmap;
        unsigned char bitmap[100];

        fd = open(argv[1] , O_RDONLY, 0644);
        if(fd < 0){
            printf(" File [%s] Open Failed !!\n", argv[1]);
            goto opps;
        }

        sz = get_file_size(fd);

        if(sz < 0){
            printf(" Get File [%d]  Size Failed !!\n", sz);
            goto opps;
        }

        lseek(fd, 0 , SEEK_SET);

        memset(&bitmap, 0x0, sizeof(bitmap));

        if(read(fd, &bitmap, sizeof(bitmap)) != sizeof(bitmap)) goto opps;

        if(bitmap[0] == 'B' && bitmap[1] == 'M') {
            memcpy(&bmpbpp, &bitmap[28], sizeof(unsigned short));
        }
        opps:
        close(fd);
    } 

    // notify to shell
    printf("%d", bmpbpp);
    return 0;
}


