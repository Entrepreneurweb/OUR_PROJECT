#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFFER_SIZE 1024
#define CHAR_SIZE 50
#define MESSAGE_SIZE 22
#define PART_SIZE 10
 
#define QUESTION_SIZE 510
#define OPTION_SIZE 100

// message protocol enums
#define MESSAGE_ERROR   "1"
#define MESSAGE_SUCCES  "2"
#define LOGIN           "3"
#define NEW_ACCOUNT     "4"
#define ANSWER_QUESTION "5"
#define GET_SESSION_LIST "13"
#define CREATE_SESSION  "11"
#define ADD_QUESTION    "5"
 
// translation



// Structure de l'utilisateur
typedef struct {
    int UserId;
    char nom[PART_SIZE];
    char password[PART_SIZE];
} User;

// Protocole de gestion des messages
// join a game 11
typedef enum {
    messageError = 1,
    messageSuccess = 2,
    LoginF = 3,
    NewAccount = 4,
    GiveAnswer = 5
} MessageProtocol;

// Fonction utilitaire pour afficher un titre centralisé avec des motifs
void PrintDecoratedTitle(const char* title) {
    const int consoleWidth = 50; // Largeur de la console pour le centrage
    int titleLength = strlen(title);
    int padding = (consoleWidth - titleLength) / 2;
    printf("\033[1;34m"); // Couleur bleue
    for (int i = 0; i < padding; i++) printf("-");
    printf("%s", title);
    for (int i = 0; i < padding; i++) printf("-");
    printf("\033[0m\n");
}

int getFlag(char *buffer) {
    char charFlag;
    charFlag = *(buffer + MESSAGE_SIZE - 1);
    switch (charFlag) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        default:
            return 5;
    }
}

char* CheckUserIdentity(User user, SOCKET socket, char* MessageBuffer) {
    PrintDecoratedTitle("IDENTITY VERIFICATION");
   // printf("\033[1;32mHere is the message before sending:\033[0m %s\n", MessageBuffer);
    int SentMessageFd = send(socket, MessageBuffer, MESSAGE_SIZE, 0);
    if (SentMessageFd == SOCKET_ERROR) {
        printf("\033[1;31mMessage send error:\033[0m %d\n", WSAGetLastError());
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        return MessageBuffer;
    }

    printf("\033[1;33mWAITING FOR IDENTITY CONFIRMATION\033[0m\n");

    int ReceivedMessage = recv(socket, MessageBuffer, MESSAGE_SIZE, 0);
    printf("\033[1;32mMessage received\033[0m\n");
    if (ReceivedMessage == SOCKET_ERROR) {
        printf("\033[1;31mMessage reception error:\033[0m %d\n", WSAGetLastError());
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        return MessageBuffer;
    }

    int ResponseFlag = getFlag(MessageBuffer);
    if (ResponseFlag != 3) {
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        printf("\033[1;31mNO SUCH USER EXISTS, PLEASE CREATE AN ACCOUNT\033[0m\n");
        return MessageBuffer;
    }
    MessageBuffer[MESSAGE_SIZE - 1] = messageSuccess;
    return MessageBuffer;
}


char* CreateUser(SOCKET socket, char* MessageBuffer) {
    User myuser;
    PrintDecoratedTitle("CREATE A NEW ACCOUNT");

    printf("\033[1;36mCREATE A USERNAME:\033[0m ");
    scanf("%9s", myuser.nom);

    printf("\033[1;36mCREATE A PASSWORD:\033[0m ");
    scanf("%9s", myuser.password);

    memset(MessageBuffer, '*', MESSAGE_SIZE);
    strncpy(MessageBuffer, myuser.nom, strlen(myuser.nom));
    strncpy(MessageBuffer + PART_SIZE, myuser.password, strlen(myuser.password));
    // adding flag
    strncpy(MessageBuffer + MESSAGE_SIZE - 1, NEW_ACCOUNT, 1);
    // printf(" message from sign up: %s \n", MessageBuffer);  // Unnecessary log
    if (send(socket, MessageBuffer, MESSAGE_SIZE, 0) == SOCKET_ERROR) {
        printf("\033[1;31mSend error:\033[0m %d\n", WSAGetLastError());
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        return MessageBuffer;
    }

    printf("\033[1;33mWAITING FOR ACCOUNT CREATION\033[0m\n");

    memset(MessageBuffer, '*', MESSAGE_SIZE);
    int ReceivedMessage = recv(socket, MessageBuffer, MESSAGE_SIZE, 0);
    if (ReceivedMessage <= 0) {
        printf("\033[1;31mReception error or connection closed:\033[0m %d\n", WSAGetLastError());
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        return MessageBuffer;
    }

    int ResponseFlag = getFlag(MessageBuffer);
    if (ResponseFlag != 4) {
        printf("\033[1;31mError during account creation.\033[0m\n");
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        MessageBuffer[MESSAGE_SIZE - 1] = messageError;
        return MessageBuffer;
    }

    memset(MessageBuffer, '*', MESSAGE_SIZE);
    MessageBuffer[MESSAGE_SIZE - 1] = messageSuccess;
    printf("\033[1;32mACCOUNT SUCCESSFULLY CREATED!\033[0m\n");
    return MessageBuffer;
}


void Login(int *Islog, SOCKET socket, char* MessageBuffer) {
    User myuser;
    myuser.UserId = 0;
    int userAction;

    PrintDecoratedTitle("LOGIN");

    while (1) {
        printf("\033[1;36mENTER YOUR CREDENTIALS TO LOG IN\033[0m\n");
        printf("\033[1;36mENTER YOUR USERNAME:\033[0m ");
        scanf("%9s", myuser.nom);
        printf("\033[1;36mENTER YOUR PASSWORD:\033[0m ");
        scanf("%9s", myuser.password);

        memset(MessageBuffer, '*', MESSAGE_SIZE);
        strncpy(MessageBuffer, myuser.nom, strlen(myuser.nom));
        strncpy(MessageBuffer + PART_SIZE - 1, myuser.password, strlen(myuser.password));
        strncpy(MessageBuffer + MESSAGE_SIZE - 1, LOGIN , 1);

        CheckUserIdentity(myuser, socket, MessageBuffer);

        if (MessageBuffer[MESSAGE_SIZE - 1] == messageSuccess) {
            *Islog = 1;
            PrintDecoratedTitle("SUCCESSFUL LOGIN");
            break;
        }

        printf("\033[1;31mLOGIN FAILED, ERROR\033[0m: %s\n", myuser.nom);
        printf("\033[1;33m1 = CREATE A NEW ACCOUNT, 2 = EXIT, OTHER = TRY AGAIN\033[0m\n");
        scanf("%d", &userAction);
        if (userAction == 1) {
            CreateUser(socket, MessageBuffer);
        } else if (userAction == 2) {
            break;
        }
    }
}

// Ma fonction pour  retourner le vrai choi
char* ChosenAnswer(char* KeyboardChar, char* opt1, char* opt2, char* opt3, char* opt4)
{
    char SwCkeck = KeyboardChar[0];
    switch (SwCkeck)
    {
    case 'A':
    case 'a':
        return opt1;
    case 'B':
    case 'b':
        return opt2;
    case 'C':
    case 'c':
        return opt3;
    case 'D':
    case 'd':
        return opt4;
    default:
        printf("YOUR INPUT IS INVALID. OPTION 1 WILL BE RETURNED BY DEFAULT.\n");
        return opt1;
    }
}

void ClearScreen() {
    // Efface l'écran (fonction compatible avec les systèmes UNIX)
    printf("\033[H\033[J");
}
 
 void DecoratedTitle(const char *title) {
    printf("\n\033[1;32m=======================================\033[0m\n");
    printf("\033[1;32m        %s \033[0m\n", title);
    printf("\033[1;32m=======================================\033[0m\n\n");
}


void Play_Game(SOCKET socket, char* MessageBuffer, int Check_Question_Load) {
    int i;
    int EatBuffer = 0;
    int Question_Number;
    char QuesMessageBuffer[QUESTION_SIZE] = {0};
    memset(MessageBuffer, '*', MESSAGE_SIZE);

    if (Check_Question_Load == 0) {
        char QuizQuestion[5];
        printf("\033[1;33mHOW MANY QUESTIONS WOULD YOU LIKE TO GENERATE IN THE QUIZ?\033[0m\n");
        scanf("%s", QuizQuestion);
        Question_Number = atoi(QuizQuestion);

        // Modify the message buffer to send the desired number of questions
        for (i = 0; i < strlen(QuizQuestion); i++) {
            MessageBuffer[i] = QuizQuestion[i];
        }

        MessageBuffer[MESSAGE_SIZE - 1] = '8';  // Indicate this is a game creation request
        int SentMessageFd = send(socket, MessageBuffer, MESSAGE_SIZE, 0);
        if (SentMessageFd == SOCKET_ERROR) {
            printf("\033[1;31mError sending message:\033[0m %d\n", WSAGetLastError());
            return;
        }

        int ReceiveMessageFd;
        memset(MessageBuffer, '*', MESSAGE_SIZE);
        ReceiveMessageFd = recv(socket, MessageBuffer, MESSAGE_SIZE, 0);
        if (ReceiveMessageFd == SOCKET_ERROR) {
            printf("\033[1;31mError receiving game creation confirmation message:\033[0m %d\n", WSAGetLastError());
            return;
        }
    }

    // Waiting for questions and sending answers
    int RecvQuestionFd, SentQuestion;
    char MyChoice[4];
    for (int i = 0; i < Question_Number; i++) {
        char bufOpt1[OPTION_SIZE] = {0};
        char bufOpt2[OPTION_SIZE] = {0};
        char bufOpt3[OPTION_SIZE] = {0};
        char bufOpt4[OPTION_SIZE] = {0};

        // Receive the question
        RecvQuestionFd = recv(socket, QuesMessageBuffer, QUESTION_SIZE, 0);
        if (RecvQuestionFd == SOCKET_ERROR) {
            printf("\033[1;31mError receiving question number :%d, ERROR :\033[0m %d\n", i, WSAGetLastError());
            return;
        } else {
            // Clear the screen after receiving the question
            ClearScreen();

            // Display the question and options
            printf("\033[1;33mQuestion %d: %s\033[0m\n", i + 1, QuesMessageBuffer);
            printf("\033[1;34mChoose your answer (A-D):\033[0m\n");

            // Display the options
            printf(" \033[1;33mA. %s\033[0m\n", QuesMessageBuffer + OPTION_SIZE);
            strncpy(bufOpt1, QuesMessageBuffer + OPTION_SIZE, OPTION_SIZE);

            printf(" \033[1;33mB. %s\033[0m\n", QuesMessageBuffer + 2 * OPTION_SIZE);
            strncpy(bufOpt2, QuesMessageBuffer + 2 * OPTION_SIZE, OPTION_SIZE);

            printf(" \033[1;33mC. %s\033[0m\n", QuesMessageBuffer + 3 * OPTION_SIZE);
            strncpy(bufOpt3, QuesMessageBuffer + 3 * OPTION_SIZE, OPTION_SIZE);

            printf(" \033[1;33mD. %s\033[0m\n", QuesMessageBuffer + 4 * OPTION_SIZE);
            strncpy(bufOpt4, QuesMessageBuffer + 4 * OPTION_SIZE, OPTION_SIZE);

            // Input the answer
            scanf("%1s", MyChoice);

            // Prepare the response
            memset(QuesMessageBuffer, 0, QUESTION_SIZE);
            strncpy(QuesMessageBuffer, ChosenAnswer(MyChoice, bufOpt1, bufOpt2, bufOpt3, bufOpt4), OPTION_SIZE);
            QuesMessageBuffer[QUESTION_SIZE - 1] = '9';  // End-of-answer flag

            // Send the answer
            SentQuestion = send(socket, QuesMessageBuffer, QUESTION_SIZE, 0);
            if (SentQuestion == SOCKET_ERROR) {
                printf("\033[1;31mError sending answer:\033[0m %d\n", WSAGetLastError());
                return;
            }
        }
    }

    recv(socket, MessageBuffer, MESSAGE_SIZE, 0);

    // Extract grade, rank, and total player score
    char Grade_Buffer[4];
    char Rank_Buffer[5];
    char Total_Player_Buffer[5];
    int u;

    i = 0;
    for (u = 0; MessageBuffer[u] != '*' && MessageBuffer[u] != '.' && i < 3; u++, i++) {
        Grade_Buffer[i] = MessageBuffer[u];
    }
    Grade_Buffer[i] = '\0';  // Null-terminate the string
    int Player_Note = atoi(Grade_Buffer);

    i = 0;
    for (u = 10; MessageBuffer[u] != '*' && MessageBuffer[u] != '.' && i < 4; u++, i++) {
        Rank_Buffer[i] = MessageBuffer[u];
    }
    Rank_Buffer[i] = '\0';  // Null-terminate the string

    i = 0;
    for (u = 15; MessageBuffer[u] != '*' && MessageBuffer[u] != '.' && i < 4; u++, i++) {
        Total_Player_Buffer[i] = MessageBuffer[u];
    }
    Total_Player_Buffer[i] = '\0';  // Null-terminate the string

    // Print results with decorations
    ClearScreen();  // Clear screen before displaying results

    DecoratedTitle("YOU GOT:");
    printf("\033[1;32m%d OVER 100 \033[0m\n", Player_Note);

    DecoratedTitle("You are now Rank:");
    printf("\033[1;32m#%s with a TotalScore of: %s\033[0m\n", Rank_Buffer, Total_Player_Buffer);
}
 
 
 void Add_Question(SOCKET socket) {
    char formattedQuestion[QUESTION_SIZE];
    char Question_Buffer[198];
    char Option1_Buffer[50];
    char Option2_Buffer[50];
    char Option3_Buffer[50];
    char Option4_Buffer[50];
    char Correct_Option_Buffer[50];
    int correctOptionIndex;

    
    // Collecting information from the user
   

    printf("ENTER THE QUESTION: ");
     getchar();
    fgets(Question_Buffer, sizeof(Question_Buffer), stdin);
    Question_Buffer[strcspn(Question_Buffer, "\n")] = '\0'; // Remove the newline character if present
    printf("\n");
    printf("ENTER OPTION 1: ");
    fgets(Option1_Buffer, sizeof(Option1_Buffer), stdin);
    Option1_Buffer[strcspn(Option1_Buffer, "\n")] = '\0'; // Remove the newline character if present

    printf("ENTER OPTION 2: ");
    fgets(Option2_Buffer, sizeof(Option2_Buffer), stdin);
    Option2_Buffer[strcspn(Option2_Buffer, "\n")] = '\0'; // Remove the newline character if present

    printf("ENTER OPTION 3: ");
    fgets(Option3_Buffer, sizeof(Option3_Buffer), stdin);
    Option3_Buffer[strcspn(Option3_Buffer, "\n")] = '\0'; // Remove the newline character if present

    printf("ENTER OPTION 4: ");
    fgets(Option4_Buffer, sizeof(Option4_Buffer), stdin);
    Option4_Buffer[strcspn(Option4_Buffer, "\n")] = '\0'; // Remove the newline character if present

    // Selecting the correct option
    do {
        printf("ENTER THE INDEX OF THE CORRECT OPTION (1-4): ");
        scanf("%d", &correctOptionIndex);
        getchar(); // Consume the residual '\n' from the previous scanf
    } while (correctOptionIndex < 1 || correctOptionIndex > 4);

    // Copy the correct option dynamically
    switch (correctOptionIndex) {
        case 1:
            strncpy(Correct_Option_Buffer, Option1_Buffer, sizeof(Correct_Option_Buffer));
            break;
        case 2:
            strncpy(Correct_Option_Buffer, Option2_Buffer, sizeof(Correct_Option_Buffer));
            break;
        case 3:
            strncpy(Correct_Option_Buffer, Option3_Buffer, sizeof(Correct_Option_Buffer));
            break;
        case 4:
            strncpy(Correct_Option_Buffer, Option4_Buffer, sizeof(Correct_Option_Buffer));
            break;
    }

    // Format the data (total max size 510)
    snprintf(formattedQuestion, QUESTION_SIZE, "# QUESTION:%s # OPTION1:%s # OPTION2:%s # OPTION3:%s # OPTION4:%s # CORRECTOPTION:%s #",
             Question_Buffer, Option1_Buffer, Option2_Buffer, Option3_Buffer, Option4_Buffer, Correct_Option_Buffer);

    //printf("THE ENTERED QUESTION IS: %s\n", formattedQuestion);

    char TestBuffer[MESSAGE_SIZE];

    memset(TestBuffer, '*', MESSAGE_SIZE);

    strncpy(TestBuffer + MESSAGE_SIZE - 1, ADD_QUESTION, 1); // Add question flag

    // Sending acknowledgment to the server
    send(socket, TestBuffer, MESSAGE_SIZE, 0);

    // Receiving server confirmation
    recv(socket, TestBuffer, MESSAGE_SIZE, 0);

    // Send the actual question data
    send(socket, formattedQuestion, QUESTION_SIZE, 0);

    printf("\033[1;32mQuestion successfully added!\033[0m\n");
}



 void User_Menu(SOCKET socket, char* MessageBuffer) {
    int Menu_Option = 0;
    while (Menu_Option == 0) {
        
        // Efface l'écran pour chaque nouvelle itération de menu pour un effet fluide
        ClearScreen();
        DecoratedTitle("USER MENU");

        // Affiche les options du menu
        printf(" \033[1;33m1. START A NEW GAME\033[0m\n");
        printf(" \033[1;33m2. ADD A QUESTION\033[0m\n");
        printf(" \033[1;33m3. EXIT\033[0m\n");
        printf("\033[1;36mPlease select an option (1-3): \033[0m");

        // Boucle pour s'assurer que l'utilisateur entre une option valide
        while (Menu_Option < 1 || Menu_Option > 3) {
            // Lecture de l'option
            if (scanf("%d", &Menu_Option) != 1) {
                // Si l'entrée n'est pas un nombre valide, vider le buffer et recommencer
                while (getchar() != '\n');
                printf("\033[1;31mInvalid input! Please enter a number between 1 and 3.\033[0m\n");
                printf("\033[1;36mPlease select an option (1-3): \033[0m");
                continue;
            }
        }

        // Traitement de l'option choisie
        switch (Menu_Option) {
            case 1:
                Play_Game(socket, MessageBuffer, 0);
                break;
            case 2:
                Add_Question(socket);
                break;
            case 3:
                printf("\n\033[1;31mExiting the menu... Goodbye!\033[0m\n");
                return;  // Quitter la fonction et sortir
            default:
                break;
        }

        // Attendre que l'utilisateur appuie sur une touche avant de continuer
        printf("\n\033[1;36mPress any key to return to the menu...\033[0m");
        getchar(); // Lire et ignorer le \n restant
        getchar(); // Attendre une pression de touche
        Menu_Option = 0;
    }
}






int main() {
    // Initialize the WinSock library
    WSADATA wsadata;
    int Islog = 0;  // Variable to check if the user is logged in or not

    // Initialize WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        printf("\033[1;31mInitialization error:\033[0m %d\n", WSAGetLastError());
        exit(1);  // Exit if initialization fails
    }

    // Create the socket
    SOCKET SocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFd == INVALID_SOCKET) {
        printf("\033[1;31mSocket creation error:\033[0m %d\n", WSAGetLastError());
        WSACleanup();  // Clean up before exiting
        exit(1);
    }

    // Initialize the server address
    struct sockaddr_in SocketAddress;
    SocketAddress.sin_family = AF_INET;
    SocketAddress.sin_port = htons(5600);  // Port used for the connection

    // Convert the IP address to binary format
    int inetReturnCode = inet_pton(AF_INET, "127.0.0.1", &SocketAddress.sin_addr);
    if (inetReturnCode <= 0) {
        printf("\033[1;31mIP address error.\033[0m\n");
        closesocket(SocketFd);  // Close the socket in case of IP address error
        WSACleanup();  // Clean up before exiting
        exit(1);
    }

    // Display the title for the connection
    PrintDecoratedTitle("CONNECTING TO THE SERVER");

    // Attempt to connect to the server
    if (connect(SocketFd, (struct sockaddr*)&SocketAddress, sizeof(SocketAddress)) == SOCKET_ERROR) {
        printf("\033[1;31mConnection error:\033[0m %d\n", WSAGetLastError());
        closesocket(SocketFd);  // Close the socket in case of connection error
        WSACleanup();  // Clean up before exiting
        exit(1);
    }

    // Display a confirmation message for successful connection
    printf("\033[1;32mConnection accepted.\033[0m\n");

    // Declare a buffer for the message
    char MessageBuffer[MESSAGE_SIZE];

    // Call the login function (Décommenter et implémenter la fonction Login)
    Login(&Islog, SocketFd, MessageBuffer);

    // If the user is logged in
    if (Islog) {
        PrintDecoratedTitle("GOOD LUCK \n");
        // Call the User_Menu function to display the user menu
        User_Menu(SocketFd, MessageBuffer);
    }

    // Close the socket and clean up WinSock before exiting
    closesocket(SocketFd);
    WSACleanup();

    return 0;
}


