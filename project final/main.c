#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//=========================================//

FILE *ReadFile() {
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL) {
        printf("Cannot open file\n");
        return NULL;
    }
    return file;
}

//=========================================//

void DisplayFile() {
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL) {
        return;
    }
    char line[256];
    printf("\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

//=========================================//

int similarity(const char *a, const char *b) {
    int lenA = strlen(a);
    int lenB = strlen(b);
    if (lenA == 0 || lenB == 0) return 0;

    int diff = abs(lenA - lenB);
    int same = 0;

    for (int i = 0; i < lenA && i < lenB; i++) {
        if (tolower(a[i]) == tolower(b[i])) same++;
    }

    float ratio = (float)same / ((lenA + lenB) / 2.0);
    if (ratio > 0.75 && diff <= 2) return 1;
    return 0;
}

//=========================================//

void OrderIDPlus1(char *newID) {
    FILE *file = fopen("tools.csv", "r");
    int maxID = 0;
    char line[256];

    if (file == NULL) {
        sprintf(newID, "T001");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        char id[10];
        sscanf(line, "%[^,]", id);
        if (id[0] == 'T') {
            int num = atoi(id + 1);
            if (num > maxID) maxID = num;
        }
    }
    fclose(file);

    sprintf(newID, "T%03d", maxID + 1);
}

//=========================================//

void AddOrder() {
    FILE *file = fopen("tools.csv", "r");
    FILE *temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        if (file) fclose(file);
        return;
    }

    char orderID[10];
    OrderIDPlus1(orderID);
    char Tools[100];
    int quantity, price;

    getchar();
    printf("Enter Tool Name: ");
    fgets(Tools, sizeof(Tools), stdin);
    Tools[strcspn(Tools, "\n")] = 0;
    if (strlen(Tools) == 0) {
        fclose(temp);
        if (file) fclose(file);
        remove("temp.csv");
        return;
    }

    char qtyStr[50];
    while (1) {
        printf("Enter Quantity: ");
        fgets(qtyStr, sizeof(qtyStr), stdin);
        qtyStr[strcspn(qtyStr, "\n")] = 0;
        int valid = 1;
        for (int i = 0; qtyStr[i] != '\0'; i++) {
            if (!isdigit(qtyStr[i])) {
                valid = 0;
                break;
            }
        }
        if (!valid || strlen(qtyStr) == 0) {
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        quantity = atoi(qtyStr);
        break;
    }

    if (quantity > 100) {
        char confirm;
        printf("Quantity %d is very high! Are you a millionaire ? (Y/N): ", quantity);
        scanf(" %c", &confirm);
        if (confirm != 'Y' && confirm != 'y') {
            fclose(temp);
            if (file) fclose(file);
            remove("temp.csv");
            return;
        }
    } else if (quantity <= 0) {
        fclose(temp);
        if (file) fclose(file);
        remove("temp.csv");
        return;
    }

    char priceStr[50];
    while (1) {
        printf("Enter Price per Unit: ");
        fgets(priceStr, sizeof(priceStr), stdin);
        priceStr[strcspn(priceStr, "\n")] = 0;
        int valid = 1;
        for (int i = 0; priceStr[i] != '\0'; i++) {
            if (!isdigit(priceStr[i])) {
                valid = 0;
                break;
            }
        }
        if (!valid || strlen(priceStr) == 0) {
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        price = atoi(priceStr);
        break;
    }

    if (price > 10000) {
        char confirm;
        printf("Price %d is very high! Want to get scammed ? (Y/N): ", price);
        scanf(" %c", &confirm);
        if (confirm != 'Y' && confirm != 'y') {
            fclose(temp);
            if (file) fclose(file);
            remove("temp.csv");
            return;
        }
    } else if (price <= 0) {
        fclose(temp);
        if (file) fclose(file);
        remove("temp.csv");
        return;
    }

    int found = 0, merge = 0, similarFound = 0;
    char existingID[10] = "", existingTool[100] = "";
    int oldQty = 0, oldPrice = 0;
    char similarID[10], similarTool[100];
    char line[256];

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            char id[10], tool[100];
            int qty, pr;
            sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &pr);
            if (strcmp(tool, Tools) == 0) {
                found = 1;
                strcpy(existingID, id);
                strcpy(existingTool, tool);
                oldQty = qty;
                oldPrice = pr;
            } else if (similarity(tool, Tools)) {
                similarFound = 1;
                strcpy(similarID, id);
                strcpy(similarTool, tool);
            }
            fprintf(temp, "%s,%s,%d,%d\n", id, tool, qty, pr);
        }
        fclose(file);
    }

    printf("\n=====================================\n");
    printf("REVIEW BEFORE ADDING ORDER\n");

    if (similarFound && !found) {
        printf("Did you mean '%s' instead of '%s'? (Y/N): ", similarTool, Tools);
        char ans;
        scanf(" %c", &ans);
        if (ans == 'Y' || ans == 'y') {
            strcpy(Tools, similarTool);
            strcpy(existingID, similarID);
            found = 1;
            file = fopen("tools.csv", "r");
            if (file != NULL) {
                while (fgets(line, sizeof(line), file)) {
                    char id[10], tool[100];
                    int qty, pr;
                    sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &pr);
                    if (strcmp(tool, Tools) == 0) {
                        oldQty = qty;
                        oldPrice = pr;
                        break;
                    }
                }
                fclose(file);
            }
        }
    }

    if (found) {
        printf("Found existing tool:\n");
        printf("[%s] %s | Old Qty: %d | Old Price: %d\n", existingID, existingTool, oldQty, oldPrice);
        printf("New input -> Qty: %d | Price: %d\n", quantity, price);
        printf("Do you want to merge (add quantity)? (Y/N): ");
        char ans;
        scanf(" %c", &ans);
        if (ans == 'Y' || ans == 'y') merge = 1;
    }

    printf("-------------------------------------\n");
    printf("This is what will be added:\n");
    if (found && merge) {
        printf("Update existing order (%s)\n", existingID);
        printf("New Qty: %d -> %d\n", oldQty, oldQty + quantity);
        printf("New Price: %d (replaced)\n", price);
    } else {
        printf("New Order\n");
        printf("Order ID: %s\n", orderID);
        printf("Tool Name: %s\n", Tools);
        printf("Quantity: %d\n", quantity);
        printf("Price per unit: %d\n", price);
        printf("Total: %d\n", quantity * price);
    }
    printf("-------------------------------------\n");
    printf("Confirm to save changes? (Y/N): ");
    char finalConfirm;
    scanf(" %c", &finalConfirm);
    if (finalConfirm != 'Y' && finalConfirm != 'y') {
        fclose(temp);
        remove("temp.csv");
        return;
    }

    fclose(temp);
    FILE *finalIn = fopen("temp.csv", "r");
    FILE *finalOut = fopen("tools.csv", "w");
    while (fgets(line, sizeof(line), finalIn)) {
        char id[10], tool[100];
        int qty, pr;
        sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &pr);
        if (found && strcmp(tool, Tools) == 0) {
            if (merge) {
                qty += quantity;
                pr = price;
                fprintf(finalOut, "%s,%s,%d,%d\n", id, tool, qty, pr);
            } else {
                fprintf(finalOut, "%s,%s,%d,%d\n", id, tool, qty, pr);
                fprintf(finalOut, "%s,%s,%d,%d\n", orderID, Tools, quantity, price);
            }
        } else {
            fprintf(finalOut, "%s,%s,%d,%d\n", id, tool, qty, pr);
        }
    }
    if (!found) {
        fprintf(finalOut, "%s,%s,%d,%d\n", orderID, Tools, quantity, price);
    }
    fclose(finalIn);
    fclose(finalOut);
    remove("temp.csv");
    if (found && merge)
        printf("Existing order updated (quantity added, price replaced)\n");
    else if (found && !merge)
        printf("Added as a new order alongside the old one\n");
    else
        printf("New order added: %s\n", orderID);
}


//=========================================//

void SearchOrder() {
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL) {
        printf("Cannot open file\n");
        return;
    }

    char search[100];
    printf("Enter Order ID or Tool Name to search: ");
    getchar();
    fgets(search, sizeof(search), stdin);
    search[strcspn(search, "\n")] = 0;

    if (strlen(search) == 0) {
        printf("Search cannot be empty.\n");
        fclose(file);
        return;
    }

    char line[256];
    int found = 0;
    char closestTool[100] = "";
    char closestID[10] = "";

    printf("\n=====================================\n");
    printf("Search Results for: '%s'\n", search);
    printf("-------------------------------------\n");

    while (fgets(line, sizeof(line), file)) {
        char id[10], tool[100];
        int qty, price;
        sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &price);

        if (strcasecmp(id, search) == 0 || strcasecmp(tool, search) == 0) {
            printf("Order ID: %s\n", id);
            printf("Tool: %s\n", tool);
            printf("Quantity: %d\n", qty);
            printf("Price: %d\n", price);
            printf("-------------------------------------\n");
            found = 1;
        } 
        else if (similarity(tool, search)) {
            strcpy(closestTool, tool);
            strcpy(closestID, id);
        }
    }

    fclose(file);

    if (!found && strlen(closestTool) > 0) {
        char confirm;
        printf("Did you mean '%s'? (Y/N): ", closestTool);
        scanf(" %c", &confirm);
        if (confirm == 'Y' || confirm == 'y') {
            file = fopen("tools.csv", "r");
            if (file != NULL) {
                while (fgets(line, sizeof(line), file)) {
                    char id[10], tool[100];
                    int qty, price;
                    sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &price);
                    if (strcmp(tool, closestTool) == 0) {
                        printf("\nOrder ID: %s\n", id);
                        printf("Tool: %s\n", tool);
                        printf("Quantity: %d\n", qty);
                        printf("Price: %d\n", price);
                        printf("-------------------------------------\n");
                        found = 1;
                        break;
                    }
                }
                fclose(file);
            }
        }
    }

    if (!found && strlen(closestTool) == 0) {
        printf("No matching or similar orders found.\n");
    }
}

//=========================================//

void DeleteOrder() {
    FILE *file = fopen("tools.csv", "r");
    if (file == NULL) {
        printf("Cannot open file\n");
        return;
    }

    FILE *temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        fclose(file);
        return;
    }

    char orderID[10];
    printf("Enter Order ID : ");
    scanf("%s", orderID);

    char line[256];
    int found = 0;
    char toolName[100];

    while (fgets(line, sizeof(line), file)) {
        char id[10], tool[100];
        int qty, price;
        sscanf(line, "%[^,],%[^,],%d,%d", id, tool, &qty, &price);
        if (strcmp(id, orderID) == 0) {
            found = 1;
            strcpy(toolName, tool);
            break;
        }
    }

    if (!found) {
        printf("Order %s not found.\n", orderID);
        fclose(file);
        fclose(temp);
        remove("temp.csv");
        return;
    }

    printf("\nYou are about to delete:\n");
    printf("Tool: %s\n", toolName);
    printf("Order ID: %s\n", orderID);

    char confirm1, confirm2;
    printf("Are you sure to delete this order? (Y/N): ");
    scanf(" %c", &confirm1);

    if (confirm1 != 'Y' && confirm1 != 'y') {
        printf("Delete canceled.\n");
        fclose(file);
        fclose(temp);
        remove("temp.csv");
        return;
    }

    printf("Please confirm again to delete '%s' (Y/N): ", toolName);
    scanf(" %c", &confirm2);

    if (confirm2 != 'Y' && confirm2 != 'y') {
        printf("Delete canceled.\n");
        fclose(file);
        fclose(temp);
        remove("temp.csv");
        return;
    }

    rewind(file);
    int deleted = 0;
    while (fgets(line, sizeof(line), file)) {
        char id[10];
        sscanf(line, "%[^,]", id);
        if (strcmp(id, orderID) == 0) {
            deleted = 1;
            continue;
        }
        fputs(line, temp);
    }

    fclose(file);
    fclose(temp);

    remove("tools.csv");
    rename("temp.csv", "tools.csv");

    if (deleted)
        printf("Order %s (%s) deleted successfully.\n", orderID, toolName);
    else
        printf("Order %s not found.\n", orderID);
}

//=========================================//

int Menu() {
    int menu;
    printf("\n===== Order management system =====\n");
    printf("1) Display all orders\n");
    printf("2) Add/Update new order\n");
    printf("3) Search for an order\n");
    printf("4) Delete an order\n");
    printf("5) Exit program\n");
    printf("Select an option (1-5): ");
    scanf("%d", &menu);
    return menu;
}

//=========================================//

int main() {
    int menu;
    do {
        menu = Menu();
        switch (menu) {
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
                DeleteOrder();
                break;
            case 5:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice\n");
        }
    } while (menu != 5);
    return 0;
}
