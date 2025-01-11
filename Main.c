#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h> // For Sleep on Windows
#else
#include <unistd.h>  // For usleep on Unix-based systems
#endif

#define HGRN "\e[0;92m"
#define HRED "\e[0;91m"
#define CYN "\e[0;36m"
#define MAG "\e[0;35m"
#define GOLD "\x1b[33m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define RED "\x1b[31m"
#define ITALIC_ON "\033[3m"
#define OFF "\033[0m"
#define BOLD_ON "\033[1m"
#define YELLOW "\e[0;33m"

#define SAGENT_THRESHOLD 3
#define LAGENT_THRESHOLD 5
#define SUP_THRESHOLD 7

int guideIndicator = 1;
int CEO_Comm = 0;
int Employee_Count = 0;
int textSpeed = 0;
char CompanyName[200];
int lastUsedID = 0; // Variable to track the last used ID


typedef struct Node {
    int age;
    int ID;
    char name[50];
    char position[50];
    int downLine;
    float commission;
    int parentID;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    int threshold;
    char position[50];
} ThresholdPosition;

typedef struct QueueNode {
    Node* treeNode;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode* front;
    QueueNode* rear;
} Queue;



// Forward declarations
void writeFileHelper(FILE* file, Node* root, int depth);
Node* createNode(char name[], int age, int ID, char position[], int downLine, float commission,int parentID);
Node* addToTree(Node* root, Node* newNode, int  ID);
void displayTree(Node* root);
Node* removeNode(Node* root, int ID);
void updateDownlineAndCommission(Node* root);
void freeTree(Node* root);
void clearScreen();
int ID_Generator();
void setAsset();
void getAsset();
void typeText(const char* text, int delay_ms);
void Hiring_Process(int age, char name[], Node* root, int ParentID);
bool isFileEmpty(const char* filename);
Node* loadFromFileToTree(Node* root);
void wipeData_fromFile(Node* root);
void enqueue(Queue* queue, Node* treeNode);
Node* dequeue(Queue* queue);
int isQueueEmpty(Queue* queue);
void displayTreeHelper(Node* root, int depth);
void bannerDisplay();
void writeFile(FILE* file, Node* root);
void endSentences();
void Hiring_Validation(int age, char name[], Node* root, int ParentID);
void updatePositions(Node* root, ThresholdPosition* thresholds, int numThresholds);
void traverseAndUpdate(Node* node, ThresholdPosition* thresholds, int numThresholds);
void checkAndUpdatePosition(Node* node, ThresholdPosition* thresholds, int numThresholds);




//FILE HANDLING
// Check if file is empty
bool isFileEmpty(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return false; // Indicate that the file could not be opened
    }

    // Move the file pointer to the end of the file
    fseek(file, 0, SEEK_END);

    // Get the position of the file pointer, which is the size of the file
    long fileSize = ftell(file);

    // Close the file
    fclose(file);

    return fileSize == 0;
}

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

    fprintf(file, "Name: %s\tAge:%d\tID: %d\tPosition: %s\tDownline: %d\tCommission: $%.2f\tParent ID: %d\n", root->name, root->age, root->ID, root->position, root->downLine, root->commission,root->parentID);

    writeFileHelper(file, root->left, depth + 1);
    writeFileHelper(file, root->right, depth + 1);
}


Node* loadFromFileToTree(Node* root) {
    FILE *file = fopen("EmployeeData.txt", "r");
    if (file == NULL) {
        printf("Could not open file\n");
        return NULL;
    }

    int age, ID, downLine, parentID;
    char name[50], position[50];
    float commission;

    while (fscanf(file, "Name: %49[^\t]\tAge:%d\tID: %d\tPosition: %49[^\t]\tDownline: %d\tCommission: %f\tParentID: %d\n",
               name, &age, &ID, position, &downLine, &commission, &parentID) == 7) { 
        Node* newNode = createNode(name, age, ID, position, downLine, commission, parentID);
        if (parentID == 0) {
            // If ParentID is 0, this is the root node
            root = newNode;
        } else {
            // Add the node to the tree based on ParentID
            root = addToTree(root, newNode, parentID);
        }
        if (ID > lastUsedID) {
            lastUsedID = ID; // Update lastUsedID to the maximum ID found
        }
    }

    fclose(file);
    return root;
}


void wipeData_fromFile(Node* root) {
    FILE* file = fopen("EmployeeData.txt", "w");
    if (file == NULL) {
        printf("Could not open file\n");
        return;
    }
    fclose(file);

    file = fopen("Asset.txt", "w");
    if (file == NULL) {
        printf("Could not open file\n");
        return;
    }
    fclose(file);

    root = NULL;
    CEO_Comm = 0;
    Employee_Count = 0;
}

// Sends the asset to the asset database
void setAsset(){
    FILE* file = fopen("Asset.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "%d %d %s", CEO_Comm, Employee_Count, CompanyName);
    fclose(file);
}

// Gets the data from the database to the system
void getAsset(){
    FILE* file = fopen("Asset.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fscanf(file, "%d %d %[^\n]", &CEO_Comm, &Employee_Count, CompanyName);
    fclose(file);
}





//BREADTH FIRST SEARCH OPERATIONS
void enqueue(Queue* queue, Node* treeNode) {
    // Allocate memory for a new queue node
    QueueNode* newQueueNode = (QueueNode*)malloc(sizeof(QueueNode));
    newQueueNode->treeNode = treeNode;  // Set the tree node as the data for this queue node
    newQueueNode->next = NULL;          // This new node will be the last, so its next pointer is NULL

    // Check if the queue is empty
    if (queue->rear == NULL) {
        // If the queue is empty, both front and rear will point to the new node
        queue->front = queue->rear = newQueueNode;
    } else {
        // If the queue is not empty, add the new node at the end of the queue
        queue->rear->next = newQueueNode;
        queue->rear = newQueueNode;  // Update the rear to point to the new node
    }
}

Node* dequeue(Queue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }
    QueueNode* temp = queue->front;
    Node* treeNode = temp->treeNode;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return treeNode;
}

int isQueueEmpty(Queue* queue) {
    return queue->front == NULL;
}






// TREE OPERATIONS

// Add to tree
Node* addToTree(Node* root, Node* newNode, int parentID) {
    if (root == NULL) {
        return newNode;
    }

    Queue queue = {NULL, NULL};
    enqueue(&queue, root);

    while (!isQueueEmpty(&queue)) {
        Node* current = dequeue(&queue);

        if (current->ID == parentID) {
            if (current->left == NULL) {
                current->left = newNode;
                return root;
            } else if (current->right == NULL) {
                current->right = newNode;
                return root;
            } else {
                printf("Node with ID %d already has two children.\n", parentID);
                return root;
            }
        }

        if (current->left != NULL) {
            enqueue(&queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(&queue, current->right);
        }
    }

    printf("Parent node with ID %d not found.\n", parentID);
    return root;
}


// Create node
Node* createNode(char name[], int age, int ID, char position[], int downLine, float commission, int parentID) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->age = age;
    newNode->ID = ID;
    strcpy(newNode->position, position);
    newNode->downLine = downLine; 
    newNode->commission = commission; 
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->parentID = parentID;

    CEO_Comm += 500;
    Employee_Count++;
    return newNode;
}


// Remove node
Node* removeNode(Node* root, int ID) {
    if (root == NULL) {
        printf(YELLOW ITALIC_ON"You don't have any employees to fire. So get to hiring already!"OFF);
        typeText(YELLOW"[ENTER] "OFF HGRN"Okay..."OFF, textSpeed);
        getchar();
        clearScreen();
        return root;
    }

    if (ID == 1) {
        typeText(YELLOW ITALIC_ON"You are the CEO. Why would you fire yourself?\n\n"OFF,textSpeed);
        typeText(YELLOW"[ENTER]"OFF HGRN" My bad..."OFF, 10);
        getchar();
        typeText(YELLOW ITALIC_ON"Think about what you've done..."OFF, 10);
        getchar();
        return root;
    }

    Node* targetNode = NULL;
    Node* parentOfTargetNode = NULL;
    Node* deepestNode = NULL;
    Node* parentOfDeepestNode = NULL;

    Queue queue = {NULL, NULL};
    enqueue(&queue, root);

    while (!isQueueEmpty(&queue)) {
        Node* current = dequeue(&queue);

        // Find the node to be deleted and track its parent
        if (current->ID == ID) {
            targetNode = current;
        }

        // Track the deepest node and its parent
        if (current->left != NULL) {
            parentOfDeepestNode = current;
            enqueue(&queue, current->left);
        }
        if (current->right != NULL) {
            parentOfDeepestNode = current;
            enqueue(&queue, current->right);
        }
        deepestNode = current;
    }

    // If the node to be removed is found, replace it with the deepest node
    if (targetNode != NULL) {
        targetNode->age = deepestNode->age;
        targetNode->ID = deepestNode->ID;
        strcpy(targetNode->name, deepestNode->name);
        strcpy(targetNode->position, deepestNode->position);
        targetNode->downLine = deepestNode->downLine;
        targetNode->commission = deepestNode->commission;
        targetNode->parentID = deepestNode->parentID;

        // Remove the deepest node
        if (parentOfDeepestNode->left == deepestNode) {
            parentOfDeepestNode->left = NULL;
        } else {
            parentOfDeepestNode->right = NULL;
        }

        free(deepestNode);
        typeText(YELLOW ITALIC_ON"It's always sad to fire an employee. Oh well, that's just life. Let's move on"OFF, textSpeed);
        getchar();
        clearScreen();
        CEO_Comm -= 500;
        Employee_Count--;
    } else {
        printf("Employee with ID %d not found.\n", ID);
    }

    updateDownlineAndCommission(root);
    return root;
}


// Display tree
void displayTreeHelper(Node* root, int depth) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("   "); // Add indentation
    }

    printf("Name: %s\t("CYN"%d, "OFF""MAG"%d, "OFF""RED"%s"OFF", %d, "GREEN"$%.2f,"OFF YELLOW" %d"OFF")\n",
           root->name, root->age, root->ID, root->position, root->downLine, root->commission);

    displayTreeHelper(root->left, depth + 1);
    displayTreeHelper(root->right, depth + 1);
}

// Display tree
void displayTree(Node* root) {
    displayTreeHelper(root, 0);
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



//FORMAT
void clearScreen() {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #else
        system("clear");
    #endif
}

void bannerDisplay(){
    double space = 81;
    double unRounded = (space - strlen(CompanyName)) / 2;
    int CompanyNamePlacement = ceil(unRounded);

    clearScreen();
    printf(BLUE"+"OFF);
    for(int i = 0; i < space; i++){
        printf(BLUE"="OFF);
    }
    printf(BLUE"+\n"OFF);
    

    
        for(int j = 0; j < 7; j++){
            printf(BLUE"|"OFF);

            if(j == 3){

                for(int i = 0; i < CompanyNamePlacement; i++){
                    printf(" ");
                }
                printf(BOLD_ON BLUE"%s"OFF, CompanyName);
                for(int i = 0; i < CompanyNamePlacement; i++){
                    printf(" ");
                }
            }else{
                for(int k = 0; k < space; k++){
                printf(" ");
                }
            }
            printf(BLUE"|\n"OFF);
        }
    

    printf(BLUE"+"OFF);
    for(int i = 0; i < space; i++){
        printf(BLUE"="OFF);
    }
    printf(BLUE"+\n\n\n"OFF);

}

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




// PROCESSES

// Generates ID for employees
int ID_Generator() {
    return ++lastUsedID; // Increment and return the next ID
}

void showVacantNodes(Node* node) {
    if (node == NULL) {
        return;
    }
    
    // Check if the current node has any vacant child
    if (node->left == NULL || node->right == NULL) {
        printf("%s ID: %d has unreferred referral/s.\n", node->name,node->ID);
    } else {
        printf("%s ID: %d has no referrals left.\n",node->name, node->ID);
    }

    // Recursively check left and right subtrees
    showVacantNodes(node->left);
    showVacantNodes(node->right);
}

// Hiring process
void Hiring_Process(int age, char name[], Node* root, int ParentID) {
    char position[50] = "Agent";
    int ID;

    while (1) {
        typeText(YELLOW"\n\n\n[1]"OFF" "GREEN"Hired!\n"OFF YELLOW"[2]"OFF RED" I think I'll pass.\n\n"OFF, textSpeed);
        int choice; 
        printf(YELLOW"Decision: "OFF);
        scanf("%d", &choice); 
        getchar();
        clearScreen();

        if (choice == 1) {
            typeText(YELLOW"Welcome to the Team "OFF, textSpeed);
            printf(BLUE"%s"OFF, name);
            typeText(YELLOW ITALIC_ON"\nDo your best to earn us money ;)\n"OFF, textSpeed);
            ID = ID_Generator(); // Generate new ID
            Node* newNode = createNode(name, age, ID, position, 0, 0, ParentID);
            root = addToTree(root, newNode, ParentID); // Use 0 as a placeholder, will be updated correctly in HireFromReferal
            getchar();
            clearScreen();
            return;

        } else if (choice == 2) {
            typeText(YELLOW ITALIC_ON"Awww thats a shame, better luck next time "OFF, textSpeed);
            printf(BLUE"%s"OFF, name);
            getchar();
            clearScreen();
            return;
        } else {
            typeText(HRED ITALIC_ON"There are only 2 choices. How can you mess that up?!"OFF, textSpeed);
            getchar();
            clearScreen();
        }
    }
}

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

// Function to update the positions based on the downline threshold
void updatePositions(Node* root, ThresholdPosition* thresholds, int numThresholds) {
    if (root != NULL) {
        if (root->left != NULL) traverseAndUpdate(root->left, thresholds, numThresholds);
        if (root->right != NULL) traverseAndUpdate(root->right, thresholds, numThresholds);
    }
}

// Helper function to traverse the tree and update positions
void traverseAndUpdate(Node* node, ThresholdPosition* thresholds, int numThresholds) {
    if (node == NULL) {
        return;
    }

    checkAndUpdatePosition(node, thresholds, numThresholds);

    traverseAndUpdate(node->left, thresholds, numThresholds);
    traverseAndUpdate(node->right, thresholds, numThresholds);
}

void checkAndUpdatePosition(Node* node, ThresholdPosition* thresholds, int numThresholds) {
    for (int i = 0; i < numThresholds; i++) {
        if (node->downLine >= thresholds[i].threshold) {
            strcpy(node->position, thresholds[i].position);
        }
    }
}



void endSentences(){
    getchar();
    clearScreen();
}

void guide(int option){
    switch(option){
        case 1:
            typeText(YELLOW"Check Employees\n"OFF"This section showcases the overall hierarchy of the company as well as details on each employee.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 2:
            typeText(YELLOW"Hire Employee\n" OFF"This is where the hiring process is located.\nThe hiring process includes 2 parts:\n1. The checking of vacant positions.\n2. Employee details input\n\t-Employee input must follow a format: \n\t\tName = \"Full name excluding middle name\"\n\t\tAge = \"age\"\nInput [-1] on employee ID to return.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 3:
            typeText(YELLOW"Fire Employee\n"OFF"This section is where the firing process happens.\nIt asks for an employee's ID number and searches for it in the binary tree then removes it.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 4:
            typeText(YELLOW"Save File\n"OFF "Manually saves the file to the current directory.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 5:
            typeText(YELLOW"Pocket Money\n"OFF"This is where the separate commission and total employee hired displays.\nThis commission is earned through "YELLOW"Hiring."OFF"\neverytime you hire an employee, you gain a $500 commission.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 6:
            typeText(YELLOW"Demolish Company\n"OFF"This is where the company is completely deleted.\nThis is irreversible.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 7:
            typeText(YELLOW"Settings\n"OFF"This section contains the customizable aspects of the program.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
        case 8:
            typeText(YELLOW"Clock Out\n"OFF"This section is where the user can clock out and end their session.\n\n\n"YELLOW"[ENTER] to continue.."OFF,textSpeed);
            endSentences();
            break;
    }
}


void Hiring_Validation(int age, char name[], Node* root, int ParentID){
    printf("Hire Employee\n\n");
                typeText(YELLOW ITALIC_ON"Hmmmm "OFF,10);
                printf(BLUE"%d"OFF,age);
                typeText(YELLOW ITALIC_ON" huh. "OFF,10);
                if (age < 25 && age > 17){
                    typeText(YELLOW ITALIC_ON"This person seems to be young, you know.",textSpeed);
                    typeText("\nMight lack experience. But we might hit a gold mine with this one. \nThis kid may be full of potential!"OFF,textSpeed);
                    
                    Hiring_Process(age, name, root, ParentID);
                } else if (age > 45 && age < 60){
                    typeText(YELLOW ITALIC_ON"Well, this person sure isn't going to be happy with being an "OFF HRED"agent"OFF YELLOW ITALIC_ON"\nRickety knees might be the cause of their death.\nBut you know as they say, \"With age, comes wisdom and experience\""OFF,textSpeed);
                    Hiring_Process(age, name, root, ParentID);
                } else if (age >= 60){
                    typeText(YELLOW ITALIC_ON"I trust that you have the common sense to know that we are not running a nursing home. Let's just send this old person to their home so we can move on with our lives\n\n\n"OFF,textSpeed);
                    typeText(YELLOW"[ENTER] "OFF HGRN"I know, I know..."OFF,textSpeed);
                    endSentences();
                    return;
                } else if (age <= 17){
                    typeText(YELLOW ITALIC_ON"Send this child home right this instant. We can't have children working here. We don't want to be invloved in some unsightly scandal"OFF,textSpeed);
                    typeText(YELLOW"[ENTER]"OFF HGRN" Yeah, it would be best to stay away from this one..."OFF,textSpeed);
                    endSentences();
                    return;
                } else {
                    typeText(YELLOW ITALIC_ON"This seems to be an appropriate age to hire. So, why not right?"OFF,textSpeed);
                    
                    Hiring_Process(age, name, root, ParentID);
                }
}


void HireFromReferal(Node* root, int ParentID, char name[], int age) {
    if (root == NULL) {
        return;
    }

    if (root->ID == ParentID) {
        printf("Hiring from referral of %s: %s ID number: %d\n", root->position, root->name, root->ID);
        if (root->left == NULL || root->right == NULL) {
            Hiring_Validation(age, name, root, ParentID);
            return;
        } else {
            typeText("This person already used up their referrals..\n\n\n[ENTER] to return...", textSpeed);
            endSentences();
            return;
        }
    }

    HireFromReferal(root->left, ParentID, name, age);
    HireFromReferal(root->right, ParentID, name, age);
}

void IntroProcess(){
    clearScreen();
        typeText(RED"Finalizing Contracts",textSpeed);Sleep(500);printf(".");Sleep(500);printf(".");Sleep(500);printf("."OFF);Sleep(500);
        clearScreen();
        typeText(HGRN"Contracts Finalized!"OFF,textSpeed);Sleep(500);
        clearScreen();
        typeText(RED"Building Company",textSpeed);Sleep(500);printf(".");Sleep(500);printf(".");Sleep(500);printf("."OFF);Sleep(500);
        clearScreen();
        typeText(HGRN"Company Built!"OFF,textSpeed);Sleep(500);
        clearScreen();
        typeText(RED"Booting Assistant",textSpeed);Sleep(500);printf(".");Sleep(500);printf(".");Sleep(500);printf("."OFF);Sleep(500);
        clearScreen();
        typeText(HGRN"Assistant Booted!"OFF,textSpeed);Sleep(500);
        clearScreen();

        typeText(YELLOW"Company Successfully created!\n\n\n\n[ENTER] to continue..."OFF,textSpeed);
        getchar();
        
        clearScreen();

        typeText(YELLOW ITALIC_ON"\t\tWELCOME TO YOUR COMPANY SCHEME SIMULATION\n", textSpeed);
        typeText("\t\t     You are the CEO of the company.\n   For every employee that you hire, you gain 500 dollars as pocket money! ;)\n\n[ENTER] to continue..."OFF, textSpeed);
        getchar(); clearScreen();

}

bool nameCheck( Node* root, char name[]){
    Node* current = root;
    if(current == NULL){
        return false;
    }

    if(strcasecmp(current->name, name) == 0){
        return true;
    }

    return nameCheck(root->left, name) || nameCheck(root->right, name);
}

void setting(){
    while(1){
                    printf("Settings\n\n");
                    typeText(YELLOW ITALIC_ON"What do you want to customize\n\n"OFF,textSpeed);
                    
                    typeText(YELLOW "[1] Text Animation.\n[2] Guides.\n[3] Return\n\n\nDecision: "OFF,textSpeed);
                    int choice; scanf("%d", &choice); 
                    endSentences();

                    if(choice == 1){
                        printf("Settings -> Text Animation\n\n");
                        if(textSpeed != 0){
                            typeText(YELLOW ITALIC_ON"Do you want to remove text Animation?\n\n\n[1]"OFF RED" Yeah, it's getting kind of annoying\n"OFF YELLOW"[2]"OFF HGRN" No, I changed my mind"OFF YELLOW"\n\n Decision: "OFF,textSpeed);
                            int choice2; scanf("%d", &choice2);
                            clearScreen();
                            if (choice2 == 1) {
                            clearScreen();
                            typeText(YELLOW ITALIC_ON"It took quite a lot of effort to make it look good you know. \nBut oh well, \"The user is always right\" I guess.\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            getchar();
                            textSpeed = 0;
                            clearScreen();
                            break;
                            }
                            else if (choice2 == 2) {
                            typeText(YELLOW ITALIC_ON"You've got good taste boss! You truly have an eye for aesthetics!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            getchar();
                            clearScreen();
                            break;
                            } else {
                            clearScreen();
                            typeText(YELLOW ITALIC_ON"There literally only 2 choices."OFF HRED" TWO CHOICES!"OFF YELLOW ITALIC_ON" How can you mess that up!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            getchar();
                            clearScreen;
                            }
                        }else{
                            typeText(YELLOW ITALIC_ON"Oh, you want the animations back? Changed your mind? Kind of an indecisive person aren't you.\n\n\n"OFF YELLOW"[1]"HGRN" Yeah, so what?!\n"OFF YELLOW"[2]"OFF RED" Well, maybe you're right!"OFF YELLOW"\n\n\nDecision: "OFF,textSpeed);
                            int choice; scanf("%d", &choice); getchar();
                            if (choice == 1) {
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"Good for you, I guess...\n\n\n"OFF YELLOW"[ENTER]"HGRN" ..."OFF,textSpeed);
                                textSpeed = 15;
                                getchar();
                                clearScreen();
                                break;
                            }
                            else if (choice == 2){
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"Okay then, I suppose...\n\n\n"OFF YELLOW"[ENTER]"HGRN" ..."OFF,textSpeed);
                                getchar();
                                clearScreen();
                                break;
                            } else {
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"There literally only 2 choices."OFF HRED" TWO CHOICES!"OFF YELLOW ITALIC_ON" How can you mess that up!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                                getchar();
                                clearScreen;
                            }
                        }
                        
                    }
                    if(choice == 2){
                        printf("Settings -> Guides");
                        if(guideIndicator != 0){
                            typeText(YELLOW ITALIC_ON"\n\n\nDo you want to remove guides?"OFF"\n\n\n"YELLOW"[1]"OFF RED"Yeah, I know it all."OFF YELLOW"\n[2]"OFF HGRN" No, I changed my mind."OFF YELLOW"\n\n Decision: "OFF,textSpeed);
                            int choice2; scanf("%d", &choice2);
                            if (choice2 == 1) {
                            clearScreen();
                            typeText(YELLOW ITALIC_ON"You do you, I guess. Guides don't seem to be that harmful, in fact, its quite helpful.\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            
                            guideIndicator = 0;
                            endSentences();
                            break;
                            }
                            else if (choice2 == 2) {
                            typeText(YELLOW ITALIC_ON"Now, that is some wise thinking. Guides are helpful. Make you not forget about instructions!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            endSentences();
                            break;
                            } else {
                            clearScreen();
                            typeText(YELLOW ITALIC_ON"There literally only 2 choices."OFF HRED" TWO CHOICES!"OFF YELLOW ITALIC_ON" How can you mess that up!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                            endSentences();
                            }
                        }else{
                            typeText(YELLOW ITALIC_ON"Oh you want the guides back? Changed your mind? Realized the usefulness of guides?\n\n\n"OFF YELLOW"[1]"HGRN" Yeah, you were right\n"OFF YELLOW"[2]"OFF RED" Nope, still turning it off"OFF YELLOW"\n\n\nDecision: "OFF,textSpeed);
                            int choice; scanf("%d", &choice); getchar();
                            clearScreen();
                            if (choice == 1) {
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"Guides are nice, aren't they.\n\n\n"OFF YELLOW"[ENTER]"HGRN" ..."OFF,textSpeed);
                                guideIndicator = 1;
                                endSentences();
                                break;
                            }
                            else if (choice == 2){
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"Okay then, I suppose...\n\n\n"OFF YELLOW"[ENTER]"HGRN" ..."OFF,textSpeed);
                                endSentences();
                                break;
                            } else {
                                clearScreen();
                                typeText(YELLOW ITALIC_ON"There literally only 2 choices."OFF HRED" TWO CHOICES!"OFF YELLOW ITALIC_ON" How can you mess that up!\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                                endSentences();
                            }
                        }
                        
                    }
                    if(choice == 3){
                        return;
                    }
                    else{
                        printf(YELLOW ITALIC_ON"Please choose something from the choices. Not from anywhere else\n\n\n[ENTER]"OFF HGRN" ..."OFF);
                        endSentences();
                    }
                    
                        
                }

}



int main() {
    Node* root = NULL;
    root = loadFromFileToTree(root);
    getAsset();

    ThresholdPosition thresholds[] = {
        {SAGENT_THRESHOLD, "Senior Agent"},
        {LAGENT_THRESHOLD, "Lead Agent"},
        {SUP_THRESHOLD, "Supervisor"},
    };

    if (isFileEmpty("EmployeeData.txt")) {
        printf("Chief Executive Officer information input section:\n");
        printf("Enter CEO Full Name: ");
        char name[50]; scanf("%[^\n]", name); while (getchar() != '\n'); 
        printf("Enter CEO Age: ");
        int age; scanf("%d", &age); while (getchar() != '\n'); 
        printf("Enter Company name: ");
        scanf("%[^\n]", CompanyName); while (getchar() != '\n'); 

        char position[50] = "Chief Executive Officer";
        int downLine = 0;
        int ID = ID_Generator();
        Node* newNode = createNode(name, age, ID, position, downLine, 0, 0);
        root = addToTree(root, newNode, ID);
        IntroProcess();
        
    } else if(isFileEmpty("EmployeeData.txt") != 0){
        textSpeed = 15;
    }

    while (1) {
        int choice;
        
        bannerDisplay();
        typeText(YELLOW ITALIC_ON"What would you like to do today boss?\n\n"OFF,textSpeed);
        typeText(YELLOW"[1]. Check Employees\n[2]. Hire Employee\n[3]. Fire Employee\n[4]. Save File\n[5]. Pocket Money ;)\n[6]. Demolish Company?\n[7]. Settings\n[8]. Clock Out\n\n\nEnter your choice: "OFF, textSpeed);

        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch (choice) {
            case 1:
                if(guideIndicator == 1){guide(1);}
                updatePositions(root, thresholds, sizeof(thresholds) / sizeof(thresholds[0]));
                clearScreen();
                printf("Check Employees\n\n");
                typeText(GOLD"Legend:"OFF"\n\nName: \t("BLUE"Age,"OFF""MAG" ID number,"OFF""RED" Position,"OFF" Downline, "GREEN"Commission"OFF")\n\n\n",textSpeed);
                displayTree(root);
                typeText(YELLOW"[ENTER] to return..."OFF,textSpeed);
                getchar();
                clearScreen();
                break;

            case 2: {
                if(guideIndicator == 1){guide(2);}
                char name[50];
                int age;

                printf("Hire Employee\n\n");
                showVacantNodes(root);
                
                typeText("\n\n\nEnter Referal Employee ID: ",textSpeed);
                int ParentID; scanf("%d", &ParentID); getchar();

                if(ParentID == -1){
                    break;
                }
                typeText(YELLOW"\nThe applicants you will hire is assigned the position of "OFF""HRED"Agent\n"OFF, textSpeed);
                printf("Enter Applicant's Full Name: ");
                fgets(name, sizeof(name), stdin);

                
                if(nameCheck(root, name) == true){
                    printf("Employee already exists\n");
                    endSentences();
                    break;
                }
                
                printf("Enter Applicant's Age: ");
                scanf("%d", &age);getchar();
                clearScreen();
                HireFromReferal(root, ParentID, name, age);

                clearScreen();
                
                break;
            }

            case 3: {
                if(guideIndicator == 1){guide(3);}
                int ID;
                printf("Fire Employee\n\n");
                typeText(HRED ITALIC_ON"Uh oh, looks like someones going to have a bad day.\n\n\n"OFF""YELLOW"(Enter [-1] to cancel)\n",textSpeed);
                printf("Enter ID to fire: "OFF);
                scanf("%d", &ID); while (getchar() != '\n'); 

                if(ID == -1){
                    typeText(YELLOW ITALIC_ON"\n\nYou should've looked at the employee list before coming here. Tsk tsk tsk"OFF,textSpeed);
                    getchar();
                    clearScreen();
                    break;
                }
                root = removeNode(root, ID);
                
                clearScreen();
                break;
            }

            case 4: {
                if(guideIndicator == 1){guide(4);}
                printf("Save File\n\n");
                FILE* file = fopen("EmployeeData.txt", "w");
                if (file != NULL) {
                    updatePositions(root, thresholds, sizeof(thresholds) / sizeof(thresholds[0]));
                    writeFile(file, root);
                    setAsset();
                    
                    typeText(GREEN"Saving employee data...\n",textSpeed);
                    Sleep(500);
                    typeText("Saving assets data...\n"OFF,textSpeed);
                    
                    Sleep(500);
                    clearScreen();
                    printf("Save File\n\n");
                    typeText(GREEN"Employee data has been saved to EmployeeData.txt successfully\nAssets data has been saved to Asset.txt successfully!\n\n\n"OFF,textSpeed);
                    typeText("[ENTER]",textSpeed);
                    getchar();
                    typeText(YELLOW ITALIC_ON"Well, that sounds reassuring doesn't it? Let's just hope that no one tries to hack into our system"OFF"\n\n"YELLOW"[ENTER] to continue..."OFF,textSpeed);
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
                if(guideIndicator == 1){guide(5);}
                printf("See Pocketed Money\n\n");
                typeText(YELLOW"Its always nice to see progress in our work\n\n",textSpeed);
                printf("Total Money Pocketed:"OFF""GREEN" $%d"OFF"\t\t"YELLOW"Total Employees Hired:"OFF" "RED"%d\n"OFF, CEO_Comm, Employee_Count);
                typeText("\n\n\n[ENTER] to return...",textSpeed);
                getchar();
                clearScreen();
                break;

            case 6:
                if(guideIndicator == 1){guide(6);}
                while (1) {
                    printf("Demolish Company\n\n");
                    typeText(YELLOW"You are about to destroy everything in this company. \nAre you sure you want to go through with this?\n\n[1]"OFF""HRED" Lets burn it down!"OFF""YELLOW"\n[2]"OFF""HGRN" I think I'm having second thoughts.\n\n"OFF YELLOW"Decision: "OFF,textSpeed);
                    int choice; scanf("%d", &choice); getchar(); clearScreen();

                    if (choice == 1) {

                        wipeData_fromFile(root);
                        typeText(HRED"*the whole company is now covered in blazing flames*"OFF""YELLOW ITALIC_ON"\n\nWell boss, it was fun working with you.",textSpeed);
                        for(int i = 0; i<2; i++){
                            printf(". ");
                            Sleep(500);
                        }
                        clearScreen();
                        printf(YELLOW"[ENTER] "OFF HGRN"..."OFF);getchar();

                        typeText("Goodbye boss",textSpeed);
                        for(int i = 0; i<2; i++){
                            printf(". ");
                            Sleep(500);
                        }
                        
                        Sleep(1000);
                        typeText(RED"\n\n\nSimulation Terminated."OFF,textSpeed);
                        exit(0);

                        break;
                    } else if (choice == 2) {
                        typeText(YELLOW ITALIC_ON"If you're having second thoughts then maybe we should NOT think about destroying everything we've built, okay?\nMaybe we'll do it some other day, when you're more decisive in your life.\n\n\n"OFF YELLOW"[ENTER] "OFF HGRN"..."OFF,textSpeed);
                        endSentences();
                        break;
                    }
                }
                break;

            case 7:
                if(guideIndicator == 1){guide(7);}
                setting();
                break;
            case 8:
                if(guideIndicator == 1){guide(8);}
                printf("Clock Out\n\n");
                typeText(YELLOW ITALIC_ON"Before clocking out, are you sure you "OFF HGRN"saved your work"OFF YELLOW ITALIC_ON" today?\nNow I'm not accusing you of being incompetent or whatever but people tend to forget this kind of thing.\nSo yeah, just check it.\nIt doesn't hurt to check you know\n\n"OFF,textSpeed);
                Sleep(1000);
                typeText(YELLOW"[1]"OFF""RED" Of course I saved my work, I'm not an idiot."OFF"\n"YELLOW"[2]"OFF" "GREEN"Yeah, I should probably check just to be sure.\n\n"OFF,textSpeed);
                int choice; scanf("%d", &choice); getchar();
                clearScreen();
                if (choice == 1) {
                    typeText(YELLOW ITALIC_ON"Good work today boss!, Let's do our best again tomorrow and beyond!"OFF,textSpeed);
                    getchar();
                    freeTree(root);
                    exit(0);
                    }
                else if (choice == 2) {break;}
                
                break;
            default:
                clearScreen();
                typeText(YELLOW ITALIC_ON"There are choices provided for you, you know.\n\nI know, that you know, what you're supposed to do.\n\n\n"OFF,textSpeed);
                typeText(GREEN"[ENTER] ..."OFF,textSpeed);
                endSentences();
                break;
        }
    }

    return 0;
}