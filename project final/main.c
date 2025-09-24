#include <stdio.h>

FILE* ReadFile(){
    FILE *file =fopen("tools.csv","r");
    if (file == NULL)
    {
        printf("File: %p\n", file);
        printf("ไม่สามารถเปิดไฟล์ได้\n");
    }
    return file;
}

void DisplayFile(){
    FILE *file = ReadFile();
    if (file == NULL) {
        return;
    }
    char line[100];
    printf("\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}


void AddOrder(){

}

void SearchOrder(){

}

void UpdateOrder(){

}

void DeleteOrder(){

}

int Menu(){
    int menu;
    printf("\nOrder management system\n");
    printf("1) Display all order\n");
    printf("2) Add new order\n");
    printf("3) Search for an order\n");
    printf("4) Update order\n");
    printf("5) Delete an order\n");
    printf("6) Exit program\n");
    printf("Select an option(1-6) ");
    scanf("%d",&menu);
    return menu ;
}

int main(){
    int menu ;
    do {
        menu = Menu(); 
        switch (menu){
            case 1 :
                DisplayFile();
                break ;
            case 2 :
                AddOrder();
                break ;
            case 3 :
                SearchOrder();
                break ;
            case 4 :
                UpdateOrder();
                break ;
            case 5 :
                DeleteOrder();
                break ;
            case 6 :
                printf("Exiting...");
                break ;
            default :
                printf("Invalid choise\n");
            }
    }while (menu != 6);         
    return 0;
}