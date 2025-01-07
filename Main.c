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

int CEO_Comm = 0;
int Employee_Count = 0;
int textSpeed = 25;
char CompanyName[200];
int lastUsedID = 0; // Variable to track the last used ID


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
Node* createNode(char name[], int age, int ID, char position[], int downLine, float commission);
Node* addToTree(Node* root, Node* newNode);
void displayTree(Node* root);
Node* removeNode(Node* root, int ID);
void updateDownlineAndCommission(Node* root);
void freeTree(Node* root);
void clearScreen();
int ID_Generator();
void setAsset();
void getAsset();
void typeText(const char* text, int delay_ms);
void Hiring_Process(int age, char name[], Node* root);
bool isFileEmpty(const char* filename);
Node* loadFromFileToTree(Node* root);
void wipeData_fromFile(Node* root);
void enqueue(Queue* queue, Node* treeNode);
Node* dequeue(Queue* queue);
int isQueueEmpty(Queue* queue);
void displayTreeHelper(Node* root, int depth);
void bannerDisplay();
void writeFile(FILE* file, Node* root);



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

    fprintf(file, "Name: %s\tAge:%d\tID: %d\tPosition: %s\tDownline: %d\tCommission: %f\n", root->name, root->age, root->ID, root->position, root->downLine, root->commission);

    writeFileHelper(file, root->left, depth + 1);
    writeFileHelper(file, root->right, depth + 1);
}


Node* loadFromFileToTree(Node* root) {
    FILE *file = fopen("EmployeeData.txt", "r");
    if (file == NULL) {
        printf("Could not open file\n");
        return NULL;
    }

    int age, ID, downLine;
    char name[50], position[50];
    float commission;

    while (fscanf(file, "Name: %49[^\t]\tAge:%d\tID: %d\tPosition: %49[^\t]\tDownline: %d\tCommission: %f\n",
                  name, &age, &ID, position, &downLine, &commission) == 6) {
        Node* newNode = createNode(name, age, ID, position, downLine, commission);
        root = addToTree(root, newNode);
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
Node* addToTree(Node* root, Node* newNode) {
    if (root == NULL) {
        return newNode;
    }

    Queue queue = {NULL, NULL}; //declaration of queue {left, right}
    enqueue(&queue, root); //(queue, treeNode)

    while (!isQueueEmpty(&queue)) {
        Node* current = dequeue(&queue);

        // Check if the current node can have a left child
        if (current->left == NULL) {
            current->left = newNode;
            break;
        } else {
            enqueue(&queue, current->left);
        }

        // Check if the current node can have a right child
        if (current->right == NULL) {
            current->right = newNode;
            break;
        } else {
            enqueue(&queue, current->right);
        }
    }

    // Update downline and commission for the entire tree
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
    newNode->downLine = downLine; 
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

    printf("Name: %s\t("CYN"%d, "OFF""MAG"%d, "OFF""RED"%s"OFF", %d, "GREEN"$%.2f"OFF")\n",
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

// Hiring process
void Hiring_Process(int age, char name[], Node* root){
    while(1){
        char position[50] = "Agent";
        int ID;

        typeText(YELLOW"\n\n\n[1]"OFF" "GREEN"Hired!\n"OFF YELLOW"[2]"OFF RED" I think I'll pass.\n\n"OFF,textSpeed);
        int choice; 
        printf(YELLOW"Decision: "OFF);
        scanf("%d", &choice); 
        getchar();
        clearScreen();

        if(choice == 1){
            typeText(YELLOW"Welcome to the Team "OFF,textSpeed);
            printf(BLUE"%s"OFF, name);
            typeText(YELLOW ITALIC_ON"\nDo your best to earn us money ;)\n"OFF,textSpeed);
            ID = ID_Generator(); // Generate new ID
            Node* newNode = createNode(name, age, ID, position, 0, 0);
            root = addToTree(root, newNode);
            getchar();
            clearScreen();
            return;

        } else if(choice == 2){
            typeText(YELLOW ITALIC_ON"Awww thats a shame, better luck next time "OFF, textSpeed);
            printf(BLUE"%s"OFF, name);
            getchar();
            clearScreen();
            return;
        } else {
            typeText(HRED ITALIC_ON"There are only 2 choices. How can you mess that up?!"OFF,textSpeed);
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







int main() {
    Node* root = NULL;
    root = loadFromFileToTree(root);
    getAsset();

    if (isFileEmpty("EmployeeData.txt")) {
        printf("Enter Name: ");
        char name[50]; scanf("%[^\n]", name); while (getchar() != '\n'); 
        printf("Enter Age: ");
        int age; scanf("%d", &age); while (getchar() != '\n'); 
        printf("Enter Company name: ");
        scanf("%[^\n]", CompanyName); while (getchar() != '\n'); 

        char position[50] = "Chief Executive Officer";
        int downLine = 0;
        int ID = ID_Generator();
        Node* newNode = createNode(name, age, ID, position, downLine, 0);
        root = addToTree(root, newNode);

        clearScreen();
        typeText(RED"Finalizing Contracts",textSpeed);Sleep(1000);printf(".");Sleep(1000);printf(".");Sleep(1000);printf("."OFF);Sleep(1000);
        clearScreen();
        typeText(HGRN"Contracts Finalized!"OFF,textSpeed);Sleep(1000);
        clearScreen();
        typeText(RED"Building Company",textSpeed);Sleep(1000);printf(".");Sleep(1000);printf(".");Sleep(1000);printf("."OFF);Sleep(1000);
        clearScreen();
        typeText(HGRN"Company Built!"OFF,textSpeed);Sleep(1000);
        clearScreen();
        typeText(RED"Booting Assistant",textSpeed);Sleep(1000);printf(".");Sleep(1000);printf(".");Sleep(1000);printf("."OFF);Sleep(1000);
        clearScreen();
        typeText(HGRN"Assistant Booted!"OFF,textSpeed);Sleep(1000);
        clearScreen();

        typeText(YELLOW"Company Successfully created!\n\n\n\n[ENTER] to continue..."OFF,textSpeed);
        getchar();
        
        clearScreen();

        typeText(YELLOW ITALIC_ON"\t\tWELCOME TO YOUR COMPANY SCHEME SIMULATION\n", textSpeed);
        typeText("\t\t     You are the CEO of the company.\n   For every employee that you hire, you gain 500 dollars as pocket money! ;)\n\n"OFF, textSpeed);
        getchar(); clearScreen();

    } else if(isFileEmpty("EmployeeData.txt") != 0){
        textSpeed = 15;
    }

    while (1) {
        int choice;

        bannerDisplay();
        typeText(YELLOW ITALIC_ON"What would you like to do today boss?\n\n"OFF,textSpeed);
        typeText(YELLOW"[1]. Check Employees\n[2]. Hire Employee\n[3]. Fire Employee\n[4]. Save File\n[5]. Pocket Money ;)\n[6]. Demolish Company?\n[7]. Text Animation\n[8]. Clock Out\n\n\nEnter your choice: "OFF, textSpeed);

        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch (choice) {
            case 1:
                typeText(GOLD"Legend:"OFF"\n\nName: \t("BLUE"Age,"OFF""MAG" ID number,"OFF""RED" Position,"OFF" Downline, "GREEN"Commission"OFF")\n\n\n",textSpeed);
                displayTree(root);
                typeText(YELLOW"[ENTER] to return..."OFF,textSpeed);
                getchar();
                clearScreen();
                break;

            case 2: {
                char name[50];
                int age;

                typeText(YELLOW"The applicants you will hire will be assigned the position of "OFF""HRED"Agent\n"OFF, textSpeed);
                printf("Enter Name: ");
                scanf("%[^\n]", name);getchar();
                printf("Enter Age: ");
                scanf("%d", &age);getchar();

                clearScreen();
                typeText(YELLOW ITALIC_ON"Hmmmm "OFF,10);
                printf(BLUE"%d"OFF,age);
                typeText(YELLOW ITALIC_ON" huh. "OFF,10);
                if (age < 25 && age > 17){
                    typeText(YELLOW ITALIC_ON"This person seems to be young, you know.",textSpeed);
                    typeText("\nMight lack experience. But we might hit a gold mine with this one. \nThis kid may be full of potential!"OFF,textSpeed);
                    
                    Hiring_Process(age, name, root);
                } else if (age > 45 && age < 60){
                    typeText(YELLOW ITALIC_ON"Well, this person sure isn't going to be happy with being an "OFF HRED"agent"OFF YELLOW ITALIC_ON"\nRickety knees might be the cause of their death.\nBut you know as they say, \"With age, comes wisdom and experience\""OFF,textSpeed);
                    Hiring_Process(age, name, root);
                } else if (age >= 60){
                    typeText(YELLOW ITALIC_ON"I trust that you have the common sense to know that we are not running a nursing home. Let's just send this old person to their home so we can move on with our lives\n\n\n"OFF,textSpeed);
                    typeText(YELLOW"[ENTER] "OFF HGRN"I know, I know..."OFF,textSpeed);
                    getchar();
                    clearScreen();
                    break;
                } else if (age <= 17){
                    typeText(YELLOW ITALIC_ON"Send this child home right this instant. We can't have children working here. We don't want to be invloved in some unsightly scandal"OFF,textSpeed);
                    typeText(YELLOW"[ENTER]"OFF HGRN" Yeah, it would be best to stay away from this one..."OFF,textSpeed);
                    getchar();
                    clearScreen();
                    break;
                } else {
                    typeText(YELLOW ITALIC_ON"This seems to be an appropriate age to hire. So, why not right?"OFF,textSpeed);
                    
                    Hiring_Process(age, name, root);
                }

                
                break;
            }

            case 3: {
                int ID;
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
                FILE* file = fopen("EmployeeData.txt", "w");
                if (file != NULL) {
                    writeFile(file, root);
                    setAsset();
                    typeText(GREEN"Saving employee data...\n",textSpeed);
                    Sleep(1000);
                    typeText("Saving assets data...\n",textSpeed);
                    Sleep(1000);
                    clearScreen();
                    typeText("Employee data has been saved to EmployeeData.txt successfully\nAssets data has been saved to Asset.txt successfully!\n\n\n"OFF,textSpeed);
                    typeText("[ENTER] to continue...",textSpeed);
                    getchar();
                    typeText(YELLOW ITALIC_ON"Well now, that sounds reassuring doesn't it? Let's just hope that no one tries to hack into our system"OFF"\n\n"YELLOW"[ENTER] to continue..."OFF,textSpeed);
                    textSpeed = 15;
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
                typeText(YELLOW"Its always nice to see progress in our work\n\n",textSpeed);
                printf("Total Money Pocketed:"OFF""GREEN" $%d"OFF"\t\t"YELLOW"Total Employees Hired:"OFF" "RED"%d\n"OFF, CEO_Comm, Employee_Count);
                typeText("\n\n\n[ENTER] to return...",textSpeed);
                getchar();
                clearScreen();
                break;

            case 6:
                while (1) {
                    typeText(YELLOW"You are about to destroy everything in this company. \nAre you sure you want to go through with this?\n\n[1]"OFF""HRED" Lets burn it down!"OFF""YELLOW"\n[2]"OFF""HGRN" I think I'm having second thoughts.\n\n"OFF YELLOW"Decision: "OFF,textSpeed);
                    int choice; scanf("%d", &choice); getchar(); clearScreen();
                    if (choice == 1) {

                        wipeData_fromFile(root);
                        typeText(HRED"*the whole company is now covered in blazing flames*"OFF""YELLOW ITALIC_ON"\n\nWell boss, it was fun working with you.",textSpeed);
                        Sleep(1000);
                        printf(". ");
                        Sleep(1000);
                        printf(". ");
                        Sleep(1000);
                        printf(". ");
                        Sleep(1000);
                        clearScreen();

                        typeText("Goodbye boss",textSpeed);
                        printf(". ");
                        Sleep(1000);
                        printf(". ");
                        Sleep(1000);
                        printf(". \n\n\n"OFF);
                        Sleep(1500);
                        typeText(RED"Simulation Terminated."OFF,textSpeed);
                        exit(0);

                        break;
                    } else if (choice == 2) {
                        typeText(YELLOW ITALIC_ON"If you're having second thoughts then maybe we should NOT think about destroying everything we've built, okay?\nMaybe we'll do it some other day, when you're more decisive in your life.\n\n\n"OFF YELLOW"[ENTER] "OFF HGRN"...",textSpeed);
                        getchar();
                        clearScreen();
                        break;
                    }
                }
                break;

            case 7:

                while(1){
                    if(textSpeed != 0){
                    typeText(YELLOW ITALIC_ON"Do you want to remove the text animation?\n\n\n"OFF YELLOW"[1]"OFF HRED" Yeah, its kind of annoying."OFF YELLOW"\n[2]"OFF HGRN" No, I changed my mind.\n\n"OFF YELLOW"Decision: ",textSpeed);
                    int choice; scanf("%d", &choice); getchar();
                    if (choice == 1) {
                        
                        typeText(YELLOW ITALIC_ON"It took quite a lot of effort to make it look good you know. \nBut oh well, \"The user is always right\" I guess.\n\n\n"OFF YELLOW"[ENTER]"OFF HGRN" ..."OFF,textSpeed);
                        getchar();
                        textSpeed = 0;
                        clearScreen();
                        break;
                    }
                    else if (choice == 2) {
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
                } else {
                    typeText(YELLOW ITALIC_ON"Changed your mind? Kind of an indecisive person aren't you.\n\n\n"OFF YELLOW"[1]"HGRN" Yeah, so what?!\n"OFF YELLOW"[2]"OFF RED" Well, maybe you're right!"OFF YELLOW"\n\n\nDecision: "OFF,textSpeed);
                    int choice; scanf("%d", &choice); getchar();
                    if (choice == 1) {
                        typeText(YELLOW ITALIC_ON"Good for you, I guess...\n\n\n"OFF YELLOW"[ENTER]"HGRN" ..."OFF,textSpeed);
                        textSpeed = 15;
                        getchar();
                        clearScreen();
                        break;
                    }
                    else if (choice == 2){
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
                
                break;
            case 8:
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
                getchar();
                clearScreen();
                break;
        }
    }

    return 0;
}