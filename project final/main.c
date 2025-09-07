#include <stdio.h>
int main(){
FILE *file = fopen("tools.CVS", "r");
if (file == NULL) {
    printf("ไม่สามารถเปิดไฟล์ได้\n");
    return 1;
}

int ch;
while ((ch = fgetc(file)) != EOF) {
    putchar(ch);
}

fclose(file);
}

