#include <stdio.h>

void ReadFile(){
    FILE *file =fopen("tools.csv","r");
    if (file == NULL)
    {
        printf("File: %p\n", file);
        printf("ไม่สามารถเปิดไฟล์ได้\n");
    }
    else
    {
        char line[100];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
    }

}


// void WriteFile(){

// }

// void AddOrder(){

// }

int main(){
    ReadFile();
    return 0;
}