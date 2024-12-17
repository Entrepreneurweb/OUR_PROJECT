#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include  "GameUtil.h"

// Fonction pour ajouter des questions
int Add_Question_From_String(char *questionData) {
    FILE *Question_FileStream = fopen("Doc.txt", "a+");
    FILE *Id_FileStream = fopen("QuestionId.txt", "r+");

    if (Question_FileStream == NULL || Id_FileStream == NULL) {
        printf("ERREUR D'OUVERTURE DE FICHIER\n");
        return -2;
    }

    char buffer[170];
    int Cur_Id = 0;

    // Lire l'ID actuel depuis QuestionId.txt
    if (fgets(buffer, sizeof(buffer), Id_FileStream)) {
        sscanf(buffer, "ID:%d", &Cur_Id);
    }

    // Ajouter la question avec l'ID actuel
    fprintf(Question_FileStream, "ID:%d %s\n", Cur_Id, questionData);

    // Incrémenter l'ID
    Cur_Id++;

    // Mettre à jour le fichier QuestionId.txt
    freopen("QuestionId.txt", "w", Id_FileStream);
    fprintf(Id_FileStream, "ID:%d", Cur_Id);

    printf("LA QUESTION A ETE AJOUTEE AVEC SUCCES\n");
    fclose(Question_FileStream);
    fclose(Id_FileStream);

    return 0;
}

// Fonction pour préparer un tableau de questions
char **Prepared_Ques_Arr(int QuestionNumber) {
    // Correction : Utilisation correcte d'un tableau de pointeurs
    char **QuestionMatrix = (char**)malloc(QuestionNumber * sizeof(char*));
    if (QuestionMatrix == NULL) {
        printf("Erreur d'allocation mémoire pour QuestionMatrix.\n");
        return NULL;
    }

    for (int i = 0; i < QuestionNumber; i++) {
        QuestionMatrix[i] = (char*)malloc(500 * sizeof(char));
        if (QuestionMatrix[i] == NULL) {
            printf("Erreur d'allocation mémoire pour QuestionMatrix[%d].\n", i);
            for (int j = 0; j < i; j++) {
                free(QuestionMatrix[j]); // Libérer les allocations précédentes
            }
            free(QuestionMatrix);
            return NULL;
        }
    }

    return QuestionMatrix;
}

// Générer des nombres aléatoires uniques
int *Random_Without_Reps(int min, int max, int sizeOfArray, int *AllreadyGenerated) {
    int OurRandNumber;
    int isDuplicate;
srand(time(NULL)); 
    memset(AllreadyGenerated, -1, sizeOfArray * sizeof(int)); // Initialisation

    for (int i = 0; i < sizeOfArray; i++) {
        isDuplicate = 1;

        while (isDuplicate) {
            OurRandNumber = rand() % (max - min + 1) + min;
            isDuplicate = 0;

            for (int j = 0; j < i; j++) {
                if (AllreadyGenerated[j] == OurRandNumber) {
                    isDuplicate = 1;
                    break;
                }
            }
        }

        AllreadyGenerated[i] = OurRandNumber;
        printf("Unique random number: %d\n", OurRandNumber);
    }

    return AllreadyGenerated;
}

// Vérifier si une valeur est dans un tableau
int checkIfInRandQues(int *array, int arraySize, int CheckVal) {
    int Result = 0;
    for (int i = 0; i < arraySize; i++) {
        if (array[i] == CheckVal) {
            Result = 1;
            break;
        }
    }
    return Result;
}

// Préparer un quiz
void Prepare_Quiz(char ** QuestionMatrix , int* questionNumber) {
    char ReaderBuffer[500];
  //  printf("ENTER THE NUMBER OF QUESTION IN THE QUIZZ \n");
   // int questionNumber;
   // scanf("%d", questionNumber);

    int *Rand_Integer_Arr = (int *)malloc(sizeof(int) * ( *(questionNumber)));
    if (Rand_Integer_Arr == NULL) {
        printf("Erreur d'allocation mémoire pour Rand_Integer_Arr.\n");
        return;
    }
    Random_Without_Reps(0, 50, (*questionNumber), Rand_Integer_Arr);

    //char **QuestionMatrix = Prepared_Ques_Arr(Question_Number);
    if (QuestionMatrix == NULL) {
        free(Rand_Integer_Arr);
        return;
    }

    FILE *Question_FileStream = fopen("Doc.txt", "r");
    if (Question_FileStream == NULL) {
        printf("Erreur d'ouverture du fichier Doc.txt.\n");
        free(Rand_Integer_Arr);
        
        for (int i = 0; i < (*questionNumber); i++) {
            free(QuestionMatrix[i]);
        }
        free(QuestionMatrix);
        return;
    }

    int counter = 0;
    while (fgets(ReaderBuffer, sizeof(ReaderBuffer), Question_FileStream) && counter < (*questionNumber)) {
        // Extraire l'ID de la question (dans le format "ID:1 QUESTION:...")
        int questionID = -1;
        sscanf(ReaderBuffer, "ID:%d", &questionID);

        // Vérifier si l'ID de la question est dans le tableau Rand_Integer_Arr
        if (checkIfInRandQues(Rand_Integer_Arr, (*questionNumber), questionID)) {
            strcpy(QuestionMatrix[counter], ReaderBuffer);
            counter++;
        }
    }

    fclose(Question_FileStream);
/*
    for (int i = 0; i < (*questionNumber); i++) {
        if (strlen(QuestionMatrix[i]) > 0) {
            printf(" %s\n", QuestionMatrix[i]);
        }
        free(QuestionMatrix[i]); // Libérer chaque ligne
    }

    free(QuestionMatrix);*/
    free(Rand_Integer_Arr);
}

void TakeQuiz(char ** QuestionMatrix)
{
    
}