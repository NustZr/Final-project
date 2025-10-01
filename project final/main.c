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
    int price;

    printf("Enter Order ID: ");
    scanf("%s", orderID);

    getchar();
    printf("Enter Tool Name: ");
    fgets(Tools, sizeof(Tools), stdin);
    Tools[strcspn(Tools, "\n")] = 0;

    printf("Enter Quantity: ");
    scanf("%d", &quantity);

    printf("Enter Price per Unit: ");
    scanf("%d", &price);

    fprintf(file, "%s,%s,%d,%d\n", orderID, Tools, quantity, price);

    fclose(file);
    printf("Order added\n");
}

void SearchOrder(){

}

void UpdateOrder(){

}

void DeleteOrder(){
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL){
        printf("Cannot open file\n");
        return;
    }

    FILE *temp = fopen("temp.csv", "w");
    if (temp == NULL){
        printf("Cannot create temp file\n");
        fclose(file);
        return;
    }

    char orderID[5];
    printf("Enter Order ID : ");
    scanf("%s", orderID);

    char line[256];
    int deleted = 0;

    while (fgets(line, sizeof(line), file)) {
        char buf[256];
        strcpy(buf, line);


        char *id = strtok(buf, ",");
        if (id != NULL && strcmp(id, orderID) == 0) {
            deleted = 1;
            continue;
        }
        fputs(line, temp);
    }

    fclose(file);
    fclose(temp);

    remove("tools.csv");
    rename("temp.csv", "tools.csv");

    if (deleted) {
        printf("Order %s deleted successfully.\n", orderID);
    } else {
        printf("Order %s not found.\n", orderID);
    }
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
