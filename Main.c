#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <windows.h>

int CEO_Comm;
int Employee_Count;

typedef struct Node {
    int age;
    int ID;
    char name[50];
    char position[50];
    int downLine;
    float commission;
    struct Node* left;
    struct Node* right;
} Node;


// Forward declarations
void writeFileHelper(FILE* file, Node* root, int depth);
Node* createNode(char name[], int age, int ID, char position[], int downLine, float commission);
Node* addToTree(Node* root, Node* newNode);
void displayTree(Node* root);
Node* removeNode(Node* root, int ID);
void updateDownlineAndCommission(Node* root);
void freeTree(Node* root);

// FILE HANDLING

// Save nodes to file
void writeFile(FILE* file, Node* root) {
    writeFileHelper(file, root, 0);
}

// Helper function, adds indentation
void writeFileHelper(FILE* file, Node* root, int depth) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        fprintf(file, "   "); 
    }

    fprintf(file, "Name: %s\tAge:%d\tID: %d\tPosition: %s\tDownline: %d\tCommission: %f\n", root->name, root->age, root->ID, root->position, root->downLine, root->commission);

    writeFileHelper(file, root->left, depth + 1);
    writeFileHelper(file, root->right, depth + 1);
}

Node* loadFromFileToTree(Node* root) {
    FILE* file = fopen("EmployeeData.txt", "r");
    if (file == NULL) {
        printf("Could not open file\n");
        return NULL;
    }

    int age, ID, downLine;
    char name[50], position[50];
    float commission;

    while (fscanf(file, "Name: %s\tAge:%d\tID: %d\tPosition: %s\tDownline: %d\tCommission: %f\n", name, &age, &ID, position, &downLine, &commission) == 6) {
        Node* newNode = createNode(name, age, ID, position, downLine, commission);
        root = addToTree(root, newNode);
    }

    fclose(file);
    return root;
}

void wipeData_fromFile(){
    FILE* file = fopen("EmployeeData.txt", "w");
    if (file == NULL) {
        printf("Could not open file\n");
        return;
    }

    file = fopen("Asset.txt", "w");
    if (file == NULL) {
        printf("Could not open file\n");
        return;
    }
    fclose(file);
}







// TREE OPERATIONS

// Add to tree
Node* addToTree(Node* root, Node* newNode) {
    if (root == NULL) {
        return newNode;
    }

    if (newNode->ID < root->ID) {
        root->left = addToTree(root->left, newNode);
    } else if (newNode->ID > root->ID) {
        root->right = addToTree(root->right, newNode);
    }

    updateDownlineAndCommission(root);

    return root;
}

// Create node
Node* createNode(char name[], int age, int ID, char position[], int downLine, float commission) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->age = age;
    newNode->ID = ID;
    strcpy(newNode->position, position);
    newNode->downLine = 0;
    newNode->commission = commission;
    newNode->left = NULL;
    newNode->right = NULL;

    CEO_Comm += 500;
    Employee_Count++;
    return newNode;
}

// Remove node
Node* removeNode(Node* root, int ID) {
    if (root == NULL) {
        return root;
    }

    if(ID == 0) {
        printf("You are the CEO. Why would you fire yourself?\n");
        getchar();
        return root;
    }

    if (ID < root->ID) {
        root->left = removeNode(root->left, ID);
    } else if (ID > root->ID) {
        root->right = removeNode(root->right, ID);
    } else {
        if (root->left == NULL) {
            Node* temp = root->right;
            CEO_Comm -= 500;
            Employee_Count--;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            CEO_Comm -= 500;
            Employee_Count--;
            free(root);
            return temp;
        }

        Node* temp = root->right;
        while (temp && temp->left != NULL) {
            temp = temp->left;
        }

        root->ID = temp->ID;
        root->right = removeNode(root->right, temp->ID);
    }

    updateDownlineAndCommission(root);

    return root;
}

// Update downline and commission based on downline pairs
void updateDownlineAndCommission(Node* root) {
    if (root != NULL) {
        int leftDownline = root->left ? root->left->downLine + 1 : 0;
        int rightDownline = root->right ? root->right->downLine + 1 : 0;
        root->downLine = leftDownline + rightDownline;
        root->commission = (root->downLine / 2) * 500; // Each pair gains a commission of 500
        if (root->left) updateDownlineAndCommission(root->left);
        if (root->right) updateDownlineAndCommission(root->right);
    }
}

// Display tree
void displayTree(Node* root) {
    if (root == NULL) {
        return;
    }

    printf("Name: %s\tAge: %d\tID: %d\tPosition: %s\tDownLine: %d\tCommission: %.2f\n", root->name, root->age, root->ID, root->position, root->downLine, root->commission);
    displayTree(root->left);
    displayTree(root->right);
}

// Free tree memory
void freeTree(Node* root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}





//MISCELLANEOUS
void clearScreen() {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #else
        system("clear");
    #endif
}

int ID_Generator(){
    static int id = 0;
    return id++;
}

void setAsset(){
    FILE* file = fopen("Asset.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "%d %d", CEO_Comm, Employee_Count);
    fclose(file);
    return;

}

void getAsset(){
    FILE* file = fopen("Asset.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }
    fscanf(file, "%d %d", &CEO_Comm, &Employee_Count);
    fclose(file);

    fclose(file);
    return;
}

//TEXT Formatting
void typeText(const char* text, int delay_ms) 
{
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        
        #ifdef _WIN32
            Sleep(delay_ms);  
        #else
            usleep(delay_ms * 1000); 
        #endif
    }
}

int main() {
    Node* root = NULL;
    root = loadFromFileToTree(root);
    getAsset();


    while (1) {
        int choice;

        typeText("WELCOME TO YOUR COMPANY SCHEME SIMULATION\n", 30);
        typeText("You are the CEO of the company.\nFor every employee that you Hire you gain a 500 dollars commission!\n\n",30);
        typeText("1. Check Employees\n2. Hire Employee\n3. Fire Employee\n4. Save File\n5. See Money\n6. Demolish Company?\n7. Exit\n",30);

        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch (choice) {
            case 1:
                displayTree(root);
                getchar();
                clearScreen();
                break;

            case 2: {
                char name[50], position[50];
                int age, ID, downLine;

                printf("Enter Name: ");
                scanf("%s", name);
                printf("Enter Age: ");
                scanf("%d", &age);
                printf("Enter Position: ");
                scanf("%s", position);

                ID = ID_Generator();
                Node* newNode = createNode(name, age, ID, position, downLine, 0);
                root = addToTree(root, newNode);
                clearScreen();
                break;
            }

            case 3: {
                int ID;
                printf("Enter ID to fire: ");
                scanf("%d", &ID);
                getchar();
                root = removeNode(root, ID);
                clearScreen();
                break;
            }

            case 4: {
                FILE* file = fopen("EmployeeData.txt", "w");
                if (file != NULL) {
                    writeFile(file, root);
                    setAsset();
                    printf("File saved successfully.\n");
                    fclose(file);
                    getchar();
                    clearScreen();

                } else {
                    printf("Error opening file for writing.\n");
                    getchar();
                    clearScreen();
                }
                break;
            }

            case 5:
                printf("Total Money Earned: %d\t\tTotal Employees Hired: %d",CEO_Comm,Employee_Count);
                getchar();
                clearScreen();
                break;

            case 6:
                while(1){
                    printf("You are about to wipe the entire database.\n\nPress [1] to confirm or [2] to cancel.\n");
                    int choice;scanf("%d",&choice);getchar();
                    if (choice == 1) {
                    root = NULL;
                    Employee_Count = 0;
                    CEO_Comm = 0;
                    wipeData_fromFile();
                    printf("Database wiped.\n");
                    getchar();
                    clearScreen();
                    break;
                    } else if (choice == 2) {
                    printf("Operation cancelled.\n");
                    getchar();
                    clearScreen();
                    break;
                    }
                }
                break;

            case 7: 
                freeTree(root);
                exit(0);
                break;
            default:
                printf("Invalid Choice!\n");
                break;
        }
    }

    return 0;
}