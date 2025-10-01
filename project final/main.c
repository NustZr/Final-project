#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *ReadFile(){
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL){
        printf("Cannot open file\n");
        return NULL;
    }
    return file;
}

void DisplayFile(){
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL){
        return;
    }
    char line[256];
    printf("\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

void AddOrder(){
    FILE *file = fopen("tools.csv","a");
    if (file == NULL){
        printf("Cannot open file\n");
        return;
    }

    char orderID[5];
    char Tools[100];
    int quantity;
    float price;

    printf("Enter Order ID: ");
    scanf("%s", orderID);

    getchar();
    printf("Enter Tool Name: ");
    fgets(Tools, sizeof(Tools), stdin);
    Tools[strcspn(Tools, "\n")] = 0;

    printf("Enter Quantity: ");
    scanf("%d", &quantity);

    printf("Enter Price per Unit: ");
    scanf("%f", &price);

    fprintf(file, "%s,%s,%d,%.2f\n", orderID, Tools, quantity, price);

    fclose(file);
    printf("Order added\n");
}

void SearchOrder(){

}

void UpdateOrder(){

}

void DeleteOrder(){

}

// MENU
int Menu(){
    int menu;
    printf("\n===== Order management system =====\n");
    printf("1) Display all orders\n");
    printf("2) Add new order\n");
    printf("3) Search for an order\n");
    printf("4) Update order\n");
    printf("5) Delete an order\n");
    printf("6) Exit program\n");
    printf("Select an option (1-6): \n");
    scanf("%d",&menu);
    return menu;
}

// MAIN
int main(){
    int menu;
    do {
        menu = Menu();
        switch (menu){
            case 1:
                DisplayFile();
                break;
            case 2:
                AddOrder();
                break;
            case 3:
                SearchOrder();
                break;
            case 4:
                UpdateOrder();
                break;
            case 5:
                DeleteOrder();
                break;
            case 6:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice\n");
        }
    } while (menu != 6);
    return 0;
}
